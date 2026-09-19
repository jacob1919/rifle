## 2024-10-25 - Avoid heap allocations during GGUF parsing
**Learning:** String concatenations (e.g., `field_name + ".size"`) on high-frequency parsing paths cause excessive heap allocations and instruction overhead.
**Action:** Defer string concatenations to the error handling path (e.g., inside `throw`) and prefer `std::string_view` over `const std::string&` for parameter passing in internal parsing helpers.
