## 2024-03-24 - Zero-initialization overhead in strings

**Learning:** When preparing a string to receive raw bytes (e.g., via `istream::read`), using `std::string value(size, '\0')` introduces unnecessary CPU cycles due to zero-initialization.

**Action:** Prefer default-constructing and then calling `value.resize(size)` instead of `std::string value(size, '\0');`. In C++23, `resize_and_overwrite` is strictly better, but `resize` is still an improvement over `(size, '\0')`.
