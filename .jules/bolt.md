## 2024-08-11 - Use std::string_view for parsing fields
**Learning:** High-frequency parsing paths allocating std::string from string literals causes significant overhead.
**Action:** Prefer std::string_view over const std::string& for string parameters in internal parsing helpers, and explicitly convert to std::string only when needed for string concatenation (e.g., exceptions).
