# MCP Server – Rust dylib Plan

## Overview

A Rust `cdylib` (`libpulse_mcp.dylib`) loaded by `AppController` at startup.
It spins up an in-process HTTP + SSE server (axum + Tokio) that speaks MCP 2.0
to any number of simultaneous clients (VS Code Copilot, Claude Desktop, Cursor,
etc.). The dylib calls back into Qt via a thin C ABI when tools are invoked.

```
MCP client (VS Code / Claude Desktop / Cursor)
    │  HTTP + SSE  →  http://localhost:7341/sse
    ▼
libpulse_mcp.dylib  (Rust, loaded inside PulsePro.app)
    │  C function-pointer callbacks
    ▼
AppController / CaptureEngine  (Qt main thread, QMetaObject::invokeMethod)
```

---

## Crate Layout

```
mcp/
├── Cargo.toml
└── src/
    ├── lib.rs          — extern "C" entry points, Tokio runtime lifecycle
    ├── server.rs       — McpServer impl: tool registration, dispatch
    ├── tools/
    │   ├── screenshot.rs
    │   ├── recording.rs
    │   ├── sources.rs
    │   └── status.rs
    ├── transport.rs    — axum SSE router, /sse and /message endpoints
    └── ffi.rs          — McpCallbacks struct, callback wrappers
```

---

## C ABI  (`mcp/include/mcpplugin.h`)

Shared between C++ and Rust. Pure C — no C++ types cross the boundary.

```c
#pragma once
#include <stdint.h>

/* Opaque pointer back to AppController. */
typedef void* McpCtx;

/* Callback signatures — all called on Qt main thread via invokeMethod. */
typedef void (*McpSnapshotFn)      (const char* out_path, McpCtx ctx);
typedef void (*McpStartRecordingFn)(const char* out_path, McpCtx ctx);
typedef void (*McpStopRecordingFn) (McpCtx ctx);
typedef void (*McpListScreensFn)   (McpCtx ctx);   /* result via signal */
typedef void (*McpListWindowsFn)   (McpCtx ctx);   /* result via signal */
typedef void (*McpSetScreenFn)     (int screen_index, McpCtx ctx);
typedef void (*McpSetWindowFn)     (uint64_t window_id, McpCtx ctx);

typedef struct {
    McpSnapshotFn       take_snapshot;
    McpStartRecordingFn start_recording;
    McpStopRecordingFn  stop_recording;
    McpListScreensFn    list_screens;
    McpListWindowsFn    list_windows;
    McpSetScreenFn      set_screen;
    McpSetWindowFn      set_window;
    McpCtx              ctx;
} McpCallbacks;

/* Rust exports. */
#ifdef __cplusplus
extern "C" {
#endif
    int  mcp_start(uint16_t port, McpCallbacks callbacks); /* 0 = ok */
    void mcp_stop(void);
    /* Called by Qt to deliver async results back to a pending tool call. */
    void mcp_deliver_snapshot(const char* path);
    void mcp_deliver_screens(const char* json);   /* JSON array of screen names */
    void mcp_deliver_windows(const char* json);   /* JSON array of {id, name} */
    void mcp_deliver_status(const char* json);    /* {recording, active_source} */
#ifdef __cplusplus
}
#endif
```

---

## Rust Implementation Tasks

### 1. Crate scaffolding
- [ ] `Cargo.toml` — `crate-type = ["cdylib"]`
- [ ] Dependencies: `rmcp`, `axum`, `tokio` (rt-multi-thread), `serde_json`,
      `oneshot` channels, `std::panic::catch_unwind` where needed
- [ ] CMake integration: `ExternalProject_Add` or `corrosion-rs` to build the
      crate as part of the main build and copy the dylib into the `.app` bundle

### 2. Runtime lifecycle  (`lib.rs`)
- [ ] `mcp_start()`: wrapped in `catch_unwind`; creates a `tokio::Runtime`
      stored in a `OnceLock<Runtime>`; spawns `transport::serve(port, callbacks)`
- [ ] `mcp_stop()`: signals Tokio shutdown via a `CancellationToken`; waits for
      the runtime to drain before returning so `QLibrary::unload()` is safe
- [ ] All `extern "C"` functions: wrap body in `std::panic::catch_unwind`,
      return an error code rather than unwinding across the FFI boundary

