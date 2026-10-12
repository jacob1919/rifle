## 2024-05-24 - Remove string allocations in happy path
**Learning:** `std::string` concatenation on the hot path in C++ (like `field_name + ".size"`) causes unnecessary heap allocations that significantly bloat instruction counts, even if the result is only needed for error messages.
**Action:** Always prefer passing `std::string_view` for string parameters in parsing/hot paths and defer any string concatenation to the error handling path (e.g., inside the `throw` statement) to avoid allocations during normal execution.
