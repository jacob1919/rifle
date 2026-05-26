#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace rifle {

enum class GgufMetadataType : std::uint32_t {
    Uint8 = 0,
    Int8 = 1,
    Uint16 = 2,
    Int16 = 3,
    Uint32 = 4,
    Int32 = 5,
    Float32 = 6,
    Bool = 7,
    String = 8,
    Array = 9,
    Uint64 = 10,
    Int64 = 11,
    Float64 = 12,
};

enum class GgmlTensorType : std::uint32_t {
    F32 = 0,
    F16 = 1,
    Q4_0 = 2,
    Q4_1 = 3,
    Q5_0 = 6,
    Q5_1 = 7,
    Q8_0 = 8,
    Q8_1 = 9,
    Q2_K = 10,
    Q3_K = 11,
    Q4_K = 12,
    Q5_K = 13,
    Q6_K = 14,
    Q8_K = 15,
    IQ2_XXS = 16,
    IQ2_XS = 17,
    IQ3_XXS = 18,
    IQ1_S = 19,
    IQ4_NL = 20,
    IQ3_S = 21,
    IQ2_S = 22,
    IQ4_XS = 23,
    I8 = 24,
    I16 = 25,
    I32 = 26,
    I64 = 27,
    F64 = 28,
    IQ1_M = 29,
    BF16 = 30,
    TQ1_0 = 34,
    TQ2_0 = 35,
};

struct GgufHeader {
    std::uint32_t version = 0;
    std::uint64_t tensor_count = 0;
    std::uint64_t metadata_kv_count = 0;
};

struct GgufMetadataValue {
    GgufMetadataType type = GgufMetadataType::Uint8;
    GgufMetadataType array_element_type = GgufMetadataType::Uint8;
    std::uint64_t unsigned_value = 0;
    std::int64_t signed_value = 0;
    double floating_value = 0.0;
    bool bool_value = false;
    std::string string_value;
    std::vector<GgufMetadataValue> array_values;
};

struct GgufMetadataEntry {
    std::string key;
    GgufMetadataValue value;
};

struct GgufTensorInfo {
    std::string name;
    std::vector<std::uint64_t> dimensions;
    GgmlTensorType type = GgmlTensorType::F32;
    std::uint64_t offset = 0;
};

struct GgufFileInfo {
    GgufHeader header;
    std::vector<GgufMetadataEntry> metadata;
    std::vector<GgufTensorInfo> tensors;
    std::uint64_t tensor_data_offset = 0;
};

[[nodiscard]] std::string to_string(GgufMetadataType type);
[[nodiscard]] std::string to_string(GgmlTensorType type);
[[nodiscard]] std::string summarize_metadata_value(const GgufMetadataValue& value, std::size_t max_array_items = 6);

class GgufParser {
public:
    explicit GgufParser(std::string path);

    [[nodiscard]] const std::string& path() const noexcept;
    [[nodiscard]] const GgufHeader& header() const noexcept;
    [[nodiscard]] const std::vector<GgufMetadataEntry>& metadata() const noexcept;
    [[nodiscard]] const std::vector<GgufTensorInfo>& tensors() const noexcept;
    [[nodiscard]] std::uint64_t tensor_data_offset() const noexcept;
    [[nodiscard]] const GgufFileInfo& info() const noexcept;
    [[nodiscard]] const GgufMetadataValue* find_metadata(std::string_view key) const noexcept;

    static GgufHeader read_header(const std::string& path);

private:
    static GgufFileInfo parse_file(const std::string& path);

    std::string path_;
    GgufFileInfo info_;
};

} // namespace rifle
