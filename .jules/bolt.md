## 2024-10-22 - Avoid string allocations on happy path
**Learning:** Using `std::string(field_name) + ".size"` directly in function arguments (e.g., `read_little_endian`) triggers expensive heap allocations for every read operation.
**Action:** Replace string concatenations on the happy path with `std::string_view` arguments (e.g., `std::string_view suffix = ""`) and defer concatenation strictly to the error handling path (e.g., inside the `throw` statement). This prevents allocations when parsing is successful.
