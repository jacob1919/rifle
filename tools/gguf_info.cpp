#include "core/gguf_parser.h"

#include <algorithm>
#include <exception>
#include <iostream>
#include <string_view>

namespace {

bool has_flag(int argc, char** argv, std::string_view flag) {
    for (int i = 2; i < argc; ++i) {
        if (std::string_view(argv[i]) == flag) {
            return true;
        }
    }
    return false;
}

void print_dimensions(const std::vector<std::uint64_t>& dimensions) {
    std::cout << '[';
    for (std::size_t i = 0; i < dimensions.size(); ++i) {
        if (i > 0) {
            std::cout << ", ";
        }
        std::cout << dimensions[i];
    }
    std::cout << ']';
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: rifle-gguf-info <model.gguf> [--all]\n";
        return 2;
    }

    try {
        const rifle::GgufParser parser(argv[1]);
        const rifle::GgufHeader& header = parser.header();
        const bool show_all = has_flag(argc, argv, "--all");

        std::cout << "GGUF file: " << parser.path() << '\n';
        std::cout << "version: " << header.version << '\n';
        std::cout << "metadata_kv_count: " << header.metadata_kv_count << '\n';
        std::cout << "tensor_count: " << header.tensor_count << '\n';
        std::cout << "tensor_data_offset: " << parser.tensor_data_offset() << '\n';

        const std::size_t metadata_limit = show_all ? parser.metadata().size() : std::min<std::size_t>(parser.metadata().size(), 32);
        std::cout << "\nmetadata:\n";
        for (std::size_t i = 0; i < metadata_limit; ++i) {
            const rifle::GgufMetadataEntry& entry = parser.metadata()[i];
            std::cout << "  " << entry.key << " ("
                      << rifle::to_string(entry.value.type) << "): "
                      << rifle::summarize_metadata_value(entry.value) << '\n';
        }
        if (metadata_limit < parser.metadata().size()) {
            std::cout << "  ... " << (parser.metadata().size() - metadata_limit) << " more metadata entries. Use --all to print all.\n";
        }

        const std::size_t tensor_limit = show_all ? parser.tensors().size() : std::min<std::size_t>(parser.tensors().size(), 32);
        std::cout << "\ntensors:\n";
        for (std::size_t i = 0; i < tensor_limit; ++i) {
            const rifle::GgufTensorInfo& tensor = parser.tensors()[i];
            std::cout << "  " << tensor.name << " "
                      << rifle::to_string(tensor.type) << " dims=";
            print_dimensions(tensor.dimensions);
            std::cout << " offset=" << tensor.offset << '\n';
        }
        if (tensor_limit < parser.tensors().size()) {
            std::cout << "  ... " << (parser.tensors().size() - tensor_limit) << " more tensors. Use --all to print all.\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "rifle-gguf-info: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
