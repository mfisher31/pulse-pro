# Pulse Extension System

Inspired directly by CLAP's architecture. The core is deliberately minimal — just
enough to load, identify, and lifecycle a plugin. **Everything the plugin actually
does is an extension**, including what category it belongs to. The host queries
extensions it understands; it ignores ones it doesn't. Plugins declare nothing
upfront except identity.

---

## The One Mandatory Export

```c
// Every .pulse bundle exports exactly one symbol:
PULSE_EXPORT extern const pulse_entry_t pulse_entry;
```

That's it. Same as `clap_entry`. The host `dlopen`s the bundle, resolves this
symbol, and the entire plugin contract flows from there. If the symbol is absent,
the bundle is rejected immediately.

---

## Core ABI  (`pulse/pulse.h`)

Four structs form the immutable core. Nothing here changes after v1.

```c
// pulse/version.h
typedef struct { uint32_t major, minor, patch; } pulse_version_t;
#define PULSE_VERSION ((pulse_version_t){1, 0, 0})

// pulse/entry.h
//
// The entry point. init() is called before any other symbol is touched.
// get_factory() returns a factory by ID string, or null if not supported.
// This is the only place factories are discovered — new factory types
// are added here without changing any other struct.
typedef struct {
    pulse_version_t pulse_version;  // set to PULSE_VERSION at build time
    bool  (*init)(const char *bundle_path);
    void  (*deinit)(void);
    const void *(*get_factory)(const char *factory_id);  // [thread-safe]
} pulse_entry_t;

// pulse/plugin.h
//
// The descriptor is pure metadata — no code. The factory returns these.
typedef struct {
    pulse_version_t pulse_version;
    const char *id;           // reverse-DNS: "io.mie.pulse.mcp-server"
    const char *name;         // "MCP Server"
    const char *vendor;       // "Medical Informatics Engineering"
    const char *version;      // "1.0.0"
    const char *description;
} pulse_plugin_descriptor_t;

// pulse/plugin.h
//
// A plugin instance. The host calls init, then start. Capabilities are
// discovered entirely via get_extension — the plugin declares nothing else.
typedef struct {
    const pulse_plugin_descriptor_t *descriptor;
    void *plugin_data;   // plugin-private, opaque to host

    bool (*init)   (const pulse_plugin_t *plugin, const pulse_host_t *host);
    void (*destroy)(const pulse_plugin_t *plugin);
    bool (*start)  (const pulse_plugin_t *plugin);
    void (*stop)   (const pulse_plugin_t *plugin);

    // The core extensibility mechanism.
    // Returns a pointer to the plugin's implementation of the named extension,
    // or null if the plugin does not support it. The returned pointer is valid
    // for the lifetime of the plugin instance.
    // [thread-safe]
    const void *(*get_extension)(const pulse_plugin_t *plugin, const char *id);
} pulse_plugin_t;

// pulse/host.h
//
// The host side is symmetric. The plugin calls get_extension on the host
// to discover what the host offers (capture API, UI, log, etc.).
typedef struct {
    pulse_version_t pulse_version;
    const char *name;     // "PulsePro"
    const char *version;  // host version
    void *host_data;      // opaque, for internal host use

    // [thread-safe]
    const void *(*get_extension)(const pulse_host_t *host, const char *id);

    // Plugin calls this to request the host call a specific function
    // on the main thread. Used by plugins that need to touch host UI state.
    void (*request_callback)(const pulse_host_t *host);
    void (*on_main_thread)  (const pulse_plugin_t *plugin);  // host calls back here
} pulse_host_t;
```

---

## Factory Pattern

CLAP separates "list and create plugins" from the entry point via a factory ID
string. Pulse does the same, and deliberately uses the same pattern for future
factory types (e.g. a preset factory, a resource factory).

```c
// pulse/factory/plugin-factory.h
#define PULSE_PLUGIN_FACTORY_ID "pulse.plugin-factory/1"

typedef struct {
    uint32_t (*count)(const pulse_plugin_factory_t *factory);
    const pulse_plugin_descriptor_t *(*descriptor)(
        const pulse_plugin_factory_t *factory, uint32_t index);
    const pulse_plugin_t *(*create)(
        const pulse_plugin_factory_t *factory,
        const pulse_host_t *host,
        const char *plugin_id);
} pulse_plugin_factory_t;
```

