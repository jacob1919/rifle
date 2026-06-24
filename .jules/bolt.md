## 2024-06-24 - Defer String Construction for Logging/Errors

**Learning:**
In C++ parsing loops (like GGUF metadata/tensor parsers), constructing std::string via concatenation (e.g. `std::string(field_name) + ".size"`) on the hot path purely for error context creates immense unnecessary allocations. For this codebase, it accounted for nearly half the total instructions.

**Action:**
Pass prefix and suffix context strings separately via `std::string_view` to low-level readers (`read_exact`, `read_little_endian`). Only construct the final concatenated `std::string` inside the error path (`throw std::runtime_error()`). Use default arguments (`std::string_view suffix = ""`) to keep call sites clean.
