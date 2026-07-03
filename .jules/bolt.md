## 2024-05-15 - [Avoid string constructions for error contexts in loops]
**Learning:** In high-frequency parsing loops, using `const std::string&` for an error context parameter (like field name) causes unnecessary `std::string` allocations whenever a string literal is passed.
**Action:** Prefer `std::string_view` for string parameters in high-frequency parsing paths to prevent heap allocations from string literals. Only construct `std::string` on the actual error path where an exception is thrown.
