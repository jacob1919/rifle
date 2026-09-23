## 2024-10-26 - [Avoid unnecessary std::string allocations on hot path by utilizing std::string_view]
**Learning:** High-frequency parsing paths in C++ often suffer from performance degradation due to implicit  allocations when string literals are passed to functions accepting  and concatenated for error contexts.
**Action:** Use  for string parameters on the hot path and defer all  concatenations exclusively to the error handling path (e.g., inside the  statement) to avoid unnecessary heap allocations during normal execution.
## 2024-10-26 - [Avoid unnecessary std::string allocations on hot path by utilizing std::string_view]
**Learning:** High-frequency parsing paths in C++ often suffer from performance degradation due to implicit `std::string` allocations when string literals are passed to functions accepting `const std::string&` and concatenated for error contexts.
**Action:** Use `std::string_view` for string parameters on the hot path and defer all `std::string` concatenations exclusively to the error handling path (e.g., inside the `throw` statement) to avoid unnecessary heap allocations during normal execution.
