#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <regex>
#include "./clip/clip.h"

// Class to manage, parse, and match .gitignore rules dynamically
class IgnoreManager {
public:
    struct Rule {
        std::regex pattern;
        bool isDirOnly;
        bool isNegation;
        bool isFullPath;
        std::filesystem::path baseDir;
    };

    bool active = false;
    std::vector<Rule> rules;

    void loadGitIgnore(const std::filesystem::path& gitignorePath) {
        if (!active) return;
        std::ifstream file(gitignorePath);
        if (!file) return;

        std::filesystem::path baseDir = gitignorePath.parent_path();
        std::string line;

        while (std::getline(file, line)) {
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty() || line[0] == '#') continue;

            bool isNegation = false;
            if (line[0] == '!') {
                isNegation = true;
                line = line.substr(1);
            }

            bool isDirOnly = false;
            if (!line.empty() && line.back() == '/') {
                isDirOnly = true;
                line.pop_back();
            }

            bool isFullPath = false;
            if (line.find('/') != std::string::npos) {
                isFullPath = true;
                if (line[0] == '/') line = line.substr(1);
            }

            // Convert glob pattern to regex
            std::string regexStr = "^";
            for (size_t i = 0; i < line.length(); ++i) {
                if (i + 1 < line.length() && line.substr(i, 2) == "**") {
                    regexStr += ".*";
                    i++;
                    if (i + 1 < line.length() && line[i+1] == '/') {
                        regexStr += "(/.*)?";
                        i++;
                    }
                } else if (line[i] == '*') {
                    regexStr += "[^/]*";
                } else if (line[i] == '?') {
                    regexStr += "[^/]";
                } else if (std::string(".+()[]{}^$|\\").find(line[i]) != std::string::npos) {
                    regexStr += "\\";
                    regexStr += line[i];
                } else {
                    regexStr += line[i];
                }
            }
            regexStr += "$";

            try {
                Rule r;
                r.pattern = std::regex(regexStr);
                r.isDirOnly = isDirOnly;
                r.isNegation = isNegation;
                r.isFullPath = isFullPath;
                r.baseDir = baseDir;
                rules.push_back(r);
            } catch (const std::regex_error&) {
                // Ignore invalid patterns gracefully
            }
        }
    }

    bool isIgnored(const std::filesystem::path& path, bool isDir) const {
        if (!active) return false;
        
        // Always safeguard the .git folder itself
        if (path.filename() == ".git") return true;

        bool ignored = false;
        for (const auto& rule : rules) {
            if (rule.isDirOnly && !isDir) continue;

            std::string checkStr;
            if (rule.isFullPath) {
                std::error_code ec;
                std::filesystem::path rel = std::filesystem::relative(path, rule.baseDir, ec);
                if (ec) continue;
                checkStr = rel.lexically_normal().generic_string();
            } else {
                checkStr = path.filename().generic_string();
            }

            if (std::regex_match(checkStr, rule.pattern)) {
                ignored = !rule.isNegation;
            }
        }
        return ignored;
    }
};

std::string readFileContent(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        std::cerr << "Error: could not open file " << filePath << std::endl;
        return ""; 
    }

    std::string content, line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    file.close();
    return content;
}