Host usage:
```c
// 1. Resolve entry point
const pulse_entry_t *e = dlsym(handle, "pulse_entry");
e->init(bundle_path);

// 2. Get plugin factory
const pulse_plugin_factory_t *f = e->get_factory(PULSE_PLUGIN_FACTORY_ID);

// 3. Enumerate descriptors (scanner phase — no full init needed)
for (uint32_t i = 0; i < f->count(f); i++) {
    const pulse_plugin_descriptor_t *d = f->descriptor(f, i);
    // record id, name, version — then exit scanner process
}

// 4. Create instance (main process, after user grants trust)
const pulse_plugin_t *plugin = f->create(f, &host, "io.mie.pulse.mcp-server");
plugin->init(plugin, &host);
plugin->start(plugin);
```

A bundle may contain multiple plugins — the factory lists them all. One `.pulse`
bundle could ship a "MCP Server" and a "REST API" plugin under the same dylib.

---

## Extension IDs

Extension IDs are plain C strings. Versioning is embedded in the ID — a breaking
change gets a new string, not a version number bump on the old one.

```
"pulse.mcp-server/1"
"pulse.capture-filter/1"
"pulse.output-destination/1"
"pulse.ui-panel/1"
"pulse.log/1"                   ← host extension: plugin calls this to log
"pulse.capture-api/1"           ← host extension: snapshot, record, list sources
"pulse.settings/1"              ← host extension: read/write persistent settings
```

Neither side is required to support any extension. The handshake is:
```c
// Plugin queries what host offers:
const pulse_capture_api_t *cap = host->get_extension(host, "pulse.capture-api/1");
if (cap) { /* use it */ }

// Host queries what plugin offers:
const pulse_mcp_server_t *mcp = plugin->get_extension(plugin, "pulse.mcp-server/1");
if (mcp) { mcp->start(plugin, port); }
```

---

## Host Extensions  (`pulse/ext/host/`)

What the host makes available to plugins. These are the host-side API surface —
a third-party plugin calls these to interact with Pulse.

### `pulse.capture-api/1`
```c
typedef struct {
    // Async — result delivered via plugin's pulse.capture-api-receiver/1
    void (*take_snapshot)(const pulse_host_t*, const char *out_path);
    void (*start_recording)(const pulse_host_t*, const char *out_path);
    void (*stop_recording)(const pulse_host_t*);
    void (*list_screens)(const pulse_host_t*);
    void (*list_windows)(const pulse_host_t*);
    void (*set_screen)(const pulse_host_t*, uint32_t index);
    void (*set_window)(const pulse_host_t*, uint64_t window_id);
    bool (*is_recording)(const pulse_host_t*);
} pulse_capture_api_t;
```

### `pulse.log/1`
```c
typedef enum { PulseLogDebug, PulseLogInfo, PulseLogWarn, PulseLogError } pulse_log_level_t;
typedef struct {
    void (*log)(const pulse_host_t*, pulse_log_level_t, const char *msg);
} pulse_log_t;
```

### `pulse.settings/1`
```c
typedef struct {
    const char *(*get)(const pulse_host_t*, const char *key);          // returns null if absent
    void        (*set)(const pulse_host_t*, const char *key, const char *value);
} pulse_settings_t;
```

---

## Plugin Extensions  (`pulse/ext/plugin/`)

What plugins may offer back to the host.

### `pulse.mcp-server/1`
```c
typedef struct {
    bool (*start)(const pulse_plugin_t*, uint16_t port);
    void (*stop) (const pulse_plugin_t*);
    uint16_t (*port)(const pulse_plugin_t*);   // actual bound port
} pulse_mcp_server_t;
```

### `pulse.capture-filter/1`
```c
// Host calls process_frame on each captured frame before display + encode.
// frame_data is a platform pixel buffer handle (CVPixelBufferRef on macOS).
typedef struct {
    void (*process_frame)(const pulse_plugin_t*, void *frame_data,
                          uint32_t width, uint32_t height);
} pulse_capture_filter_t;
```

### `pulse.output-destination/1`
```c
// Plugin receives encoded packets and writes to its own sink
// (RTMP, HLS, custom file format, etc.)
typedef struct {
    bool (*open) (const pulse_plugin_t*, const char *uri);
    void (*write)(const pulse_plugin_t*, const uint8_t *data, uint32_t len);
    void (*close)(const pulse_plugin_t*);
} pulse_output_destination_t;
```

