## 2024-11-04 - Optimize String Operations on Happy Path
**Learning:** Constructing strings for context inside happy paths (like parsing fields that succeed millions of times) adds measurable memory allocation overhead.
**Action:** Always defer string allocation/concatenation to the error handling path (e.g. `throw`). Use string_views for passing references on the happy path.
