#include "core/gguf_parser.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace rifle {
namespace {

constexpr std::uint64_t kMaxStringBytes = 256ull * 1024ull * 1024ull;
constexpr std::uint64_t kMaxArrayItems = 10ull * 1000ull * 1000ull;
constexpr std::uint32_t kMaxTensorRank = 8;

void read_exact(std::istream& input, char* destination, std::streamsize size, const std::string& field_name) {
    if (!input.read(destination, size)) {
        throw std::runtime_error("Failed to read GGUF field: " + field_name);
    }
}

template <typename T>
T read_little_endian(std::istream& input, const std::string& field_name) {
    static_assert(std::is_unsigned_v<T>, "GGUF little-endian helper expects unsigned integer types");

    std::array<unsigned char, sizeof(T)> bytes{};
    read_exact(input, reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()), field_name);

    T value = 0;
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        value |= static_cast<T>(bytes[i]) << (i * 8);
    }
    return value;
}

std::uint64_t align_to(std::uint64_t value, std::uint64_t alignment) {
    if (alignment == 0) {
        throw std::runtime_error("GGUF alignment cannot be zero");
    }
    const std::uint64_t remainder = value % alignment;
    if (remainder == 0) {
        return value;
    }
    return value + alignment - remainder;
}

std::string read_string(std::istream& input, std::string_view field_name) {
    const std::uint64_t size = read_little_endian<std::uint64_t>(input, std::string(field_name) + ".size");
    if (size > kMaxStringBytes) {
        throw std::runtime_error("GGUF string is too large: " + std::to_string(size));
    }

    std::string value(static_cast<std::size_t>(size), '\0');
    if (!value.empty()) {
        read_exact(input, value.data(), static_cast<std::streamsize>(value.size()), std::string(field_name) + ".bytes");
    }
    return value;
}

float read_float32(std::istream& input, const std::string& field_name) {
    const std::uint32_t raw = read_little_endian<std::uint32_t>(input, field_name);
    return std::bit_cast<float>(raw);
}

double read_float64(std::istream& input, const std::string& field_name) {
    const std::uint64_t raw = read_little_endian<std::uint64_t>(input, field_name);
    return std::bit_cast<double>(raw);
}

GgufMetadataValue read_metadata_value(std::istream& input, GgufMetadataType type);

GgufMetadataValue read_array(std::istream& input) {
    GgufMetadataValue value;
    value.type = GgufMetadataType::Array;
    value.array_element_type = static_cast<GgufMetadataType>(read_little_endian<std::uint32_t>(input, "metadata.array.element_type"));

    const std::uint64_t size = read_little_endian<std::uint64_t>(input, "metadata.array.size");
    if (size > kMaxArrayItems) {
        throw std::runtime_error("GGUF metadata array is too large: " + std::to_string(size));
    }

    value.array_values.reserve(static_cast<std::size_t>(size));
    for (std::uint64_t i = 0; i < size; ++i) {
        value.array_values.push_back(read_metadata_value(input, value.array_element_type));
    }
    return value;
}

