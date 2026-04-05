# Pulse Pro – Copilot Instructions

Pulse Pro is a **professional-grade, broadcast-quality audio/video/image application** built with Qt6 and C++20. Development follows the principles of **Clean Architecture** (Robert C. Martin) and classic GoF design patterns. You are an expert pro audio/video developer with deep knowledge of real-time media pipelines, broadcast standards, and low-latency systems.

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

### Layer Model (Clean Architecture)
Enforce strict dependency direction: inner layers never depend on outer layers.

```
┌─────────────────────────────────┐
│  UI / Presentation              │  QWidget, QGraphicsScene, MainWindow
│  (frameworks & drivers)         │  TrayController — depends on interface layer
├─────────────────────────────────┤
│  Application / Controllers      │  AppController, Services (SnapshotService…)
│                                 │  Orchestrates use cases; no Qt UI types here
├─────────────────────────────────┤
│  Domain / Use Cases             │  Pure business logic; no Qt, no framework deps
│                                 │  e.g. crop regions, recording state machines
├─────────────────────────────────┤
│  Infrastructure                 │  CaptureEngine, QMediaCaptureSession, codecs
│  (interface adapters)           │  Implements domain interfaces against Qt APIs
└─────────────────────────────────┘
```

- **Depend inward only** — UI and services call into infrastructure/domain; never the reverse.
- **Services** are discrete units of application logic (e.g. `SnapshotService`, `RecordingService`). Each owns one responsibility and communicates via signals or explicit interfaces, not direct coupling.
- **Interfaces (abstract base classes) at boundaries** — domain/use-case layer defines interfaces; infrastructure implements them. This keeps domain logic testable without Qt.
- **`AppController` (or future `AppContext`)** is the composition root: it wires layers together and owns infrastructure lifetimes.

### Design Patterns in Use
- **Service Locator / Registry** — `AppController` registers and vends services; prefer explicit injection over global `qApp` casts.
- **Observer** — Qt signals/slots; prefer typed signals over string-based `SIGNAL()`/`SLOT()`.
- **Strategy** — swap capture sources (`QScreenCapture` vs `QWindowCapture`) at runtime without changing pipeline consumers.
- **Template Method** — base `Service` class defines `initialize()`/`shutdown()` lifecycle hooks; subclasses override.
- **Factory** — use factory functions or `create()` statics for objects with non-trivial construction (e.g. codec configs).

### Current Pipeline
- Capture: `QScreenCapture`/`QWindowCapture` → `QMediaCaptureSession` → `QMediaRecorder` + `QGraphicsVideoItem`
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
