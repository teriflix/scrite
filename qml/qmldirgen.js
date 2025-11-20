const fs = require('fs');
const path = require('path');

/**
 * Recursively finds all subdirectories under the given root directory.
 * @param {string} dir - The root directory to start from.
 * @returns {string[]} - Array of subdirectory paths.
 */
function findSubdirectories(dir) {
    const subdirs = [];
    const items = fs.readdirSync(dir);
    for (const item of items) {
        const fullPath = path.join(dir, item);
        const stat = fs.statSync(fullPath);
        if (stat.isDirectory()) {
            subdirs.push(fullPath);
            subdirs.push(...findSubdirectories(fullPath));
        }
    }
    return subdirs;
}

/**
 * Checks if a QML file contains 'pragma Singleton'.
 * @param {string} filePath - Path to the QML file.
 * @returns {boolean} - True if 'pragma Singleton' is found.
 */
function isSingleton(filePath) {
    try {
        const content = fs.readFileSync(filePath, 'utf8');
        const lines = content.split('\n');
        for (const line of lines) {
            if (line.trim() === 'pragma Singleton') {
                return true;
            }
        }
    } catch (error) {
        console.error(`Error reading file ${filePath}: ${error.message}`);
    }
    return false;
}

/**
 * Generates the qmldir content for a directory.
 * @param {string} dir - The directory path.
 * @param {string[]} qmlFiles - Array of QML file names in the directory.
 * @returns {string} - The content for the qmldir file.
 */
function generateQmldirContent(dir, qmlFiles) {
    const lines = [];
    for (const file of qmlFiles) {
        const filePath = path.join(dir, file);
        const fileName = path.parse(file).name; // Get filename without extension
        const isSingletonFile = isSingleton(filePath);
        const prefix = isSingletonFile ? 'singleton ' : '';
        lines.push(`${prefix}${fileName} 1.0 ${file}`);
    }
    return lines.join('\n');
}

/**
 * Main function to generate qmldir files in all subdirectories under 'qml/'.
 */
function main() {
    const qmlRoot = '.';
    if (!fs.existsSync(qmlRoot)) {
        console.error(`Directory '${qmlRoot}' does not exist.`);
        return;
    }

    const subdirs = findSubdirectories(qmlRoot);
    subdirs.push(qmlRoot); // Include the root qml directory if it has QML files

    for (const dir of subdirs) {
        const items = fs.readdirSync(dir);
        const qmlFiles = items.filter(item => item.endsWith('.qml'));
        // Filter to include only files whose name starts with a capital letter
        const validQmlFiles = qmlFiles.filter(file => {
            const fileName = path.parse(file).name;
            return fileName.length > 0 && fileName[0] === fileName[0].toUpperCase();
        });
        if (validQmlFiles.length > 0) {
            const qmldirContent = generateQmldirContent(dir, validQmlFiles);
            const qmldirPath = path.join(dir, 'qmldir');
            const fileExists = fs.existsSync(qmldirPath);
            try {
                fs.writeFileSync(qmldirPath, qmldirContent, 'utf8');
                if (fileExists) {
                    console.log(`Updated existing qmldir at ${qmldirPath}`);
                } else {
                    console.log(`Created new qmldir at ${qmldirPath}`);
                }
            } catch (error) {
                console.error(`Error writing qmldir at ${qmldirPath}: ${error.message}`);
            }
        }
    }
}

main();