// Unified manual recursion function so .gitignore scope is respected
void collectFiles(
    const std::filesystem::path& currentDir,
    const std::filesystem::path& rootDir,
    const std::string& ext,
    bool recursive,
    IgnoreManager ignoreMgr, // Copied by value to scope rules to this branch
    std::vector<std::string>& filePaths
) {
    if (ignoreMgr.active && std::filesystem::exists(currentDir / ".gitignore")) {
        ignoreMgr.loadGitIgnore(currentDir / ".gitignore");
    }

    try {
        for (const auto& entry : std::filesystem::directory_iterator(currentDir)) {
            bool isDir = entry.is_directory();
            
            // Skip ignored files and folders immediately
            if (ignoreMgr.isIgnored(entry.path(), isDir)) {
                continue;
            }

            if (isDir) {
                if (recursive) {
                    collectFiles(entry.path(), rootDir, ext, recursive, ignoreMgr, filePaths);
                }
            } else {
                if (entry.path().extension() == ext) {
                    std::filesystem::path relativePath = std::filesystem::relative(entry.path(), rootDir);
                    filePaths.push_back(relativePath.string());
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Directory access error: " << e.what() << std::endl;
    }
}

std::vector<std::string> getFilesByTypeRecursive(const std::string& dirPath, const std::string& fileType, IgnoreManager ignoreMgr) {
    std::vector<std::string> filePaths;
    std::filesystem::path directory(dirPath);

    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        std::cerr << "Error accessing the directory: " << dirPath << std::endl;
        return filePaths;
    }

    collectFiles(directory, directory, fileType, true, ignoreMgr, filePaths);
    return filePaths;
}

std::vector<std::string> getFilesByType(const std::string& dirPath, const std::string& fileType, IgnoreManager ignoreMgr) {
    std::vector<std::string> filePaths;
    std::filesystem::path directory(dirPath);

    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        std::cerr << "Error accessing the directory: " << dirPath << std::endl;
        return filePaths;
    }

    collectFiles(directory, directory, fileType, false, ignoreMgr, filePaths);
    return filePaths;
}

std::string list_files(const std::filesystem::path& startPath, IgnoreManager ignoreMgr, const std::string& prefix = "", bool isLast = true) {
    std::string ret;
    ret += prefix + (isLast ? "└── " : "├── ") + startPath.filename().string() + "/\n"; 

    if (ignoreMgr.active && std::filesystem::exists(startPath / ".gitignore")) {
        ignoreMgr.loadGitIgnore(startPath / ".gitignore");
    }

    std::filesystem::directory_iterator iter(startPath);
    std::vector<std::filesystem::directory_entry> entries;
    for (const auto& entry : iter) {
        if (!ignoreMgr.isIgnored(entry.path(), entry.is_directory())) {
            entries.push_back(entry);
        }
    }

    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        bool isEntryLast = (i == entries.size() - 1); 

        std::string newPrefix = prefix + (isLast ? "    " : "│   ");

        if (entry.is_directory()) {
            ret += list_files(entry.path(), ignoreMgr, newPrefix, isEntryLast);
        } else {
            ret += newPrefix + (isEntryLast ? "└── " : "├── ") + entry.path().filename().string() + "\n";
        }
    }

    return ret;
}

void printHelp() {
    std::cout << "Usage: ./LAC [options] <file extensions>\n\n"
        << "Description:\n"
        << "  LLM-Assist-Clip (LAC) is a developer utility that aggregates code files and \n"
        << "  directory structures into a single, well-formatted string and copies it \n"
        << "  directly to your clipboard. This makes it effortless to provide context to \n"
        << "  Large Language Models (LLMs) like ChatGPT, Claude, or Gemini.\n\n"
        << "Options:\n"
        << "  -r, --recursive       Recursively search through subdirectories for matching files.\n"
        << "  -i, --ignore          Respect .gitignore rules. Dynamically ignores files and \n"
        << "                        folders specified in any .gitignore found in the search tree.\n"
        << "  -t, --tree            Prepend a visual ASCII tree of the directory structure to \n"
        << "                        the clipboard output. Automatically respects the ignore flag.\n"
        << "  -p, --print           Print the final clipboard content to the console (stdout).\n"
        << "  --dir=<path>          Specify a custom starting directory. If omitted, the \n"
        << "                        current working directory is used. \n"
        << "                        Use quotes for paths with spaces: --dir=\"/my path/here\"\n"
        << "  -h, --help            Show this comprehensive help message and exit.\n\n"
        << "Examples:\n"
        << "  1. Bundle all .cpp and .h files in the current directory:\n"
        << "     ./LAC .cpp .h\n\n"
        << "  2. Recursively bundle .py and .md files, ignoring .gitignore contents:\n"
        << "     ./LAC -r -i .py .md\n\n"
        << "  3. Generate a directory tree and bundle .ts files from a specific path:\n"
        << "     ./LAC -t --dir=/path/to/project .ts\n\n"
        << "  4. Use a path with spaces and print the output to the console:\n"
        << "     ./LAC -p --dir=\"/path/with spaces/project\" .js\n\n"
        << "External Libraries Used:\n"
        << "  - Clip: A cross-platform library used for clipboard operations.\n\n"
        << "For more information, visit: https://github.com/iman-zamani/LLM-Assist-Clip\n";
}

