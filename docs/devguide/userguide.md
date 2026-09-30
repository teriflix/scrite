# User Guide Documentation

## mkdocs
If you want to generate documentation from the `docs/userguide` folder, you will
also need to install Python and mkdocs.

### On macOS

Most users prefer to install python from homebrew on macOS. In such cases, it
works best if a separate environment is created for serving docs.

    # Switch to docs/userguide
    cd ./docs/userguide

    # Fresh Homebrew Python (ensures pip works)
    brew install python

    # Project-specific virtualenv (isolates from system)
    cd your-scrite-docs-folder
    python3 -m venv venv
    source venv/bin/activate

    # Install MkDocs ecosystem
    pip install mkdocs mkdocs-material mkdocs-video mkdocs-rss-plugin

    # Serve docs
    mkdocs serve --livereload

    # Open http://127.0.0.1:8000 on your browser

### On Windows & Linux

Once you have python and pip installed,

    # Switch to docs/userguide
    cd ./docs/userguide

    # Install Mkdocs ecosystem
    pip install mkdocs mkdocs-material mkdocs-video

    # Serve docs
    python -m mkdocs serve

    # Open http://127.0.0.1:8000 on your browser

Further reading: https://www.mkdocs.org/user-guide/installation/
