const fs = require('fs');
const path = require('path');
const glob = require('glob');

const MACRO_REGEX = /^\s*(Q_PROPERTY|Q_CLASSINFO)\s*\(/;

/**
 * Processes a single file's content to find and format macro blocks.
 * @param {string} content The original file content.
 * @returns {string} The modified file content.
 */
function processFileContent(content) {
    const lines = content.split(/\r?\n/);
    const newLines = [];
    let i = 0;
    let fileModified = false;

    while (i < lines.length) {
        const line = lines[i];

        // Check if this line starts a macro block and is not already inside a clang-format off block.
        if (MACRO_REGEX.test(line) && !lines.slice(0, i).join('\n').includes('// clang-format off')) {
            const blockStartIndex = i;
            const macroBlock = [];
            
            // Collect all consecutive macro lines, handling multi-line macros.
            while (i < lines.length && MACRO_REGEX.test(lines[i])) {
                let currentMacro = lines[i].trim();
                i++;
                // Keep appending lines until the macro definition is complete (ends with ')').
                while (!currentMacro.endsWith(')')) {
                    if (i >= lines.length) break; // End of file reached unexpectedly.
                    currentMacro += ' ' + lines[i].trim();
                    i++;
                }
                // Clean up extra spaces and add to our block.
                macroBlock.push(currentMacro.replace(/\s+/g, ' '));
            }

            if (macroBlock.length > 0) {
                fileModified = true;
                // Add clang-format comments around the processed block.
                const indent = lines[blockStartIndex].match(/^\s*/)[0] || '';
                newLines.push(indent + '// clang-format off');
                macroBlock.forEach(macro => newLines.push(indent + macro));
                newLines.push(indent + '// clang-format on');
            }
        } else {
            // This line is not part of a macro block we need to process.
            newLines.push(line);
            i++;
        }
    }

    // Only return new content if we actually made a change.
    return fileModified ? newLines.join('\n') : content;
}

/**
 * Main function to find and process all relevant files.
 */
async function main() {
    // Define the search directory as ../../ relative to this script's location.
    const searchDir = path.join(__dirname, '..', '..');
    console.log(`Searching for .h, .cpp, and .mm files in: ${searchDir}`);

    const files = glob.sync('**/*.{h,cpp,mm}', {
        cwd: searchDir, // Set the current working directory for the search.
        ignore: '**/3rdparty/**',
        nodir: true,
    });

    console.log(`Found ${files.length} files to process.`);
    let modifiedCount = 0;

    for (const file of files) {
        // Construct the absolute path to read/write the file.
        const absolutePath = path.join(searchDir, file);
        try {
            const originalContent = fs.readFileSync(absolutePath, 'utf8');
            const newContent = processFileContent(originalContent);

            if (originalContent !== newContent) {
                fs.writeFileSync(absolutePath, newContent, 'utf8');
                console.log(`Modified: ${file}`); // Log the relative path for clarity.
                modifiedCount++;
            }
        } catch (error) {
            console.error(`Error processing file ${absolutePath}:`, error);
        }
    }

    console.log(`\nProcessing complete. Modified ${modifiedCount} files.`);
}

main();