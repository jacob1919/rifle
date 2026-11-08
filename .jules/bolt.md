## 2024-11-08 - Optimize string allocations on GGUF parsing hot path
**Learning:** In the GGUF parsing hot path, a large number of allocations occurred because `read_little_endian` and `read_exact` accepted `const std::string& field_name`, causing implicit conversions and string concatenations like `std::string(field_name) + ".size"`.
**Action:** Always avoid `std::string` allocations and concatenation on the happy path. Replace with `std::string_view` and optional `suffix` parameters to defer allocations to the error handling path (i.e. inside `throw`).
