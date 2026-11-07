## 2024-11-07 - Remove string allocations in happy path
**Learning:** String concatenations (e.g., `field_name + ".size"`) on the "happy path" of parsing logic incur measurable performance overhead (heap allocations) especially on large models with thousands of tensors.
**Action:** Defer all `std::string` allocations and concatenations to the error handling path (e.g., inside the `throw` statement) by passing default parameter strings to prevent unnecessary allocations on successful reads.
