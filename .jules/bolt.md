## 2024-10-29 - Removing string allocations on happy path
**Learning:** In C++ parsing code (like GGUF parsing), appending suffixes to string arguments (e.g. `field_name + ".size"`) on the happy path forces a heap allocation for every read operation. This overhead accumulates quickly when parsing large files with many fields.
**Action:** Defer all string allocations and concatenations strictly to the error handling path (e.g., inside the `throw` statement) by passing arguments as `std::string_view` and providing optional `suffix` parameters that only get concatenated when an error occurs.
