## 2024-07-21 - Replace const std::string& with std::string_view for parsing functions
**Learning:** High-frequency parsing paths with string literal arguments for context/field names can result in significant heap allocation overhead if the parameter type is `const std::string&`.
**Action:** Prefer `std::string_view` for read-only string parameters in utility functions (like `read_exact`, `read_little_endian`, etc.). Explicitly construct `std::string` from `std::string_view` when throwing exceptions or concatenating.