int main(int argc, char *argv[]) {
    if (argc <= 1){
        std::cerr << "Error: You need to specify extensions of the files you want to copy.\n"
                  << "Type ./LAC --help for usage information." << std::endl;
        return EXIT_FAILURE;
    }

    std::string dirPath;
    try {
        dirPath = std::filesystem::current_path().string();
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error getting the current directory: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    std::string clipboard;
    bool recursive = false;
    bool printAtTheEnd = false;
    IgnoreManager baseIgnoreMgr;

    // Pass 1: Global Flags Processing (Help, Dir, Options)
    for (int i = 1; i < argc; i++) {
        std::string arg = std::string(argv[i]);
        
        if (arg == "-h" || arg == "--help" || arg == "--h" || arg == "-help") {
            printHelp();
            return 0;
        } else if (arg == "-r" || arg == "--recursive") {
            recursive = true;
        } else if (arg == "-p" || arg == "--print") {
            printAtTheEnd = true;
        } else if (arg == "-i" || arg == "--ignore") {
            baseIgnoreMgr.active = true;
        } else if (arg.rfind("--dir=", 0) == 0) { // Check if string starts with "--dir="
            std::string targetDir = arg.substr(6);
            // In case quotes actually passed through the shell
            if (targetDir.size() >= 2 && targetDir.front() == '"' && targetDir.back() == '"') {
                targetDir = targetDir.substr(1, targetDir.size() - 2);
            }
            dirPath = targetDir;
        }
    }

    // Verify the requested directory exists before proceeding
    std::filesystem::path targetPath(dirPath);
    if (!std::filesystem::exists(targetPath) || !std::filesystem::is_directory(targetPath)) {
        std::cerr << "Error: The specified directory does not exist or is not a folder: " << dirPath << std::endl;
        return EXIT_FAILURE;
    }

    // Pass 2: Execution (Tree and Extensions)
    for (int i = 1; i < argc; i++) {
        std::string extension = std::string(argv[i]);
        
        // Skip parsed flags so they aren't treated as file extensions
        if (extension == "-r" || extension == "--recursive" || 
            extension == "-p" || extension == "--print" || 
            extension == "-i" || extension == "--ignore" ||
            extension.rfind("--dir=", 0) == 0) {
            continue;
        }
        else if (extension == "-t" || extension == "--tree"){
            clipboard += list_files(targetPath, baseIgnoreMgr) + "\n\n";
            continue;
        }

        // Add '.' to extension specified by user if it doesn't have one  
        if (extension.at(0) != '.') {
            extension = "." + extension;
        }

        std::vector<std::string> filesWithThisExtension;
        if (recursive) filesWithThisExtension = getFilesByTypeRecursive(dirPath, extension, baseIgnoreMgr);
        else filesWithThisExtension = getFilesByType(dirPath, extension, baseIgnoreMgr);
        
        if (filesWithThisExtension.empty()) { continue; }
        
        for (const std::string &file : filesWithThisExtension){
            clipboard += file + ":\n\n\n";
            // Important: read the file from the correct full path
            clipboard += readFileContent((targetPath / file).string()) + "\n\n\n";
        }
    }

    if (clipboard.empty()) {
        std::cout << "No matching files or output generated. Clipboard not updated." << std::endl;
        return 0;
    }

    clip::set_text(clipboard);
    if (printAtTheEnd) {
        std::cout << clipboard << std::endl;
    } else {
        std::cout << "Success: Data copied to clipboard!" << std::endl;
    }
    
    return 0;
}