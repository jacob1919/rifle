## 2024-07-18 - Replacing const std::string& with std::string_view for parameters
**Learning:** In high-frequency parsing paths, taking string parameters as `const std::string&` causes excessive heap allocations when string literals are passed, because they must be converted to temporary `std::string` objects. Using `std::string_view` instead avoids this overhead.
**Action:** Use `std::string_view` for string parameters that are only read, especially in internal helper functions within parsing loops.
