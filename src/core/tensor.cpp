#include "core/tensor.h"

#include <limits>
#include <sstream>
#include <stdexcept>

namespace rifle {

std::size_t TensorShape::rank() const noexcept {
    return dimensions.size();
}

std::uint64_t TensorShape::element_count() const {
    std::uint64_t count = 1;
    for (std::uint64_t dimension : dimensions) {
        if (count != 0 && dimension > std::numeric_limits<std::uint64_t>::max() / count) {
            throw std::overflow_error("uint64 overflow while computing tensor shape element count");
        }
        count *= dimension;
    }
    return count;
}

TensorDesc make_tensor_desc(const GgufTensorInfo& tensor, std::uint64_t tensor_data_offset, TensorRole role) {
    const GgmlTypeInfo type_info = ggml_type_info(tensor.type);

    TensorDesc desc;
    desc.name = tensor.name;
    desc.shape.dimensions = tensor.dimensions;
    desc.storage_type = tensor.type;
    desc.block_size = type_info.block_size;
    desc.type_size = type_info.type_size;
    desc.quantized = type_info.quantized;
    desc.role = role;
    desc.element_count = tensor.element_count();
    desc.byte_size = tensor.byte_size();
    desc.file_offset = tensor.absolute_offset(tensor_data_offset);
    return desc;
}

std::string to_string(TensorRole role) {
    switch (role) {
    case TensorRole::Unknown:
        return "unknown";
    case TensorRole::Weight:
        return "weight";
    case TensorRole::Activation:
        return "activation";
    case TensorRole::KvCache:
        return "kv_cache";
    default:
        return "unknown";
    }
}

std::string shape_to_string(const TensorShape& shape) {
    std::ostringstream output;
    output << '[';
    for (std::size_t i = 0; i < shape.dimensions.size(); ++i) {
        if (i > 0) {
            output << ", ";
        }
        output << shape.dimensions[i];
    }
    output << ']';
    return output.str();
}

} // namespace rifle
