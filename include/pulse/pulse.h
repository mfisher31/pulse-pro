#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// Symbol visibility for .pulse bundle entry points.
// Plugins must mark pulse_entry with PULSE_EXPORT so the host can resolve it
// via dlsym/QLibrary regardless of the compiler's default visibility setting.
#if defined(_WIN32) || defined(_WIN64)
#   if defined(PULSE_PLUGIN_BUILD)
#       define PULSE_EXPORT __declspec(dllexport)
#   else
#       define PULSE_EXPORT __declspec(dllimport)
#   endif
#elif defined(__GNUC__) || defined(__clang__)
#   define PULSE_EXPORT __attribute__((visibility("default")))
#else
#   define PULSE_EXPORT
#endif

// pulse/version.h
typedef struct { uint32_t major, minor, patch; } PulseVersion;
#define PULSE_VERSION ((PulseVersion){1, 0, 0})

// The entry point. init() is called before any other symbol is touched.
// get_factory() returns a factory by ID string, or null if not supported.
// This is the only place factories are discovered — new factory types
// are added here without changing any other struct.
typedef struct {
    PulseVersion pulse_version;  // set to PULSE_VERSION at build time
    bool  (*init)(const char *bundle_path);
    void  (*deinit)(void);
    const void *(*get_factory)(const char *factory_id);  // [thread-safe]
} PulseEntry;

typedef struct {
    PulseVersion version;
} PulseHost;

// The descriptor is pure metadata — no code. The factory returns these.
typedef struct {
    PulseVersion api;
    const char *id;           // reverse-DNS: "io.mie.pulse.mcp-server"
    const char *name;         // "MCP Server"
    const char *vendor;       // "Medical Informatics Engineering"
    const char *version;      // "1.0.0"
    const char *description;
} PulseDescriptor;

// pulse/plugin.h
//


// A plugin instance. The host calls init, then start. Capabilities are
// discovered entirely via get_extension — the plugin declares nothing else.
typedef struct PulseService PulseService;
struct PulseService {
    const PulseDescriptor *descriptor;
    void *handle;   // plugin-private, opaque to host

    bool (*init)   (const PulseService *service, const PulseHost *host);
    void (*destroy)(const PulseService *service);
    bool (*start)  (const PulseService *service);
    void (*stop)   (const PulseService *service);

    // The core extensibility mechanism.
    // Returns a pointer to the plugin's implementation of the named extension,
    // or null if the plugin does not support it. The returned pointer is valid
    // for the lifetime of the plugin instance.
    // [thread-safe]
    const void *(*get_extension)(const PulseService *plugin, const char *id);
};

#ifdef __cplusplus
} // extern "C"
#endif
