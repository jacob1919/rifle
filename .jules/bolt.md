## 2026-10-06 - Remove String Allocations in Happy Path
**Learning:** Passing string literals (e.g., ".size") to functions expecting `const std::string&` forces implicit heap allocations. In high-frequency parsing paths, string concatenations should be deferred entirely to the error handling path.
**Action:** Use `std::string_view` for string parameters in parsing helpers and introduce a `suffix` parameter to delay concatenations until an exception is thrown.
