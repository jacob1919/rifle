
## 2024-05-14 - String Allocation in Happy Path
**Learning:** Found a performance bottleneck where `std::string` was being constructed and concatenated in high-frequency parsing functions (like `read_string`, `read_little_endian`, etc) during the happy path just to build error context like `field_name + ".size"`.
**Action:** Defer all string allocation to the error path by using `std::string_view` parameters and passing `suffix` variables down. This avoids string memory allocations and zero initialization unless an error occurs.
