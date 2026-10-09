## 2023-10-09 - Remove string concatenations in happy path
**Learning:** In high-frequency parsing paths, concatenating strings for error context (e.g. \`field_name + ".size"\`) on the happy path causes excessive heap allocations and instruction overhead.
**Action:** Always defer string concatenation to the error handling path. Use \`std::string_view\` parameters with a default \`suffix = ""\` parameter to allow passing context without allocating until an error occurs.
