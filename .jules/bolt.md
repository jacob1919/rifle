## 2026-08-03 - Prevent Heap Allocations in High-Frequency Parsing Paths
**Learning:** Passing const std::string& for field names in high-frequency parsing paths causes unnecessary string constructions when called with string literals.
**Action:** Use std::string_view for string parameters in internal helper functions to prevent heap allocations, explicitly converting to std::string only when needed.
