## 2023-10-24 - Avoid string allocations on happy path in parsing
**Learning:** String allocations on the happy path, such as constructing string representations (e.g. `field_name + ".size"`) inside frequently called functions (like `read_string`), cause significant instruction bloat.
**Action:** Avoid constructing or concatenating strings in high-frequency parsing paths unless an error occurs. Pass `std::string_view` along with optional suffixes and defer the full `std::string` creation only to the `throw` blocks where an error is actually raised.
