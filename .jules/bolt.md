## 2026-10-26 - Remove string allocations in high-frequency parsing paths
**Learning:** Constructing error context strings on the happy path in high-frequency C++ parsing functions causes massive unnecessary allocation overhead (~25% of total instructions in this case).
**Action:** Always defer std::string concatenations and allocations to the error handling path (e.g., inside the throw statement) by using std::string_view and default arguments for error context.
