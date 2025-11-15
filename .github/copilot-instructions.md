# Scrite Codebase Guide for AI Assistants

This document provides essential guidance for AI coding assistants working on the Scrite codebase.

## Big Picture: C++/QML Architecture

Scrite is a desktop screenwriting application built with the Qt framework. It follows a common architectural pattern for Qt applications:

-   **C++ Backend**: The core application logic, data models, and business rules are implemented in C++. The entry point is `main.cpp`.
-   **QML Frontend**: The user interface is built with QML. The main UI file is `main.qml`, which defines the window structure and loads other UI components from the `qml/` directory.

### Key Components & Data Flow

-   **`ScriteDocument` (Singleton)**: This is the most critical class in the C++ backend. It's a singleton (`ScriteDocument::instance()`) that holds the entire data model for an open screenplay document. This includes the screenplay content, structure, notes, and more. Any logic that needs to read or modify the screenplay data will interact with this object. It's located in `src/document/scritedocument.h`.

-   **C++ to QML Integration**: The C++ backend exposes data and functionality to the QML frontend primarily through:
    -   **Context Properties**: In `main.cpp`, you'll see singletons and other objects exposed to the QML engine using `scriteWindow.setContextProperty()`. This makes C++ objects available as global variables in QML.
    -   **`Q_PROPERTY`**: C++ classes registered with the QML type system use `Q_PROPERTY` to expose their properties to QML. These can be read directly and often have `NOTIFY` signals for binding.
    -   **`Q_INVOKABLE`**: C++ methods marked as `Q_INVOKABLE` can be called directly from QML, allowing the UI to trigger backend logic.

-   **UI Structure (`qml/`)**: The UI is modular. Major parts of the application have their own view files (e.g., `ScreenplayEditor.qml`, `StructureView.qml`, `TimelineView.qml`). These are assembled in `ScriteMainWindow.qml`.

## Developer Workflow

### Building the Project

The standard way to build Scrite is to use Qt Creator:
1.  Install Qt 5.15.x.
2.  Open the `scrite.pro` file in Qt Creator.
3.  Configure the project for your platform (Desktop).
4.  Build and run the project.

The `.pro` file defines all the source files, dependencies, and build steps. There are platform-specific configurations for Windows, macOS, and Linux within `scrite.pro`.

### Testing

There is no dedicated test suite immediately apparent in the project structure. Validation is likely done through manual testing. When adding new features, ensure they are manually verifiable.

## Project-Specific Conventions

### C++ Conventions

-   **Singletons**: The project makes heavy use of singleton classes for managing global state (e.g., `ScriteDocument`, `LanguageEngine`, `UndoHub`). Access them via the static `instance()` method.
-   **QObject Models**: Data collections exposed to QML are often subclasses of `QAbstractListModel` to enable list-based views in QML. See `src/core/qobjectlistmodel.h` for a generic implementation used in the project.
-   **File System Abstraction**: The `DocumentFileSystem` class provides an abstraction for reading/writing to the `.scrite` file format, which is a zipped archive.

### QML Conventions

-   **Component-Based UI**: The `qml/` directory is organized by feature or component. Reusable controls are in `qml/controls/`.
-   **Global Objects**: Many C++ singletons are available globally in QML. For example, you can access the main document via the `scriteDocument` global object in QML.

### File Types

-   **`.scrite`**: The native file format is a zip archive containing the screenplay in a structured format (likely JSON or XML), along with metadata and attachments.
-   **`.pro`**: Qt project file.
-   **`.qml`**: UI definition files.
-   **`.cpp`/`.h`**: C++ source and header files.

## Key Files & Directories

-   `scrite.pro`: The main project file. Defines the entire build.
-   `main.cpp`: Application entry point. Good place to understand C++ object initialization.
-   `main.qml`: QML entry point. Good place to understand the high-level UI structure.
-   `src/document/scritedocument.h`: The central data model for the application.
-   `qml/ScriteMainWindow.qml`: The main window QML component, which assembles the different views.
-   `src/importers/` & `src/exporters/`: Logic for importing from and exporting to other formats (Final Draft, PDF, etc.).
-   `packaging/`: Scripts and templates for creating distributable application packages.
