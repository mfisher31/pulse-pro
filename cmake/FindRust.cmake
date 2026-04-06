# Copyright (C) 2026 Medical Informatics Engineering.
# SPDX-License-Identifier: LGPL-3.0-or-later

# Discovers the Rust toolchain (cargo + rustc) from the standard install locations
# so developers don't need to set any environment variables manually.
#
# Search order (first match wins):
#   1. RUST_CARGO / RUST_COMPILER cache variables (explicit override)
#   2. $ENV{CARGO_HOME}/bin          — custom CARGO_HOME
#   3. $ENV{HOME}/.cargo/bin         — default rustup install
#   4. /opt/homebrew/bin             — Homebrew on Apple Silicon
#   5. /usr/local/bin                — Homebrew on Intel / manual installs
#   6. System PATH via find_program  — any other location
#
# After inclusion, the following variables are set in the parent scope:
#   Rust_FOUND         — TRUE if both cargo and rustc were found
#   Rust_CARGO         — absolute path to the cargo executable
#   Rust_COMPILER      — absolute path to the rustc executable
#   Rust_VERSION       — rustc release version string (e.g. "1.94.0")
#   Rust_HOST_TRIPLE   — host target triple (e.g. "aarch64-apple-darwin")
#   Rust_LLVM_VERSION  — LLVM version bundled with rustc
#
# Enforces a minimum version via the standard find_package VERSION mechanism.
# Set Rust_FIND_VERSION in CMakeLists.txt before calling find_package(Rust).

cmake_minimum_required(VERSION 3.21)

# ── 1. Collect candidate search paths ─────────────────────────────────────────

set(_rust_search_paths "")

if(DEFINED ENV{CARGO_HOME})
    list(APPEND _rust_search_paths "$ENV{CARGO_HOME}/bin")
endif()

list(APPEND _rust_search_paths
    "$ENV{HOME}/.cargo/bin"       # rustup default
    "/opt/homebrew/bin"           # Homebrew arm64
    "/usr/local/bin"              # Homebrew x86_64 / manual
)

# ── 2. Locate cargo ───────────────────────────────────────────────────────────

find_program(Rust_CARGO
    NAMES cargo
    HINTS ${_rust_search_paths}
    DOC   "Path to the Cargo package manager"
)

# ── 3. Locate rustc (prefer the same directory as cargo) ─────────────────────

if(Rust_CARGO)
    get_filename_component(_cargo_dir "${Rust_CARGO}" DIRECTORY)
    list(INSERT _rust_search_paths 0 "${_cargo_dir}")
endif()

find_program(Rust_COMPILER
    NAMES rustc
    HINTS ${_rust_search_paths}
    DOC   "Path to the Rust compiler"
)

# ── 4. Extract version and host triple from rustc -vV ────────────────────────

if(Rust_COMPILER)
    execute_process(
        COMMAND "${Rust_COMPILER}" -vV
        OUTPUT_VARIABLE _rustc_verbose
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # release: 1.94.0
    if(_rustc_verbose MATCHES "release: ([0-9]+\\.[0-9]+\\.[0-9]+)")
        set(Rust_VERSION "${CMAKE_MATCH_1}")
    endif()

    # host: aarch64-apple-darwin
    if(_rustc_verbose MATCHES "host: ([a-zA-Z0-9_\\-]+)")
        set(Rust_HOST_TRIPLE "${CMAKE_MATCH_1}")
    endif()

    # LLVM version: 21.1.8
    if(_rustc_verbose MATCHES "LLVM version: ([0-9]+\\.[0-9]+\\.[0-9]+)")
        set(Rust_LLVM_VERSION "${CMAKE_MATCH_1}")
    endif()
endif()

# ── 5. Standard find_package version check ────────────────────────────────────

if(Rust_FIND_VERSION AND Rust_VERSION)
    if(Rust_VERSION VERSION_LESS Rust_FIND_VERSION)
        message(FATAL_ERROR
            "FindRust: required version ${Rust_FIND_VERSION} but found ${Rust_VERSION}. "
            "Update your Rust toolchain with: rustup update  (or: brew upgrade rust)")
    endif()
endif()

# ── 6. Standard find_package result handling ──────────────────────────────────

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Rust
    REQUIRED_VARS Rust_CARGO Rust_COMPILER
    VERSION_VAR   Rust_VERSION
)

if(Rust_FOUND)
    message(STATUS "pulse-pro: found Rust ${Rust_VERSION} (${Rust_HOST_TRIPLE})")
    message(STATUS "  cargo:  ${Rust_CARGO}")
    message(STATUS "  rustc:  ${Rust_COMPILER}")
    if(Rust_LLVM_VERSION)
        message(STATUS "  LLVM:   ${Rust_LLVM_VERSION}")
    endif()
endif()

mark_as_advanced(Rust_CARGO Rust_COMPILER)

