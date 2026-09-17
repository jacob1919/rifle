#include "core/gguf_parser.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <fstream>
#include <limits>
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

std::uint64_t checked_multiply(std::uint64_t lhs, std::uint64_t rhs, std::string_view context) {
    if (lhs != 0 && rhs > std::numeric_limits<std::uint64_t>::max() / lhs) {
        throw std::overflow_error("uint64 overflow while computing " + std::string(context));
    }
    return lhs * rhs;
}

std::uint64_t checked_add(std::uint64_t lhs, std::uint64_t rhs, std::string_view context) {
    if (rhs > std::numeric_limits<std::uint64_t>::max() - lhs) {
        throw std::overflow_error("uint64 overflow while computing " + std::string(context));
    }
    return lhs + rhs;
}

void read_exact(std::istream& input, char* destination, std::streamsize size, std::string_view field_name, std::string_view suffix = "") {
    if (!input.read(destination, size)) {
        throw std::runtime_error("Failed to read GGUF field: " + std::string(field_name) + std::string(suffix));
    }
}

template <typename T>
T read_little_endian(std::istream& input, std::string_view field_name, std::string_view suffix = "") {
    static_assert(std::is_unsigned_v<T>, "GGUF little-endian helper expects unsigned integer types");

    std::array<unsigned char, sizeof(T)> bytes{};
    read_exact(input, reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()), field_name, suffix);

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
    const std::uint64_t size = read_little_endian<std::uint64_t>(input, field_name, ".size");
    if (size > kMaxStringBytes) {
        throw std::runtime_error("GGUF string is too large: " + std::to_string(size));
    }

    std::string value(static_cast<std::size_t>(size), '\0');
    if (!value.empty()) {
        read_exact(input, value.data(), static_cast<std::streamsize>(value.size()), field_name, ".bytes");
    }
    return value;
}

float read_float32(std::istream& input, std::string_view field_name) {
    const std::uint32_t raw = read_little_endian<std::uint32_t>(input, field_name);
    return std::bit_cast<float>(raw);
}

double read_float64(std::istream& input, std::string_view field_name) {
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

const GgufTensorInfo* GgufParser::find_tensor(std::string_view name) const noexcept {
    for (const GgufTensorInfo& tensor : info_.tensors) {
        if (tensor.name == name) {
            return &tensor;
        }
    }
    return nullptr;
}

std::vector<std::byte> GgufParser::read_tensor_bytes(const GgufTensorInfo& tensor) const {
    const std::uint64_t size = tensor.byte_size();
    if (size > static_cast<std::uint64_t>(std::numeric_limits<std::streamsize>::max())) {
        throw std::runtime_error("Tensor is too large to read with std::istream: " + tensor.name);
    }

    std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    if (bytes.empty()) {
        return bytes;
    }

    std::ifstream input(path_, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Failed to open GGUF file: " + path_);
    }

    const std::uint64_t file_offset = tensor.absolute_offset(info_.tensor_data_offset);
    input.seekg(static_cast<std::streamoff>(file_offset), std::ios::beg);
    if (!input) {
        throw std::runtime_error("Failed to seek tensor data: " + tensor.name);
    }

    read_exact(input, reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()), "tensor.data.", tensor.name);
    return bytes;
}

std::optional<std::vector<std::byte>> GgufParser::read_tensor_bytes(std::string_view tensor_name) const {
    const GgufTensorInfo* tensor = find_tensor(tensor_name);
    if (tensor == nullptr) {
        return std::nullopt;
    }
    return read_tensor_bytes(*tensor);
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
    case GgmlTensorType::MXFP4:
        return "MXFP4";
    case GgmlTensorType::NVFP4:
        return "NVFP4";
    case GgmlTensorType::Q1_0:
        return "Q1_0";
    default:
        return "UNKNOWN(" + std::to_string(static_cast<std::uint32_t>(type)) + ")";
    }
}

