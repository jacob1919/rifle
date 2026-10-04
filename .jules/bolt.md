## 2026-10-04 - Remove string allocations in happy path
**Learning:** String concatenation in the 'happy path' (e.g., constructing error contexts like `field_name + ".size"` before they are needed) forces heap allocations, which drastically inflates execution time in high-frequency parsing loops.
**Action:** Use a default `suffix` parameter (e.g., `std::string_view suffix = ""`) to defer string concatenation to the error handling path (inside `throw`). Also, use `string::resize` instead of zero-initializing string buffers to save CPU cycles.