### 3. Transport layer  (`transport.rs`)
- [ ] axum router with two endpoints:
  - `GET  /sse`      — SSE stream, sends MCP events to client
  - `POST /message`  — receives MCP JSON-RPC requests from client
- [ ] Support multiple simultaneous SSE connections (broadcast channel per port)
- [ ] Graceful shutdown on `CancellationToken` cancellation

### 4. MCP server / tool registration  (`server.rs`)
- [ ] Implement `rmcp::ServerHandler` (or equivalent for the SSE transport)
- [ ] `initialize` handshake: return server name `pulse-pro`, version, capabilities
- [ ] `tools/list`: enumerate all registered tools with JSON Schema input schemas
- [ ] `tools/call`: dispatch to the appropriate tool module

### 5. Tool implementations

#### `tools/screenshot.rs` — `take_screenshot`
- Input: `{ "screen_index": int? }` (optional, defaults to active screen)
- Flow:
  1. Store a `oneshot::Sender` keyed to a pending request ID
  2. Call `callbacks.take_snapshot(tmp_path, ctx)` via FFI
  3. Qt writes the file, then calls `mcp_deliver_snapshot(path)`
  4. `mcp_deliver_snapshot` resolves the `oneshot` channel
  5. Read file → base64-encode PNG bytes → return `image` content block
- [ ] Implement happy path
- [ ] Implement timeout (5 s) if Qt never responds

#### `tools/recording.rs` — `start_recording`, `stop_recording`
- `start_recording` input: `{ "path": string? }` (optional output path)
- Flow: fire-and-forget FFI call; return `{ "ok": true }` immediately
- [ ] `start_recording` tool
- [ ] `stop_recording` tool
- [ ] Guard against calling start when already recording (track state locally)

#### `tools/sources.rs` — `list_screens`, `list_windows`, `set_screen`, `set_window`
- `list_screens` / `list_windows`: same async/deliver pattern as screenshot
- `set_screen` input: `{ "index": int }`
- `set_window` input: `{ "window_id": int }`
- [ ] `list_screens` tool
- [ ] `list_windows` tool
- [ ] `set_screen` tool
- [ ] `set_window` tool

#### `tools/status.rs` — `get_status`
- Returns current state: recording on/off, active source type, output path if recording
- [ ] Implement (async deliver from Qt or maintain mirror state in Rust)

### 6. Pending-request bookkeeping  (`server.rs`)
- [ ] `DashMap<RequestId, oneshot::Sender<serde_json::Value>>` for in-flight
      async tool calls
- [ ] `mcp_deliver_*` functions look up and resolve the matching sender
- [ ] Cleanup on timeout or client disconnect

### 7. Error handling
- [ ] All tool errors return MCP `isError: true` content blocks, never panics
- [ ] FFI callbacks that receive null pointers log and return without crashing
- [ ] Port-in-use error from axum surfaces as `mcp_start` return code `-1`

---

## Qt Side (tracked separately, brief reference here)

The C++ work needed to support this dylib:

- `AppController::startMcpPlugin()` — loads dylib, resolves symbols, calls `mcp_start`
- Callback lambdas post work to Qt main thread via `QMetaObject::invokeMethod`
- After each operation, call the matching `mcp_deliver_*` FFI function with the result
- `AppController::~AppController()` — calls `mcp_stop()` before unloading

---

## Build Integration

- Rust crate lives at `mcp/` in the repo root
- CMake uses [corrosion](https://github.com/corrosion-rs/corrosion) to build it:
  ```cmake
  find_package(Corrosion REQUIRED)
  corrosion_import_crate(MANIFEST_PATH mcp/Cargo.toml)
  ```
- The `.dylib` is added to the macOS bundle via `MACOSX_BUNDLE_INFO_PLIST` +
  `install(FILES ...)` into `PulsePro.app/Contents/Frameworks/`
- `QLibrary` resolves it by framework-relative path at runtime

---

## `.vscode/mcp.json`

```json
{
  "servers": {
    "pulse-pro": {
      "type": "sse",
      "url": "http://localhost:7341/sse"
    }
  }
}
```

PulsePro.app must be running before VS Code can connect. No spawning — the
server is embedded in the app process.
