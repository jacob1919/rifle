## 2024-08-05 - Avoid string copies in high-frequency paths
**Learning:** Prefer std::string_view over const std::string& for string parameters in high-frequency parsing paths to prevent heap allocations from string literals.
**Action:** Use std::string_view for read-only string parameters and explicitly convert to std::string when concatenation is required.
