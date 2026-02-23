# LLM-Assist-Clip (LAC)

**LLM-Assist-Clip (LAC)** is a simple utility designed to enhance the way you visualize, extract, and share project structures and code. With its intuitive tree-like structure visualization, recursive file search, dynamic `.gitignore` parsing, and seamless clipboard integration, LAC simplifies the process of bundling both the structure and content of project files. It’s a versatile tool perfect for developers, technical writers, and teams who need to easily paste entire project contexts into Large Language Models (LLMs) like ChatGPT, Claude, or Gemini for better AI understanding and assistance.

<div align="center">
    <img src="Screenshot.png" alt="app Screenshot" width="800"/>
</div>

## Key Features
- **LLM-Optimized Output**: Aggregates code files and directory structures into a single, well-formatted string optimized for AI context windows.
- **Project(directory) Tree Visualization**: Visualize project directories and file structures in a clean, hierarchical ASCII tree format.
- **.gitignore Support**: Automatically parses and respects `.gitignore` rules (including wildcards, directory exclusions, and negations) to keep unwanted files out of your clipboard context.
- **Selective Recursive File Search**: Specify file types for recursive directory searches, providing flexibility in the exact files you target.
- **Custom Target Directories**: Run the tool against any specific path on your system without needing to `cd` into it first.
- **Clipboard Integration**: Automatically formats and copies file paths, contents, and tree structures directly to your clipboard.
- **Cross-Platform Compatibility**: Works seamlessly on Linux, macOS, and Windows.

## Usage
To get the most out of **LLM-Assist-Clip (LAC)**, first add it to your system's PATH so it can be invoked from any directory. Compile and run the program from your terminal or command prompt, specifying options and file extensions as arguments. 

### Adding LAC to Your System PATH
To make **LAC** accessible from any directory, you need to add the compiled binary to your system's PATH.

#### Linux / macOS
1. After building the project, find the location of the compiled binary (e.g., `/path/to/LLM-Assist-Clip/build/LAC`).
2. Open your shell configuration file (e.g., `.bashrc`, `.zshrc`):
   ```bash
   nano ~/.bashrc

```

3. Add the following line, replacing `/path/to/LLM-Assist-Clip/build` with the actual path where **LAC** is located:
```bash
export PATH=$PATH:/path/to/LLM-Assist-Clip/build

```


4. Save and close the file. Then reload the shell configuration:
```bash
source ~/.bashrc

```



#### Windows

1. After building **LAC**, find the location of the compiled binary (e.g., `C:\path\to\LLM-Assist-Clip\build\LAC.exe`).
2. Open the System Properties by searching for "Environment Variables."
3. Under "System variables," find the `Path` variable, select it, and click "Edit."
4. Add the path to your **LAC** executable directory (e.g., `C:\path\to\LLM-Assist-Clip\build`).
5. Click "OK" to close the dialogs, and restart your terminal or command prompt.

### Command Syntax

Once **LAC** is in your PATH, you can run it globally:

```bash
LAC [options] <file extensions>

```

### Options

* `-r, --recursive`: Apply recursive search to subdirectories for matching files.
* `-i, --ignore`: Respect `.gitignore` rules. Dynamically ignores files and folders specified in any `.gitignore` found in the search tree.
* `-t, --tree`: Generate a tree-structured view of the project directory (automatically respects the ignore flag).
* `-p, --print`: Print the copied content directly to the terminal.
* `--dir=<path>`: Specify a custom starting directory. If omitted, the current working directory is used. (Use quotes for paths with spaces: `--dir="/my path/here"`).
* `-h, --help`: Show the comprehensive help message and manual.

### Examples

* Bundle all `.cpp` and `.h` files in the current directory non-recursively:
```bash
LAC .cpp .h

```


* Recursively bundle `.py` and `.md` files, ignoring anything listed in your `.gitignore`:
```bash
LAC -r -i .py .md

```


* Generate a project tree and bundle `.ts` files from a specific custom directory:
```bash
LAC -t --dir=/path/to/project .ts

```


* Use a path with spaces, apply `.gitignore` rules, and print the output to the console:
```bash
LAC -p -i --dir="/path/with spaces/project" .js

```



## Building the Project

To build **LLM-Assist-Clip (LAC)**, follow these steps:

1. **Clone and navigate to the project**:
```bash
git clone https://github.com/iman-zamani/LLM-Assist-Clip.git
cd LLM-Assist-Clip

```


2. **Initialize required submodules**:
```bash
git submodule update --init --recursive

```


3. **Compile the application**:
```bash
mkdir build && cd build
cmake ..
cmake --build .

```


4. **Add the compiled binary to your system’s PATH** (see instructions above).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

