## 2024-05-24 - Prevent heap allocations in high-frequency parsing paths
**Learning:** Passing string literals (e.g., `"metadata.type"`) to `const std::string&` parameters in highly-used internal parsing functions (like `read_exact` and `read_little_endian`) triggers frequent heap allocations because temporary `std::string` objects are created.
**Action:** Prefer `std::string_view` over `const std::string&` for string parameters in high-frequency parsing paths. Convert `std::string_view` to `std::string` explicitly when concatenating strings (e.g., in exception messages).
