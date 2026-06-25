## 2024-06-25 - GGUF Parser String Allocations
**Learning:** Passing `const std::string&` for field names in the hot path of parsing GGUF files causes excessive string allocations when string literals are passed, significantly increasing instruction counts and memory overhead.
**Action:** Use `std::string_view` for string parameters in parsing hot paths to avoid unnecessary allocations, especially when handling string literals. Always remember to convert `std::string_view` back to `std::string` when concatenating in exception messages.
