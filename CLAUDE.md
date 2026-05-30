# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Scrite is an open-source desktop screenwriting application built with Qt/QML. It supports Windows, macOS, and Linux. The codebase targets Qt 6.11+ (the Qt 5 → Qt 6 port is complete and merged to `master`).

## Build System

The project uses CMake 3.27+ with Qt 6.11+. Open the project in Qt Creator and build, or use the command line:

```bash
cmake -B build -S .
cmake --build build
```

Build artifacts go to `binary/` (not `build/`). The main app lives under `apps/desktop/`.

There are no automated tests — the project relies on manual testing.

## Code Formatting

Uses clang-format with Qt's coding style (WebKit base). Key rules:
- Column limit: 100 characters
- Pointer binds to type: `int *ptr` (not `int* ptr`)
- No namespace indentation
- Braces wrap after class/function/struct declarations, not after control statements

Run formatter: `clang-format -i <file>`

## Architecture

### C++ Backend (`apps/desktop/src/`)

| Module | Purpose |
|--------|---------|
| `core/` | App lifecycle, language engine, user auth, auto-update, crash reporting |
| `document/` | All screenplay data structures, serialization, undo/redo (`UndoHub`), notes, attachments |
| `quick/` | QML bindings, Qt Quick custom items, UI-facing wrappers for document objects |
| `importers/` | Final Draft (.fdx), Fountain, HTML importers |
| `exporters/` | PDF, Final Draft, Fountain, HTML, ODT, text exporters |
| `printing/` | Print support with custom screenplay formatting |
| `reports/` | Character/location reports, statistics |
| `network/` | REST API integration |
| `utils/` | Platform-specific transliteration (separate `.cpp` files per platform) |
| `crashpad/` | Google Crashpad integration and crash recovery dialog |

**Key singleton classes** (all accessed via `::instance()`):
- `ScriteDocument` — the root document model; owns `Screenplay`, scenes, characters
- `User` — authentication and account state
- `LanguageEngine` — multilingual input (Indian and international languages)
- `UndoHub` — application-wide undo/redo stack
- `ScriteDocumentVault` — recent documents management

**Main entry point:** `apps/desktop/main.cpp` → loads `apps/desktop/main.qml`

### QML Frontend (`apps/desktop/qml/`)

The UI is almost entirely QML (Qt Quick). Main structure:

- `main.qml` — application window root
- `ScriteMainWindowContent.qml` — central workspace
- Core editor views: `ScreenplayEditor.qml`, `StructureView.qml`, `NotebookView.qml`, `TimelineView.qml`, `ScritedView.qml`
- `globals/` — QML singletons and app-wide state
- `controls/` — reusable UI components
- `dialogs/` — modal dialogs
- `commandcenter/` — toolbar/command palette
- `floatingdockpanels/` — dockable side panels
- `helpers/` — utility QML components

The `Scrite` QML singleton (defined in `core/`) is the primary bridge between QML and the C++ backend — it exposes `ScriteDocument`, `User`, app settings, and other globals to QML.

### Third-Party Dependencies (`thirdparty/`)

- **KDE Sonnet** (submodule) — spell checking
- **QuaZip** (submodule) — ZIP archive support for `.scrite` files (which are ZIPs)
- **poly2tri** — polygon triangulation (structure canvas)
- **SimpleCrypt** — lightweight encryption
- **OpenSSL** — TLS for network features

## Platform-Specific Notes

- `utils/platformtransliterator_*.cpp` — separate transliteration implementations for Windows, macOS, Linux
- Windows uses Hunspell for spell check; macOS/Windows use native OS spell checking via KDE Sonnet backends
- macOS builds as universal binaries (x86_64 + arm64)
- Packaging scripts: NSIS (Windows), DMG (macOS), AppImage (Linux)

## QML Style Notes

Always use Qt 6 QML APIs — avoid any Qt 5-era deprecated APIs. `SystemPalette` is used for theming colors throughout the UI.

## Documentation

User guide lives in `docs/userguide/` (MkDocs). To serve locally:

```bash
cd docs/userguide
pip install mkdocs mkdocs-material mkdocs-video
mkdocs serve
# Open http://127.0.0.1:8000
```
