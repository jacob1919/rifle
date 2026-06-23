# Rifle

Rifle is a hands-on C++ inference engine project. The current stage focuses on parsing GGUF files before moving into tensor abstractions, graph construction, memory planning, attention kernels, sampling, and quantization.

## Current Scope

- Minimal CMake/VS2022 project.
- GGUF header parsing.
- GGUF metadata parsing.
- GGUF tensor directory parsing.
- GGML tensor storage size calculation.
- Runtime tensor descriptor mapping.
- `rifle-gguf-info` inspection tool.

The parser reads model metadata and tensor descriptors. It can also read raw tensor bytes on demand, but it does not dequantize or execute tensor operations yet.

## Build

```powershell
cmake -S X:\rifle -B X:\rifle\build-vs2022 -G "Visual Studio 17 2022" -A x64
cmake --build X:\rifle\build-vs2022 --config Debug --target rifle-gguf-info
```

## Inspect a GGUF File

```powershell
X:\rifle\build-vs2022\Debug\rifle-gguf-info.exe X:\path\to\model.gguf
```

Print all metadata and tensor entries:

```powershell
X:\rifle\build-vs2022\Debug\rifle-gguf-info.exe X:\path\to\model.gguf --all
```

## Notes

Model files are intentionally ignored by Git. Keep `.gguf`, `.safetensors`, `.onnx`, and other large artifacts outside version control.
