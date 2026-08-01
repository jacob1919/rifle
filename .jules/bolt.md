## 2026-08-01 - String View in Parsing Paths
**Learning:** In C++ parsing code (like GGUF parsing), passing `const std::string&` for small literal string arguments causes unnecessary heap allocations. Replacing them with `std::string_view` significantly reduces allocations.
**Action:** Use `std::string_view` instead of `const std::string&` for string parameters in high-frequency parsing paths.
