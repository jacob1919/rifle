## 2024-05-18 - [Use string_view for high-frequency string parameters in parsing]
**Learning:** In C++ parsing codebases, using `const std::string&` for field names or lookup keys causes significant heap allocations when invoked with string literals (e.g., `read_little_endian(input, "magic")`), because a temporary `std::string` must be constructed and destroyed for every call.
**Action:** Always prefer `std::string_view` over `const std::string&` for read-only string parameters, especially in high-frequency paths like file parsers where string literals are frequently passed.
