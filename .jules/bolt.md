## 2026-08-22 - String Allocation Avoidance in GGUF Parser
**Learning:** High-frequency functions parsing small strings should use `std::string_view` instead of `const std::string&` or temporary string concatenation, especially when those strings are only used on error paths. String concatenation during standard parser paths dramatically increases instruction count via heap allocations.
**Action:** Defer all `std::string` concatenations exclusively to the error path (inside `throw` blocks) and use `std::string_view` for all parameter passing on the hot path.