GgufMetadataValue read_metadata_value(std::istream& input, GgufMetadataType type) {
    GgufMetadataValue value;
    value.type = type;

    switch (type) {
    case GgufMetadataType::Uint8:
        value.unsigned_value = read_little_endian<std::uint8_t>(input, "metadata.uint8");
        return value;
    case GgufMetadataType::Int8:
        value.signed_value = std::bit_cast<std::int8_t>(read_little_endian<std::uint8_t>(input, "metadata.int8"));
        return value;
    case GgufMetadataType::Uint16:
        value.unsigned_value = read_little_endian<std::uint16_t>(input, "metadata.uint16");
        return value;
    case GgufMetadataType::Int16:
        value.signed_value = std::bit_cast<std::int16_t>(read_little_endian<std::uint16_t>(input, "metadata.int16"));
        return value;
    case GgufMetadataType::Uint32:
        value.unsigned_value = read_little_endian<std::uint32_t>(input, "metadata.uint32");
        return value;
    case GgufMetadataType::Int32:
        value.signed_value = std::bit_cast<std::int32_t>(read_little_endian<std::uint32_t>(input, "metadata.int32"));
        return value;
    case GgufMetadataType::Float32:
        value.floating_value = read_float32(input, "metadata.float32");
        return value;
    case GgufMetadataType::Bool:
        value.bool_value = read_little_endian<std::uint8_t>(input, "metadata.bool") != 0;
        return value;
    case GgufMetadataType::String:
        value.string_value = read_string(input, "metadata.string");
        return value;
    case GgufMetadataType::Array:
        return read_array(input);
    case GgufMetadataType::Uint64:
        value.unsigned_value = read_little_endian<std::uint64_t>(input, "metadata.uint64");
        return value;
    case GgufMetadataType::Int64:
        value.signed_value = std::bit_cast<std::int64_t>(read_little_endian<std::uint64_t>(input, "metadata.int64"));
        return value;
    case GgufMetadataType::Float64:
        value.floating_value = read_float64(input, "metadata.float64");
        return value;
    default:
        throw std::runtime_error("Unsupported GGUF metadata type: " + to_string(type));
    }
}

bool metadata_unsigned_value(const GgufMetadataValue& value, std::uint64_t& output) {
    switch (value.type) {
    case GgufMetadataType::Uint8:
    case GgufMetadataType::Uint16:
    case GgufMetadataType::Uint32:
    case GgufMetadataType::Uint64:
        output = value.unsigned_value;
        return true;
    default:
        return false;
    }
}

std::string escaped_preview(std::string_view text, std::size_t max_chars) {
    std::string escaped;
    const std::size_t size = std::min(text.size(), max_chars);
    for (std::size_t i = 0; i < size; ++i) {
        switch (text[i]) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped += text[i];
            break;
        }
    }
    if (text.size() > max_chars) {
        escaped += "...";
    }
    return escaped;
}

} // namespace

GgufParser::GgufParser(std::string path)
    : path_(std::move(path)), info_(parse_file(path_)) {}

const std::string& GgufParser::path() const noexcept {
    return path_;
}

const GgufHeader& GgufParser::header() const noexcept {
    return info_.header;
}

const std::vector<GgufMetadataEntry>& GgufParser::metadata() const noexcept {
    return info_.metadata;
}

const std::vector<GgufTensorInfo>& GgufParser::tensors() const noexcept {
    return info_.tensors;
}

std::uint64_t GgufParser::tensor_data_offset() const noexcept {
    return info_.tensor_data_offset;
}

const GgufFileInfo& GgufParser::info() const noexcept {
    return info_;
}

const GgufMetadataValue* GgufParser::find_metadata(std::string_view key) const noexcept {
    for (const GgufMetadataEntry& entry : info_.metadata) {
        if (entry.key == key) {
            return &entry.value;
        }
    }
    return nullptr;
}

GgufHeader GgufParser::read_header(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Failed to open GGUF file: " + path);
    }

    std::array<char, 4> magic{};
    read_exact(input, magic.data(), static_cast<std::streamsize>(magic.size()), "magic");
    if (magic != std::array<char, 4>{'G', 'G', 'U', 'F'}) {
        throw std::runtime_error("Invalid GGUF file: magic number mismatch");
    }

    GgufHeader header;
    header.version = read_little_endian<std::uint32_t>(input, "version");
    header.tensor_count = read_little_endian<std::uint64_t>(input, "tensor_count");
    header.metadata_kv_count = read_little_endian<std::uint64_t>(input, "metadata_kv_count");

    if (header.version < 2 || header.version > 3) {
        throw std::runtime_error("Unsupported GGUF version: " + std::to_string(header.version));
    }

    return header;
}

