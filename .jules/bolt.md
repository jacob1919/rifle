## 2024-06-28 - String literals causing heap allocations
**Learning:** In C++, using `const std::string&` as a parameter type for functions frequently called with string literals (like `"metadata.type"`) causes a hidden heap allocation every time the function is called, as the literal is converted to a `std::string`. This happens frequently in parsing code (e.g. `gguf_parser.cpp`).
**Action:** Always prefer `std::string_view` over `const std::string&` for string parameters in high-frequency paths (like parsers) to prevent unnecessary heap allocations from string literals.
