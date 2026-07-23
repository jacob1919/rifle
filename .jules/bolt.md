
## 2024-05-18 - String Parameter Optimization
**Learning:** Using `const std::string&` for string parameters in high-frequency parsing paths causes unnecessary heap allocations from string literals.
**Action:** Always prefer `std::string_view` over `const std::string&` for string parameters to prevent heap allocations from string literals. Explicitly convert `std::string_view` to `std::string` when using `operator+` for string concatenation (e.g., in exception messages).
