## 2024-05-17 - Prevent heap allocations using std::string_view
**Learning:** High-frequency parsing paths like `read_exact` and `read_little_endian` cause numerous unnecessary heap allocations when taking `const std::string&` parameters, as literals are implicitly converted to strings.
**Action:** Prefer `std::string_view` over `const std::string&` for string parameters in these parsing paths to prevent heap allocations. Convert to `std::string` explicitly when using `operator+` for exception messages.
