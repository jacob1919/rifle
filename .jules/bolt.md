## 2026-08-13 - Prevent Heap Allocations in GGUF Parser
**Learning:** String literals passed as `const std::string&` parameters in high-frequency parsing functions (like `read_exact` and `read_little_endian`) cause implicit heap allocations.
**Action:** Use `std::string_view` for string parameters in parsing loops, explicitly casting to `std::string` only on error paths (like throwing exceptions) where performance doesn't matter.
