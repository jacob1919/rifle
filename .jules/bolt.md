## 2024-05-23 - String allocations in high-frequency parsing paths
**Learning:** String concatenations (e.g., `field_name + ".size"`) on the "happy path" of parsing functions cause massive instruction bloat and unnecessary heap allocations. By passing `std::string_view` down to helper functions and deferring string construction entirely to the error/throw path, we eliminate this overhead.
**Action:** When writing or optimizing parsing logic, ensure that diagnostic strings and object allocations are strictly deferred to error-handling blocks.
