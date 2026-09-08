## 2024-05-18 - Avoid std::string concatenations on parser happy path
**Learning:** `std::string(field_name) + ".size"` in heavily called inner loops (like `read_string` and `read_little_endian` in GGUF metadata parsing) causes massive Ir overhead (20M+ instructions, ~27% of total parsing time) due to temporary heap allocations.
**Action:** Replace `const std::string&` with `std::string_view` for field name parameters, add an optional `std::string_view suffix = ""` parameter for context strings, and push all `std::string` concatenation exclusively onto the error-handling `throw` path.
