# QuaZip wrapper setup

This directory wraps the vendored `quazip` submodule for Scrite's build.

## Windows setup

Windows builds use a wrapper-local vcpkg bootstrap flow to provide `zlib` and `bzip2` to QuaZip.

1. From the repository root, run:
   - `.\thirdparty\static\quazip\bootstrap-vcpkg.ps1`
2. Re-configure the root CMake project in Qt Creator.

The script prepares:
- local vcpkg checkout in `thirdparty/static/quazip/.vcpkg`
- manifest dependencies in `thirdparty/static/quazip/vcpkg_installed/<triplet>`

## macOS and other non-Windows setup

Do not run `bootstrap-vcpkg.ps1`.

Non-Windows builds use the default QuaZip dependency discovery path (the same style that worked around commit `851b492`):
- `find_package(ZLIB REQUIRED)`
- optional `find_package(BZip2)`

Provide dependencies through your platform/toolchain as usual (for example, your package manager, SDK, or toolchain environment used by Qt Creator).

## Notes

- Do not edit files inside `thirdparty/static/quazip/quazip`; that directory is the upstream submodule.
- Project-specific integration logic should stay in this wrapper directory.
