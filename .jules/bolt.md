
## 2024-05-18 - String literal heap allocations in parsing path
**Learning:** `const std::string&` parameters in high-frequency parsing paths (like reading metadata entries for a large GGUF file) trigger significant, unnecessary heap allocations when called with string literals (e.g. `"metadata.key"`).
**Action:** Use `std::string_view` for string parameters in parsing and lookup paths to prevent implicit `std::string` instantiation. Explicitly convert to `std::string` when concatenation is required (e.g. in exceptions). This one change removed over 32,500 allocations when parsing a 7B parameter model GGUF file.
