/**
 * Cross-platform tool to generate languageengine_p.h from Qt's QChar::Script enum.
 * Builds and runs on Windows, macOS, and Linux with standard C.
 *
 * Usage: qtchar_gen <qt_include_path> <output_file>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#define PATH_SEP "\\"
#else
#include <dirent.h>
#define PATH_SEP "/"
#endif

#define MAX_FILE_SIZE (1024 * 1024)
#define MAX_SCRIPTS 500

typedef struct {
    char name[64];
} Script;

int file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "ERROR: Cannot open file: %s\n", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0 || size > MAX_FILE_SIZE) {
        fprintf(stderr, "ERROR: File size invalid: %s\n", path);
        fclose(f);
        return NULL;
    }

    char *content = (char *)malloc(size + 1);
    if (!content) {
        fprintf(stderr, "ERROR: Out of memory\n");
        fclose(f);
        return NULL;
    }

    size_t read = fread(content, 1, size, f);
    fclose(f);

    if (read != (size_t)size) {
        fprintf(stderr, "ERROR: Failed to read file: %s\n", path);
        free(content);
        return NULL;
    }

    content[size] = '\0';
    return content;
}

const char *find_qchar_h(const char *qt_include_path) {
    static char path_buffer[512];

    /* Try direct qchar.h */
    snprintf(path_buffer, sizeof(path_buffer), "%s%sqchar.h", qt_include_path, PATH_SEP);
    if (file_exists(path_buffer)) {
        return path_buffer;
    }

    /* Try QtCore/qchar.h */
    snprintf(path_buffer, sizeof(path_buffer), "%s%sQtCore%sqchar.h", qt_include_path, PATH_SEP, PATH_SEP);
    if (file_exists(path_buffer)) {
        return path_buffer;
    }

    fprintf(stderr, "ERROR: Could not find qchar.h in %s\n", qt_include_path);
    return NULL;
}

int extract_scripts(const char *content, Script *scripts, int max_scripts) {
    /* Find "enum Script" specifically */
    const char *enum_pos = strstr(content, "enum");
    int found_script_enum = 0;

    while (enum_pos && !found_script_enum) {
        const char *check_pos = enum_pos + 4;  /* Skip "enum" */
        /* Skip whitespace and optional "class" */
        while (*check_pos && (*check_pos == ' ' || *check_pos == '\t' || *check_pos == '\n')) {
            check_pos++;
        }
        if (strncmp(check_pos, "class", 5) == 0) {
            check_pos += 5;
            while (*check_pos && (*check_pos == ' ' || *check_pos == '\t' || *check_pos == '\n')) {
                check_pos++;
            }
        }
        /* Now we should be at "Script" */
        if (strncmp(check_pos, "Script", 6) == 0) {
            enum_pos = check_pos;
            found_script_enum = 1;
            break;
        }
        enum_pos = strstr(enum_pos + 1, "enum");
    }

    if (!found_script_enum) {
        fprintf(stderr, "ERROR: Could not find 'enum Script'\n");
        return 0;
    }

    /* Find opening brace */
    const char *brace_start = strchr(enum_pos, '{');
    if (!brace_start) {
        fprintf(stderr, "ERROR: Could not find enum opening brace\n");
        return 0;
    }

    /* Find closing brace */
    const char *brace_end = strchr(brace_start, '}');
    if (!brace_end) {
        fprintf(stderr, "ERROR: Could not find enum closing brace\n");
        return 0;
    }

    fprintf(stderr, "DEBUG: Enum found, parsing %ld bytes\n", brace_end - brace_start);

    int count = 0;
    const char *pos = brace_start + 1;

    while (pos < brace_end && count < max_scripts) {
        /* Look for Script_ prefix */
        const char *script_pos = strstr(pos, "Script_");
        if (!script_pos || script_pos >= brace_end) {
            break;
        }

        /* Verify we're at a word boundary (not part of a longer identifier) */
        if (script_pos > brace_start + 1) {
            char prev_char = *(script_pos - 1);
            if ((prev_char >= 'a' && prev_char <= 'z') ||
                (prev_char >= 'A' && prev_char <= 'Z') ||
                (prev_char >= '0' && prev_char <= '9') ||
                prev_char == '_') {
                pos = script_pos + 1;
                continue;
            }
        }

        /* Extract the identifier - read while alphanumeric or underscore */
        int name_len = 0;
        const char *end_pos = script_pos;
        while (end_pos < brace_end && name_len < 63) {
            char c = *end_pos;
            if ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '_') {
                end_pos++;
                name_len++;
            } else {
                break;
            }
        }

        if (name_len > 0 && name_len < 63) {
            strncpy(scripts[count].name, script_pos, name_len);
            scripts[count].name[name_len] = '\0';
            fprintf(stderr, "DEBUG: Found script[%d] = %s\n", count, scripts[count].name);
            count++;
        }

        pos = end_pos;
    }

    fprintf(stderr, "DEBUG: Extracted %d scripts total\n", count);
    return count;
}

typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} Buffer;

Buffer *buffer_create(void) {
    Buffer *buf = (Buffer *)malloc(sizeof(Buffer));
    if (!buf) return NULL;
    buf->capacity = 4096;
    buf->data = (char *)malloc(buf->capacity);
    if (!buf->data) {
        free(buf);
        return NULL;
    }
    buf->size = 0;
    return buf;
}

void buffer_append(Buffer *buf, const char *str) {
    size_t len = strlen(str);
    while (buf->size + len >= buf->capacity) {
        buf->capacity *= 2;
        char *new_data = (char *)realloc(buf->data, buf->capacity);
        if (!new_data) return;
        buf->data = new_data;
    }
    strcpy(buf->data + buf->size, str);
    buf->size += len;
}

void buffer_printf(Buffer *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    size_t needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    while (buf->size + needed >= buf->capacity) {
        buf->capacity *= 2;
        char *new_data = (char *)realloc(buf->data, buf->capacity);
        if (!new_data) return;
        buf->data = new_data;
    }

    va_start(args, fmt);
    vsnprintf(buf->data + buf->size, buf->capacity - buf->size, fmt, args);
    va_end(args);
    buf->size += needed;
}

void buffer_free(Buffer *buf) {
    if (buf) {
        free(buf->data);
        free(buf);
    }
}

