## 2026-10-02 - Defer String Concatenation in Hot Paths
**Learning:** Constructing and concatenating strings (e.g., for error context) on the happy path of high-frequency parsing functions (like reading thousands of metadata entries) causes massive heap allocation overhead.
**Action:** Always use `std::string_view` for string parameters on hot paths, and introduce optional `suffix` parameters to defer string concatenation strictly to the error handling path (inside the `throw` statement).
