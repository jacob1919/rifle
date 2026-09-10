## 2024-05-18 - String literal allocation on happy paths
**Learning:** In C++, concatenating strings (e.g. `field_name + ".size"`) on high frequency execution paths like parsing loops forces heap allocations even when the code expects literal string buffers, leading to millions of unnecessary instructions.
**Action:** Use `std::string_view` along with optional default `suffix` parameters on internal helper functions to defer string concatenation solely to error paths.
