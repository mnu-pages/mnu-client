# mnu

Welcome to `mnu`! We believe documentation should be simple, scannable, and easy on the eyes. This is a POSIX C terminal document client designed specifically for the `.mn` format.

> **Looking for the Node.js version?** You can find it at [github.com/mnu-pages/mnu-nodejs-client](https://github.com/mnu-pages/mnu-nodejs-client).

It's like `man` or `less`, but focused on getting you the answers you need without the wall of text.

## Getting Started

To use `mnu`, you can build it directly from source. This ensures you have the latest version optimized for your specific system.

### Dependencies
Before building, ensure you have the following installed:
- **C Compiler** (`clang` or `gcc`)
- **libcurl** (including development headers)
- **make**

### Building from Source
```bash
# Clone the repository
git clone https://github.com/mnu-pages/mnu-client
cd mnu-client

# Compile the project
make
```

The compiled binary will be located at `build/mnu`.

### Manual Installation
To use `mnu` from anywhere in your terminal, move the binary to a folder in your system's PATH:

**Linux / macOS:**
```bash
sudo mv build/mnu /usr/local/bin/
```

**Android (Termux):**
```bash
mv build/mnu $PREFIX/bin/
```

**Windows:**
Move `build/mnu` to a folder included in your User PATH environment variable.

### Verify Installation
Run the following to ensure it's working:
```bash
mnu help
```

## How to Use It

Using `mnu` is straightforward. Just tell it which category and page you want to see:

```bash
mnu category:page
```

**Try these examples:**
```bash
mnu cli:git
mnu help
```

## The .mn Syntax

We keep things structured so you can focus on writing. Here is how a `.mn` file looks:

- `.TITLE "text"`: Centered at the top, bold and underlined.
- `.DIV "text"`: A section header with nice padding.
- **Normal text**: Just write! It wraps automatically with clean margins.
- `**bold**`: For when you really need to highlight a command.
- `__underline__`: For important terms or folders.

## Navigation

Once you're in a document, you can move around using familiar keys:
- `j` or `ArrowDown`: Scroll down
- `k` or `ArrowUp`: Scroll up
- `g`: Jump to the very top
- `G`: Jump to the very bottom
- `h`: Show help page
- `q`: Quit and get back to your shell

## Contributing

If you want to improve the `mnu` tool itself, check out [CONTRIBUTING.md](./CONTRIBUTING.md).

To contribute new documentation pages or fix existing ones, please visit the [MNU Pages repository](https://github.com/mnu-pages/pages).

## License

MIT
