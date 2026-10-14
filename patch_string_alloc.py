import re

with open('src/core/gguf_parser.cpp', 'r') as f:
    content = f.read()

# 1. Update read_exact
content = content.replace(
    "void read_exact(std::istream& input, char* destination, std::streamsize size, const std::string& field_name) {",
    "void read_exact(std::istream& input, char* destination, std::streamsize size, std::string_view field_name, std::string_view suffix = \"\") {"
)
content = content.replace(
    "throw std::runtime_error(\"Failed to read GGUF field: \" + field_name);",
    "throw std::runtime_error(\"Failed to read GGUF field: \" + std::string(field_name) + std::string(suffix));"
)

# 2. Update read_little_endian
content = content.replace(
    "T read_little_endian(std::istream& input, const std::string& field_name) {",
    "T read_little_endian(std::istream& input, std::string_view field_name, std::string_view suffix = \"\") {"
)
content = content.replace(
    "std::array<unsigned char, sizeof(T)> bytes{};\n    read_exact(input, reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()), field_name);",
    "std::array<unsigned char, sizeof(T)> bytes;\n    read_exact(input, reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()), field_name, suffix);"
)

# 3. Update read_string
content = content.replace(
    "const std::uint64_t size = read_little_endian<std::uint64_t>(input, std::string(field_name) + \".size\");",
    "const std::uint64_t size = read_little_endian<std::uint64_t>(input, field_name, \".size\");"
)
content = content.replace(
    "std::string value(static_cast<std::size_t>(size), '\\0');",
    "std::string value;\n    value.resize(static_cast<std::size_t>(size));"
)
content = content.replace(
    "read_exact(input, value.data(), static_cast<std::streamsize>(value.size()), std::string(field_name) + \".bytes\");",
    "read_exact(input, value.data(), static_cast<std::streamsize>(value.size()), field_name, \".bytes\");"
)

# 4. Update read_float32
content = content.replace(
    "float read_float32(std::istream& input, const std::string& field_name) {",
    "float read_float32(std::istream& input, std::string_view field_name) {"
)

# 5. Update read_float64
content = content.replace(
    "double read_float64(std::istream& input, const std::string& field_name) {",
    "double read_float64(std::istream& input, std::string_view field_name) {"
)

with open('src/core/gguf_parser.cpp', 'w') as f:
    f.write(content)
