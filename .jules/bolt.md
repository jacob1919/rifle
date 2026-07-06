## 2024-05-24 - Prefer string_view over const string& for high-frequency string parameters
**Learning:** In high-frequency parsing paths, passing string literals as `const std::string&` incurs a heap allocation (or small-string optimization overhead) every time the function is called.
**Action:** Replace `const std::string&` with `std::string_view` for string parameters in these paths to prevent unnecessary allocations, while being careful to properly convert the `string_view` to `std::string` when throwing exceptions using string concatenation.
