
## 2023-11-01 - Deferred Error String Construction
**Learning:** In C++ parsers that iterate heavily through binary files, constructing `std::string` objects on the "happy path" (e.g., passing `"field.size"` dynamically to a read function just for context in case it throws an error) causes massive heap allocation overhead.
**Action:** Always prefer passing `std::string_view` for field names down the call chain. If concatenation is required for error reporting, add a `suffix` parameter (`std::string_view`) with a default empty value, and perform the actual string concatenation (`std::string(field_name) + std::string(suffix)`) ONLY inside the `throw` statement.
