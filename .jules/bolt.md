## 2024-05-18 - std::string_view on Hot Paths
**Learning:** In C++ inference parsers, repeatedly creating `std::string` objects on the hot path (like reading millions of metadata elements) via implicit conversions or concatenation for error messages creates a massive allocation bottleneck.
**Action:** Always prefer `std::string_view` for string parameters in high-frequency parsing paths. Defer string allocations (like concatenating error context `.size`) entirely to the error-handling block.
