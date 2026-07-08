## 2024-05-18 - Prevent Heap Allocations in High-Frequency Parsing Paths
**Learning:** Passing string literals (like "version" or "tensor_count") to `const std::string&` parameters in frequently called parser methods forces a heap allocation for each call, significantly slowing down parsing.
**Action:** Always prefer `std::string_view` over `const std::string&` for string arguments, especially in internal utility functions. Only explicitly convert to `std::string` when absolutely necessary, such as string concatenation for exception messages.
