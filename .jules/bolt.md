
## 2024-07-26 - String allocation bottleneck in high-frequency parsing paths
**Learning:** Using `const std::string&` for string parameters in high-frequency functions (like `read_little_endian` called 30k+ times during GGUF parsing) causes a huge number of heap allocations from string literal conversions.
**Action:** Prefer `std::string_view` over `const std::string&` for string parameters, especially in parsing loops. This change alone reduced total heap allocations from 98,097 to 65,572 during GGUF inspection. Explicitly convert to `std::string` only when absolutely necessary (e.g., throwing exceptions).
