# Cranked Contribution Guidelines

## Bugs
The project is still in an early state as far as usability and compatibility are concerned, so no support can be provided
for bugs or crashes without a debugger running the emulator to inspect stack traces and variable contents. Once better
logging is implemented, a log file with a sufficient verbosity should be provided.

## Priorities
- The main priority is currently getting compatibility to a good enough state where real games can start to be played.
- Reverse engineering and implementing the exact behavior of API functions should come before trying to optimize any
API implementations (Slow rendering functions until tested to be pixel-perfect for all modifiers and parameters, for example).
- Improving the project documentation, usability, and accessibility (Install scripts, builds, error handling, logging, etc.).

## Styling
There is not a well-defined code style, but trying to match the existing style is preferred. Some specific guidelines:
- Avoid extra whitespace at end of line or blank lines.
- End files with a blank line.
- Use only one statement per line.
- Curly braces on same line as statements/loops.
- Use four space indentation.
- C++ used namespaces/names are brought into *cranked* namespace in *Utils.hpp* (A stylistic/convenience oriented experiment).
- Using good/established C++ practices and improving code quality when possible is prioritized (Careful memory management, reusing well-tested helper functions, etc.).

## AI Contributions
This is a hobby project, so generative AI produced code is not welcome. See [AGENTS](AGENTS.md) for the specifics.
