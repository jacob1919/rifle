## 2024-04-10 - Avoiding string allocation in gguf_parser
**Learning:** High frequency parsing functions concatenated std::string variables on the hot path just in case of an error message.
**Action:** Use string_view and suffix parameter on hot path, defer std::string concatenation strictly to error paths.
