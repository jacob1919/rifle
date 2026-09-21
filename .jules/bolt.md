## 2024-05-23 - Avoid String Allocations on Hot Paths
**Learning:** In C++ codebases like `rifle` that perform extensive parsing, constructing `std::string` objects (e.g., via concatenation) solely for passing context to error-handling paths causes massive overhead due to heap allocations.
**Action:** When a string parameter is used mainly for error reporting (the "cold path"), use `std::string_view` for the parameter and only convert to `std::string` or concatenate strings inside the `throw` or error-handling block to avoid unnecessary allocations on the happy path.
