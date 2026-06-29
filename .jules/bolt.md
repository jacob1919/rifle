
## $(date +%Y-%m-%d) - Prevent String Allocations on Hot Paths
**Learning:** Passing `const std::string&` to deeply nested, high-frequency parsing functions (like `read_little_endian` or `read_exact`) causes massive heap allocations when the arguments are constructed at the call site (e.g., string literals concatenated with variables).
**Action:** Always prefer `std::string_view` for read-only string parameters, and avoid dynamically constructing strings (like field names or error message components) on the happy path. Delay string construction until the error path where an exception is actually thrown.