### `pulse.capture-api-receiver/1`
Plugin implements this so the host can deliver async results:
```c
typedef struct {
    void (*on_snapshot_ready)(const pulse_plugin_t*, const char *path);
    void (*on_screens_ready) (const pulse_plugin_t*, const char *json);
    void (*on_windows_ready) (const pulse_plugin_t*, const char *json);
    void (*on_error)         (const pulse_plugin_t*, const char *message);
} pulse_capture_api_receiver_t;
```

---

## Bundle Layout

```
MyPlugin.pulse/
└── Contents/
    ├── Info.plist          ← macOS bundle identity (CFBundleIdentifier, version)
    └── MacOS/
        └── MyPlugin        ← the dylib (no extension, macOS convention)
```

The `manifest.json` from the earlier rough sketch is **dropped** — all metadata
comes from the `pulse_plugin_descriptor_t` returned by the factory. There is no
separate metadata file to keep in sync.

### Discovery paths (macOS)
```
~/Library/Application Support/PulsePro/Plugins/    ← user plugins
/Library/Application Support/PulsePro/Plugins/     ← system plugins
PulsePro.app/Contents/PlugIns/                     ← bundled first-party plugins
$PULSE_PLUGIN_PATH                                  ← colon-separated, like CLAP_PATH
```

---

## Scanner Process

The host never `dlopen`s a third-party bundle in the main process until the user
has granted trust. The scanner is a minimal helper:

```
PulsePro.app/Contents/MacOS/PulsePluginScanner
```

Protocol over stdin/stdout (newline-delimited JSON):
```json
// stdin  → scanner
{ "action": "scan", "path": "/path/to/MyPlugin.pulse" }

// stdout ← scanner (success)
{ "ok": true, "plugins": [
    { "id": "io.example.myplugin", "name": "My Plugin", "version": "1.0.0",
      "extensions": ["pulse.mcp-server/1"] }
]}

// stdout ← scanner (failure)
{ "ok": false, "error": "signature validation failed" }
```

The scanner:
1. Validates the bundle's code signature (macOS `SecStaticCode`)
2. Calls `init()` + factory scan only — no `create()`, no `start()`
3. Reports the discovered `pulse_plugin_descriptor_t` values + which extensions
   are available (queried via a stub `get_extension` on a null instance handle)
4. Calls `deinit()` and exits

If the scanner crashes, the main process records the plugin as incompatible. The
main process never crashes.

---

## Versioning Contract

| Rule | Rationale |
|---|---|
| Struct fields are append-only after v1 | Existing plugins never recompile |
| New extension ID = new string | Old hosts ignore unknown IDs; no negotiation needed |
| `pulse_version_t` in entry checked at load time | Reject plugins built against a future incompatible ABI |
| Extension structs have no version field | The ID string carries the version |

---

## SDK Structure

What third-party developers receive:

```
PulseSDK-1.0/
├── include/
│   └── pulse/
│       ├── version.h
│       ├── entry.h
│       ├── plugin.h
│       ├── host.h
│       ├── factory/
│       │   └── plugin-factory.h
│       └── ext/
│           ├── host/
│           │   ├── capture-api.h
│           │   ├── log.h
│           │   └── settings.h
│           └── plugin/
│               ├── mcp-server.h
│               ├── capture-filter.h
│               ├── output-destination.h
│               └── capture-api-receiver.h
├── examples/
│   ├── c-plugin/        ← minimal C plugin implementing pulse.mcp-server/1
│   └── rust-plugin/     ← Cargo cdylib template with safe Rust wrappers
└── CHANGELOG.md         ← I change log — critical for third parties
```

---

## Relation to the MCP Server Plan

The MCP server (`mcp-server-rust.md`) becomes the **first first-party `.pulse` plugin**,
built against the same SDK headers a third party would use. It implements:

- `pulse_entry_t pulse_entry` (the mandatory export)
- `pulse_plugin_factory_t` via `get_factory(PULSE_PLUGIN_FACTORY_ID)`
- One plugin instance exposing `pulse.mcp-server/1`
- Calls host's `pulse.capture-api/1` for all Pulse operations
- Implements `pulse.capture-api-receiver/1` to receive async results

The C ABI callbacks described in `mcp-server-rust.md` are replaced entirely by
the `pulse.capture-api/1` host extension. The MCP plan should be updated once the
core host API headers are finalized.
AB