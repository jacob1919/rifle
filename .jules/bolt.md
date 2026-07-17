## 2024-05-24 - Use std::string_view for frequent parser paths
**Learning:** Passing `const std::string&` as a field name parameter in high-frequency parsing paths (like `read_exact` and `read_little_endian` when reading GGUF models) causes significant string-literal heap allocations.
**Action:** Use `std::string_view` for string parameters in these frequent parsing paths, explicitly converting back to `std::string` only when throwing exceptions.
