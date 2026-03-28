# Pulse Pro – Copilot Instructions

Pulse Pro is a **professional-grade, broadcast-quality audio/video/image application** built with Qt6 and C++20 targeting macOS, with cross-platform intent.

## Graphics & Performance

- Always prefer high-performance Qt graphics primitives over basic ones:
  - `QGraphicsVideoItem` over `QVideoWidget`
  - `QGraphicsScene`/`QGraphicsView` for compositing and overlay layers
  - `QVideoFrameInput` + native frame pipelines for capture-side transforms (crop, scale, color)
  - HW-accelerated paths (VideoToolbox, Metal) over CPU-side frame processing
  - Avoid `QVideoFrame::toImage()` in hot paths — it forces a GPU→CPU download
- Qt's RHI is the rendering backend; keep frames GPU-resident wherever possible:
  - macOS: Metal (HW encode/decode via VideoToolbox)
  - Linux: Vulkan (HW encode/decode via VA-API or NVENC)
  - Windows: Direct3D 11/12 (HW encode/decode via Media Foundation / DXVA2)

## Architecture

Currently it's...
- Capture pipeline: `QScreenCapture`/`QWindowCapture` → `QMediaCaptureSession` → `QMediaRecorder` + `QGraphicsVideoItem`
- Capture-side transforms (affects both preview and recording): use `QVideoFrameInput`
- Display-side overlays (preview only, not recorded): use `QGraphicsItem` layers in the scene graph
- Output format: MP4 + H.264 via `QMediaFormat::MPEG4` with `resolveForEncoding()`

## Code Style

- **Getters/setters**: Qt convention — `name()` / `setName(...)`, never `getName()`
- **Private members**: underscore prefix — `_variableName`
- **Classes**: `PascalCase`
- **Functions/methods**: `camelCase`
- **Constants/enums**: `PascalCase` for enum values, `SCREAMING_SNAKE` for macros only
- **Includes**: Qt headers before project headers; alphabetically within each group
- Prefer `const` references for non-trivial parameters; pass value-type Qt classes by value

## Documentation

- Use Doxygen `/** ... */` block comments on all public and protected declarations in headers
- Format:
  ```cpp
  /**
      Brief one-sentence description.
  
      @param paramName Description of parameter.
      @param paramName2 Description of parameter.
      @returns Description of return value.
      @note Any important usage constraints or threading notes.
  */
  ```
- Inline `//` comments are fine for implementation details inside `.cpp` files
- Do not duplicate the Doxygen comment in the `.cpp` — header only

## Code Standards

- C++20, Qt6 latest stable
- Qt ownership model: parent QObjects on construction where it makes sense
- Prefer value-type Qt classes (`QMediaFormat`, `QSize`, etc.) as stack locals — they copy safely
- No CPU-side frame transforms in production paths; flag with `// TODO: GPU path needed`