GgufFileInfo GgufParser::parse_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Failed to open GGUF file: " + path);
    }

    std::array<char, 4> magic{};
    read_exact(input, magic.data(), static_cast<std::streamsize>(magic.size()), "magic");
    if (magic != std::array<char, 4>{'G', 'G', 'U', 'F'}) {
        throw std::runtime_error("Invalid GGUF file: magic number mismatch");
    }

    GgufFileInfo info;
    info.header.version = read_little_endian<std::uint32_t>(input, "version");
    info.header.tensor_count = read_little_endian<std::uint64_t>(input, "tensor_count");
    info.header.metadata_kv_count = read_little_endian<std::uint64_t>(input, "metadata_kv_count");

    if (info.header.version < 2 || info.header.version > 3) {
        throw std::runtime_error("Unsupported GGUF version: " + std::to_string(info.header.version));
    }

    info.metadata.reserve(static_cast<std::size_t>(info.header.metadata_kv_count));
    for (std::uint64_t i = 0; i < info.header.metadata_kv_count; ++i) {
        GgufMetadataEntry entry;
        entry.key = read_string(input, "metadata.key");
        const auto type = static_cast<GgufMetadataType>(read_little_endian<std::uint32_t>(input, "metadata.type"));
        entry.value = read_metadata_value(input, type);
        info.metadata.push_back(std::move(entry));
    }

    info.tensors.reserve(static_cast<std::size_t>(info.header.tensor_count));
    for (std::uint64_t i = 0; i < info.header.tensor_count; ++i) {
        GgufTensorInfo tensor;
        tensor.name = read_string(input, "tensor.name");

        const std::uint32_t rank = read_little_endian<std::uint32_t>(input, "tensor.rank");
        if (rank == 0 || rank > kMaxTensorRank) {
            throw std::runtime_error("Unsupported GGUF tensor rank " + std::to_string(rank) + " for tensor " + tensor.name);
        }

        tensor.dimensions.reserve(rank);
        for (std::uint32_t dimension = 0; dimension < rank; ++dimension) {
            tensor.dimensions.push_back(read_little_endian<std::uint64_t>(input, "tensor.dimension"));
        }

        tensor.type = static_cast<GgmlTensorType>(read_little_endian<std::uint32_t>(input, "tensor.type"));
        tensor.offset = read_little_endian<std::uint64_t>(input, "tensor.offset");
        info.tensors.push_back(std::move(tensor));
    }

    const std::streampos current_position = input.tellg();
    if (current_position < 0) {
        throw std::runtime_error("Failed to locate GGUF tensor data offset");
    }

    std::uint64_t alignment = 32;
    for (const GgufMetadataEntry& entry : info.metadata) {
        if (entry.key == "general.alignment") {
            if (!metadata_unsigned_value(entry.value, alignment)) {
                throw std::runtime_error("GGUF general.alignment must be an unsigned integer");
            }
            break;
        }
    }

    info.tensor_data_offset = align_to(static_cast<std::uint64_t>(current_position), alignment);
    return info;
}

std::string to_string(GgufMetadataType type) {
    switch (type) {
    case GgufMetadataType::Uint8:
        return "uint8";
    case GgufMetadataType::Int8:
        return "int8";
    case GgufMetadataType::Uint16:
        return "uint16";
    case GgufMetadataType::Int16:
        return "int16";
    case GgufMetadataType::Uint32:
        return "uint32";
    case GgufMetadataType::Int32:
        return "int32";
    case GgufMetadataType::Float32:
        return "float32";
    case GgufMetadataType::Bool:
        return "bool";
    case GgufMetadataType::String:
        return "string";
    case GgufMetadataType::Array:
        return "array";
    case GgufMetadataType::Uint64:
        return "uint64";
    case GgufMetadataType::Int64:
        return "int64";
    case GgufMetadataType::Float64:
        return "float64";
    default:
        return "unknown(" + std::to_string(static_cast<std::uint32_t>(type)) + ")";
    }
}