GgmlTypeInfo ggml_type_info(GgmlTensorType type) {
    switch (type) {
    case GgmlTensorType::F32:
        return {"f32", 1, 4, false};
    case GgmlTensorType::F16:
        return {"f16", 1, 2, false};
    case GgmlTensorType::Q4_0:
        return {"q4_0", 32, 18, true};
    case GgmlTensorType::Q4_1:
        return {"q4_1", 32, 20, true};
    case GgmlTensorType::Q5_0:
        return {"q5_0", 32, 22, true};
    case GgmlTensorType::Q5_1:
        return {"q5_1", 32, 24, true};
    case GgmlTensorType::Q8_0:
        return {"q8_0", 32, 34, true};
    case GgmlTensorType::Q8_1:
        return {"q8_1", 32, 36, true};
    case GgmlTensorType::Q2_K:
        return {"q2_K", 256, 84, true};
    case GgmlTensorType::Q3_K:
        return {"q3_K", 256, 110, true};
    case GgmlTensorType::Q4_K:
        return {"q4_K", 256, 144, true};
    case GgmlTensorType::Q5_K:
        return {"q5_K", 256, 176, true};
    case GgmlTensorType::Q6_K:
        return {"q6_K", 256, 210, true};
    case GgmlTensorType::Q8_K:
        return {"q8_K", 256, 292, true};
    case GgmlTensorType::IQ2_XXS:
        return {"iq2_xxs", 256, 66, true};
    case GgmlTensorType::IQ2_XS:
        return {"iq2_xs", 256, 74, true};
    case GgmlTensorType::IQ3_XXS:
        return {"iq3_xxs", 256, 98, true};
    case GgmlTensorType::IQ1_S:
        return {"iq1_s", 256, 50, true};
    case GgmlTensorType::IQ4_NL:
        return {"iq4_nl", 32, 18, true};
    case GgmlTensorType::IQ3_S:
        return {"iq3_s", 256, 110, true};
    case GgmlTensorType::IQ2_S:
        return {"iq2_s", 256, 82, true};
    case GgmlTensorType::IQ4_XS:
        return {"iq4_xs", 256, 136, true};
    case GgmlTensorType::I8:
        return {"i8", 1, 1, false};
    case GgmlTensorType::I16:
        return {"i16", 1, 2, false};
    case GgmlTensorType::I32:
        return {"i32", 1, 4, false};
    case GgmlTensorType::I64:
        return {"i64", 1, 8, false};
    case GgmlTensorType::F64:
        return {"f64", 1, 8, false};
    case GgmlTensorType::IQ1_M:
        return {"iq1_m", 256, 56, true};
    case GgmlTensorType::BF16:
        return {"bf16", 1, 2, false};
    case GgmlTensorType::TQ1_0:
        return {"tq1_0", 256, 54, true};
    case GgmlTensorType::TQ2_0:
        return {"tq2_0", 256, 66, true};
    case GgmlTensorType::MXFP4:
        return {"mxfp4", 32, 17, true};
    case GgmlTensorType::NVFP4:
        return {"nvfp4", 64, 36, true};
    case GgmlTensorType::Q1_0:
        return {"q1_0", 128, 18, true};
    default:
        throw std::runtime_error("Unknown GGML tensor type: " + to_string(type));
    }
}

bool is_known_ggml_type(GgmlTensorType type) {
    try {
        (void)ggml_type_info(type);
        return true;
    } catch (const std::runtime_error&) {
        return false;
    }
}

std::uint64_t ggml_row_size(GgmlTensorType type, std::uint64_t row_elements) {
    const GgmlTypeInfo info = ggml_type_info(type);
    if (info.block_size == 0 || info.type_size == 0) {
        throw std::runtime_error("Invalid GGML type size metadata for " + to_string(type));
    }
    if (row_elements % info.block_size != 0) {
        throw std::runtime_error(
            "Tensor row element count " + std::to_string(row_elements) +
            " is not divisible by block size " + std::to_string(info.block_size) +
            " for " + to_string(type)
        );
    }
    return checked_multiply(row_elements / info.block_size, info.type_size, "GGML row size");
}

std::uint64_t GgufTensorInfo::element_count() const {
    std::uint64_t count = 1;
    for (std::uint64_t dimension : dimensions) {
        count = checked_multiply(count, dimension, "GGUF tensor element count");
    }
    return count;
}

std::uint64_t GgufTensorInfo::byte_size() const {
    if (dimensions.empty()) {
        return 0;
    }

    std::uint64_t rows = 1;
    for (std::size_t i = 1; i < dimensions.size(); ++i) {
        rows = checked_multiply(rows, dimensions[i], "GGUF tensor row count");
    }
    return checked_multiply(ggml_row_size(type, dimensions[0]), rows, "GGUF tensor byte size");
}

std::uint64_t GgufTensorInfo::absolute_offset(std::uint64_t tensor_data_offset) const {
    return checked_add(tensor_data_offset, offset, "GGUF tensor absolute offset");
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
// Optimization: Replaced std::string concatenation on the hot path with std::string_view and deferred concatenations to the error handling path to avoid unnecessary heap allocations.
