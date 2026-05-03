# mnu

Welcome to `mnu`! We believe documentation should be simple, scannable, and easy on the eyes. This is a POSIX C terminal document client designed specifically for the `.mn` format.

> **Looking for the Node.js version?** You can find it at [github.com/mnu-pages/mnu-nodejs-client](https://github.com/mnu-pages/mnu-nodejs-client).

It's like `man` or `less`, but focused on getting you the answers you need without the wall of text.

## Getting Started

The easiest way to get `mnu` is to download a pre-built binary for your system from the [Releases](https://github.com/mnu-pages/mnu-client/releases) page.

### Pre-built Binaries
1. **Download**: Grab the binary for your platform (Linux, macOS, or Windows).
2. **Make Executable** (Linux/macOS): `chmod +x mnu-<platform>`
3. **Run**: `./mnu-<platform> cli:git`

### Building from Source
If a binary isn't available for your system, or if you prefer to build from source, ensure you have `libcurl` and `make` installed, then run:

```bash
# Clone the repository
git clone https://github.com/mnu-pages/mnu-client
cd mnu-client

# Compile the project
make

# Run it!
./build/mnu cli:git
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