std::string to_string(GgmlTensorType type) {
    switch (type) {
    case GgmlTensorType::F32:
        return "F32";
    case GgmlTensorType::F16:
        return "F16";
    case GgmlTensorType::Q4_0:
        return "Q4_0";
    case GgmlTensorType::Q4_1:
        return "Q4_1";
    case GgmlTensorType::Q5_0:
        return "Q5_0";
    case GgmlTensorType::Q5_1:
        return "Q5_1";
    case GgmlTensorType::Q8_0:
        return "Q8_0";
    case GgmlTensorType::Q8_1:
        return "Q8_1";
    case GgmlTensorType::Q2_K:
        return "Q2_K";
    case GgmlTensorType::Q3_K:
        return "Q3_K";
    case GgmlTensorType::Q4_K:
        return "Q4_K";
    case GgmlTensorType::Q5_K:
        return "Q5_K";
    case GgmlTensorType::Q6_K:
        return "Q6_K";
    case GgmlTensorType::Q8_K:
        return "Q8_K";
    case GgmlTensorType::IQ2_XXS:
        return "IQ2_XXS";
    case GgmlTensorType::IQ2_XS:
        return "IQ2_XS";
    case GgmlTensorType::IQ3_XXS:
        return "IQ3_XXS";
    case GgmlTensorType::IQ1_S:
        return "IQ1_S";
    case GgmlTensorType::IQ4_NL:
        return "IQ4_NL";
    case GgmlTensorType::IQ3_S:
        return "IQ3_S";
    case GgmlTensorType::IQ2_S:
        return "IQ2_S";
    case GgmlTensorType::IQ4_XS:
        return "IQ4_XS";
    case GgmlTensorType::I8:
        return "I8";
    case GgmlTensorType::I16:
        return "I16";
    case GgmlTensorType::I32:
        return "I32";
    case GgmlTensorType::I64:
        return "I64";
    case GgmlTensorType::F64:
        return "F64";
    case GgmlTensorType::IQ1_M:
        return "IQ1_M";
    case GgmlTensorType::BF16:
        return "BF16";
    case GgmlTensorType::TQ1_0:
        return "TQ1_0";
    case GgmlTensorType::TQ2_0:
        return "TQ2_0";
    default:
        return "UNKNOWN(" + std::to_string(static_cast<std::uint32_t>(type)) + ")";
    }
}

std::string summarize_metadata_value(const GgufMetadataValue& value, std::size_t max_array_items) {
    std::ostringstream output;

    switch (value.type) {
    case GgufMetadataType::Uint8:
    case GgufMetadataType::Uint16:
    case GgufMetadataType::Uint32:
    case GgufMetadataType::Uint64:
        output << value.unsigned_value;
        break;
    case GgufMetadataType::Int8:
    case GgufMetadataType::Int16:
    case GgufMetadataType::Int32:
    case GgufMetadataType::Int64:
        output << value.signed_value;
        break;
    case GgufMetadataType::Float32:
    case GgufMetadataType::Float64:
        output << value.floating_value;
        break;
    case GgufMetadataType::Bool:
        output << (value.bool_value ? "true" : "false");
        break;
    case GgufMetadataType::String: {
        constexpr std::size_t kMaxDisplayChars = 96;
        output << '"' << escaped_preview(value.string_value, kMaxDisplayChars) << '"';
        break;
    }
    case GgufMetadataType::Array: {
        output << "array<" << to_string(value.array_element_type) << ">[" << value.array_values.size() << "]";
        if (!value.array_values.empty() && max_array_items > 0) {
            output << " {";
            const std::size_t shown = std::min(max_array_items, value.array_values.size());
            for (std::size_t i = 0; i < shown; ++i) {
                if (i > 0) {
                    output << ", ";
                }
                output << summarize_metadata_value(value.array_values[i], 0);
            }
            if (shown < value.array_values.size()) {
                output << ", ...";
            }
            output << '}';
        }
        break;
    }
    default:
        output << "<unsupported " << to_string(value.type) << ">";
        break;
    }

    return output.str();
}

} // namespace rifle
