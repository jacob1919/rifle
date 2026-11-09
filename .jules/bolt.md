## 2024-05-18 - Optimize `std::array` memory zero-initialization overhead
**Learning:** When reading exact bytes into a stack-allocated buffer (e.g., `std::array`) that will be immediately and fully overwritten, omitting zero-initialization (`std::array<unsigned char, N> bytes;` instead of `bytes{}`) saves redundant CPU cycles.
**Action:** Remove zero-initialization for buffers that are directly overwritten on their happy paths.
