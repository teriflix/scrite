const fs = require('fs');
const path = require('path');
const glob = require('glob');

const MACRO_REGEX = /^\s*(Q_PROPERTY|Q_CLASSINFO)\s*\(/;
const Q_PROPERTY_KEYWORDS = ['READ', 'WRITE', 'NOTIFY', 'MEMBER', 'RESET', 'REVISION', 'DESIGNABLE', 'SCRIPTABLE', 'USER', 'CONSTANT', 'FINAL', 'STORED'];

/**
 * Determines the indentation for class members based on the class declaration.
 * @param {string[]} lines The file lines.
 * @param {number} lineIndex The index of the current line.
 * @returns {string} The indentation string for members.
 */
function getMemberIndent(lines, lineIndex) {
    for (let i = lineIndex - 1; i >= 0; i--) {
        const line = lines[i].trim();
        if (line.startsWith('class ') || line.startsWith('struct ')) {
            const classIndent = lines[i].match(/^\s*/)[0] || '';
            return classIndent + '    '; // Assume 4 spaces for members
        }
    }
    return '    '; // Default to 4 spaces if no class found
}

/**
 * Stage 1: Combine multi-line Q_CLASSINFO and Q_PROPERTY into single lines.
 * @param {string[]} lines The file lines.
 * @returns {string[]} Modified lines.
 */
function stage1CollapseMacros(lines) {
    const newLines = [];
    let i = 0;
    while (i < lines.length) {
        const line = lines[i];
        if (MACRO_REGEX.test(line.trim())) {
            const indent = getMemberIndent(lines, i);
            let singleMacro = '';
            let consumed = 0;
            let tempIndex = i;
            do {
                if (tempIndex >= lines.length) break;
                singleMacro += ' ' + lines[tempIndex].trim();
                consumed++;
                tempIndex++;
            } while (!singleMacro.includes(')'));
            singleMacro = singleMacro.replace(/\s+/g, ' ').trim();
            newLines.push(indent + singleMacro);
            i += consumed;
        } else {
            newLines.push(line);
            i++;
        }
    }
    return newLines;
}

/**
 * Stage 2: Get rid of clang-format off and on statements surrounding them.
 * @param {string[]} lines The file lines.
 * @returns {string[]} Modified lines.
 */
function stage2RemoveClangFormat(lines) {
    return lines.filter(line => !line.trim().startsWith('// clang-format'));
}

/**
 * Stage 3: Format Q_PROPERTY statement such that they show up as multi-line.
 * @param {string[]} lines The file lines.
 * @returns {string[]} Modified lines.
 */
function stage3FormatQProperty(lines) {
    const newLines = [];
    for (const line of lines) {
        if (line.trim().startsWith('Q_PROPERTY')) {
            const indent = line.match(/^\s*/)[0] || '';
            const contentStart = line.indexOf('(') + 1;
            const contentEnd = line.lastIndexOf(')');
            const content = line.substring(contentStart, contentEnd).trim();
            const parts = content.split(/\s+/);
            const typeAndName = parts.slice(0, 2).join(' ');
            const keywords = parts.slice(2);
            let formatted = `${indent}Q_PROPERTY(${typeAndName}`;
            for (let j = 0; j < keywords.length; j += 2) {
                const keyword = keywords[j];
                const value = keywords[j + 1] || '';
                formatted += `\n${indent}           ${keyword} ${value}`;
            }
            formatted += ')';
            newLines.push(formatted);
        } else {
            newLines.push(line);
        }
    }
    return newLines;
}

/**
 * Stage 4: Add clang-format off and on blocks such that a group of Q_CLASSINFO and Q_PROPERTY statements show up in a single block.
 * @param {string[]} lines The file lines.
 * @returns {string[]} Modified lines.
 */
function stage4AddClangFormatBlocks(lines) {
    const newLines = [];
    let i = 0;
    while (i < lines.length) {
        const line = lines[i];
        if (MACRO_REGEX.test(line.trim())) {
            const indent = line.match(/^\s*/)[0] || '';
            newLines.push(`${indent}// clang-format off`);
            while (i < lines.length && MACRO_REGEX.test(lines[i].trim())) {
                newLines.push(lines[i]);
                i++;
            }
            newLines.push(`${indent}// clang-format on`);
        } else {
            newLines.push(line);
            i++;
        }
    }
    return newLines;
}

/**
 * Processes a single file's content through all stages.
 * @param {string} content The original file content.
 * @returns {string} The modified file content.
 */
function processFileContent(content) {
    let lines = content.split(/\r?\n/);
    lines = stage1CollapseMacros(lines);
    lines = stage2RemoveClangFormat(lines);
    lines = stage3FormatQProperty(lines);
    lines = stage4AddClangFormatBlocks(lines);
    return lines.join('\n');
}

/**
 * Main function to find and process all relevant files.
 */
async function main() {
    const searchDir = path.join(__dirname, '..', '..');
    console.log(`Searching for .h, .cpp, and .mm files in: ${searchDir}`);

    const files = glob.sync('**/*.{h,cpp,mm}', {
        cwd: searchDir,
        ignore: ['**/3rdparty/**', '**/build*/**'],
        nodir: true,
    });

    console.log(`Found ${files.length} files to process.`);
    let modifiedCount = 0;

    for (const file of files) {
        const absolutePath = path.join(searchDir, file);
        try {
            const originalContent = fs.readFileSync(absolutePath, 'utf8');
            const newContent = processFileContent(originalContent);

            if (originalContent !== newContent) {
                fs.writeFileSync(absolutePath, newContent, 'utf8');
                console.log(`Modified: ${file}`);
                modifiedCount++;
            }
        } catch (error) {
            console.error(`Error processing file ${absolutePath}:`, error);
        }
    }

    console.log(`\nProcessing complete. Modified ${modifiedCount} files.`);
}

main();