int write_header_if_changed(const char *output_file, const Script *scripts, int script_count) {
    /* Generate content in memory */
    Buffer *buf = buffer_create();
    if (!buf) {
        fprintf(stderr, "ERROR: Cannot allocate memory for buffer\n");
        return 0;
    }

    buffer_append(buf, "/****************************************************************************\n");
    buffer_append(buf, "**\n");
    buffer_append(buf, "** This file is auto-generated by tools/qtchar_gen\n");
    buffer_append(buf, "** DO NOT EDIT MANUALLY\n");
    buffer_append(buf, "**\n");
    buffer_append(buf, "** This entire QtChar class is regenerated at build time to stay in sync\n");
    buffer_append(buf, "** with QChar::Script from the Qt version being used.\n");
    buffer_append(buf, "**\n");
    buffer_append(buf, "****************************************************************************/\n");
    buffer_append(buf, "\n");
    buffer_append(buf, "#ifndef LANGUAGEENGINE_P_H\n");
    buffer_append(buf, "#define LANGUAGEENGINE_P_H\n");
    buffer_append(buf, "\n");
    buffer_append(buf, "#include <QObject>\n");
    buffer_append(buf, "\n");
    buffer_append(buf, "class QtChar : public QObject\n");
    buffer_append(buf, "{\n");
    buffer_append(buf, "    Q_OBJECT\n");
    buffer_append(buf, "\n");
    buffer_append(buf, "private:\n");
    buffer_append(buf, "    QtChar() = delete;\n");
    buffer_append(buf, "    QtChar(QObject *parent) = delete;\n");
    buffer_append(buf, "    QtChar(const QtChar &other) = delete;\n");
    buffer_append(buf, "    QtChar &operator=(const QtChar &other) = delete;\n");
    buffer_append(buf, "\n");
    buffer_append(buf, "public:\n");
    buffer_append(buf, "    // Auto-generated from QChar::Script enum\n");
    buffer_append(buf, "    // Regenerated at build time by tools/qtchar_gen to stay in sync\n");
    buffer_append(buf, "    // with the actual Qt version being used.\n");
    buffer_append(buf, "    enum Script {\n");

    /* Append all scripts except ScriptCount with sequential values */
    int value = 0;
    for (int i = 0; i < script_count; i++) {
        if (strcmp(scripts[i].name, "ScriptCount") != 0) {
            buffer_printf(buf, "        %s = %d,\n", scripts[i].name, value);
            value++;
        }
    }

    /* Always append ScriptCount last without comma */
    buffer_printf(buf, "        ScriptCount = %d\n", value);

    buffer_append(buf, "    };\n");
    buffer_append(buf, "    Q_ENUM(Script)\n");
    buffer_append(buf, "};\n");
    buffer_append(buf, "\n");
    buffer_append(buf, "#endif // LANGUAGEENGINE_P_H\n");

    /* Check if file exists and has same content */
    int should_write = 1;
    FILE *existing = fopen(output_file, "r");
    if (existing) {
        fseek(existing, 0, SEEK_END);
        long existing_size = ftell(existing);
        if (existing_size == (long)buf->size) {
            fseek(existing, 0, SEEK_SET);
            char *existing_data = (char *)malloc(existing_size + 1);
            if (existing_data) {
                size_t read = fread(existing_data, 1, existing_size, existing);
                if (read == (size_t)existing_size && memcmp(existing_data, buf->data, existing_size) == 0) {
                    should_write = 0;
                    fprintf(stderr, "Header unchanged, skipping write\n");
                }
                free(existing_data);
            }
        }
        fclose(existing);
    }

    /* Only write if content changed or file doesn't exist */
    if (should_write) {
        FILE *f = fopen(output_file, "w");
        if (!f) {
            fprintf(stderr, "ERROR: Cannot open output file: %s\n", output_file);
            buffer_free(buf);
            return 0;
        }

        size_t written = fwrite(buf->data, 1, buf->size, f);
        fclose(f);

        if (written != buf->size) {
            fprintf(stderr, "ERROR: Failed to write complete header\n");
            buffer_free(buf);
            return 0;
        }
        fprintf(stderr, "Header updated\n");
    }

    buffer_free(buf);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: qtchar_gen <qt_include_path> <output_file>\n");
        return 1;
    }

    const char *qt_include_path = argv[1];
    const char *output_file = argv[2];

    printf("QtChar Generator: qt_include_path='%s'\n", qt_include_path);
    printf("QtChar Generator: output_file='%s'\n", output_file);

    /* Find qchar.h */
    const char *qchar_h = find_qchar_h(qt_include_path);
    if (!qchar_h) {
        return 1;
    }
    printf("Found qchar.h at: %s\n", qchar_h);

    /* Read qchar.h */
    char *content = read_file(qchar_h);
    if (!content) {
        return 1;
    }

    /* Extract Script enum values */
    Script scripts[MAX_SCRIPTS];
    int script_count = extract_scripts(content, scripts, MAX_SCRIPTS);
    free(content);

    if (script_count <= 0) {
        fprintf(stderr, "ERROR: Could not extract any Script values\n");
        return 1;
    }
    printf("Extracted %d script values from QChar::Script\n", script_count);

    /* Generate header only if changed */
    if (!write_header_if_changed(output_file, scripts, script_count)) {
        return 1;
    }
    printf("Generated: %s\n", output_file);

    return 0;
}
