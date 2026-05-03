# Contributing to mnu

> Our job is not to make things complex, but to make them simple and understandable.

Welcome to the `mnu` codebase. We value clean, disciplined, and beginner-friendly contributions. This guide outlines the standards we expect for the CLI client itself.

## Project Values

Our development is guided by these core principles:
- **Simplicity**: Prefer the simplest implementation that solves the problem.
- **Clarity**: Code and documentation must be easy to read and understand.
- **Structure**: Keep the codebase modular and well-organized.
- **Beginner-friendly**: Write for users and contributors who are just starting.
- **Terminal-friendly**: Ensure tools are optimized for the terminal environment.

## Writing Standards

- **Helpful README**: Keep the `README.md` playful yet practical.
- **No Hype**: Avoid fluff, exaggerated claims, or hyperbole. Use plain, precise language.
- **Scannable Docs**: Keep documentation short enough to scan but clear enough to understand.
- **Utility First**: Favor usefulness and precision over clever wording.

## Repository Structure (C Client)

We expect a predictable and modular organization:
- **include/**: Header files (`.h`).
- **source/**: Core implementation files (`.c`).
- **build/**: Compiled object files and the final binary.
- **Makefile**: Build configuration and automation.

Keep file names short, predictable, and focused on a single responsibility.

## Commit Message Rules

Use structured commit messages in the format: `type(scope): short summary`.

**Examples:**
- `docs(readme): improve usage section`
- `fix(parser): handle blank input`
- `feat(renderer): add line wrapping`

Keep messages short, specific, and useful. Avoid vague messages like `update stuff` or `fixes`.

## Technical Standards (C Client)

As a high-performance C project, we maintain strict technical requirements:
- **Memory Safety**: All contributions must be free of memory leaks and buffer overflows. Use the provided `make asan` target to build with AddressSanitizer, or use `Valgrind` to verify your changes.
- **Optimization**: Avoid redundant allocations in the rendering loop. Prefer stack allocation for small, fixed-size buffers and reuse `DynBuf`/`RenderBuf` structures where possible.
- **Portability**: Ensure code is POSIX-compliant (C11). Avoid compiler-specific extensions that break builds on macOS or Windows (via MSYS2).
- **Static Analysis**: We recommend running `clang-tidy` or the Clang Static Analyzer on new code to catch common C pitfalls early.

## AI-Assisted Contributions

If you use AI to assist with code or documentation:
- **Human Review**: Every line must be reviewed by a human before submission.
- **Consistency**: Ensure the output matches our project style and quality standards.
- **No Blind Merges**: AI output must never be merged without verification for correctness.

## Pull Request Checklist

Before submitting a Pull Request, verify that:
- [ ] The CLI still runs and behaves as expected on multiple platforms.
- [ ] Code has been checked for memory leaks (e.g., using ASan or Valgrind).
- [ ] Performance remains high with no unnecessary allocations in the render loop.
- [ ] The code is readable, modular, and follows existing patterns.
- [ ] The README remains helpful and playful.
- [ ] Commit messages follow the `type(scope): summary` format.

## Tone and Behavior

We aim for a calm, direct, and welcoming environment. We are disciplined about our standards because they keep the project open and maintainable for everyone. 

## Closing

We prefer small, clean, and focused contributions over large, complex, or messy changes. Thank you for helping us keep `mnu` simple.
