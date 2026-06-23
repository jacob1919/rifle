#pragma once

#include "core/gguf_parser.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace rifle {

enum class TensorRole {
    Unknown,
    Weight,
    Activation,
    KvCache,
};

struct TensorShape {
    std::vector<std::uint64_t> dimensions;

    [[nodiscard]] std::size_t rank() const noexcept;
    [[nodiscard]] std::uint64_t element_count() const;
};

struct TensorDesc {
    std::string name;
    TensorShape shape;
    GgmlTensorType storage_type = GgmlTensorType::F32;
    std::uint32_t block_size = 1;
    std::uint32_t type_size = 4;
    bool quantized = false;
    TensorRole role = TensorRole::Unknown;
    std::uint64_t element_count = 0;
    std::uint64_t byte_size = 0;
    std::uint64_t file_offset = 0;
};

[[nodiscard]] TensorDesc make_tensor_desc(
    const GgufTensorInfo& tensor,
    std::uint64_t tensor_data_offset,
    TensorRole role = TensorRole::Weight
);

[[nodiscard]] std::string to_string(TensorRole role);
[[nodiscard]] std::string shape_to_string(const TensorShape& shape);

} // namespace rifle

