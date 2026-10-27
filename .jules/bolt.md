## 2023-10-27 - Avoid string allocations on happy paths
**Learning:** Constructing and concatenating strings on the happy path in high-frequency functions (like parsing loops) causes significant performance overhead due to unnecessary allocations. Zero-initializing stack buffers that are immediately overwritten is also a waste of CPU cycles.
**Action:** Use `std::string_view` for passing string references and defer string concatenation (e.g., using a default `suffix` parameter) strictly to the error handling path. Omit zero-initialization for buffers that will be fully overwritten.
