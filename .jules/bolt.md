## 2023-10-24 - Zero-initialization is often redundant in explicit read buffers
**Learning:** In C++ file parsing buffers (like `std::array` or `std::string` used directly with `istream::read`), zero-initializing the buffer (e.g., `std::array<unsigned char, N> bytes{};`) is wasted CPU work if the `read` operation enforces reading exactly `N` bytes or throwing. The buffer gets completely overwritten immediately.
**Action:** Omit zero-initialization for deterministic, fully-overwritten input buffers on high-frequency paths.

## 2023-10-24 - String concatenations in exception paths
**Learning:** String concatenations (e.g., `field_name + ".size"`) used only to build informative error messages in exceptions were forcing heap allocations on the happy path. This forces thousands of tiny allocations when parsing large files with many fields like GGUF.
**Action:** Shift string concatenations strictly to the error handling path (e.g., inside the `throw` statement) by passing default suffix parameters and using `std::string_view`, thus avoiding the allocation penalty entirely when parsing succeeds.
