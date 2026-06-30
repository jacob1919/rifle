## 2024-05-18 - String View for Parser Helper Functions
**Learning:** Found a specific performance anti-pattern where frequent `const std::string&` parameters in small parsing helper functions (`read_exact`, `read_little_endian`, etc) caused heap allocations for string literals exceeding SSO (Small String Optimization, typically 15 characters).
**Action:** Use `std::string_view` for string parameters in high-frequency parsing paths to avoid implicit string construction and heap allocations from string literals.
