
## $(date +%Y-%m-%d) - Remove string allocations in happy path
**Learning:** In C++ parsers handling high-frequency byte reads, constructing or concatenating `std::string` objects (like for error messages or parameter passing) on the "happy path" introduces significant heap allocation overhead. Avoiding these by passing `std::string_view` and optional suffix parameters directly, and deferring string concatenation to the error path (e.g. `throw`), avoids allocations.
**Action:** When optimizing hot loops in C++ parsers, prefer passing `std::string_view` down the call stack instead of `std::string`. Defer all string constructions and concatenations to the error handling paths. Also, avoid zero-initializing arrays or strings before immediately overwriting them with file IO.
