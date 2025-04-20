<div align=center>

# Contributing to AUR Graphical Helper

</div>

Thank you for taking the time to contribute to this project,
any help given to this project is very much appreciated. <br>

See the [Table of Content](#table-of-contents) for different ways to help this project.

## Table of Contents
- [Bug Report](#bug-report)
- [Coding Style](#code-style)
- [Suggestions](#suggestions)

### Bug Report

A good bug report must be concise, and follow the [KISS](https://en.wikipedia.org/wiki/KISS_principle)
principle, you must give enough information about the bugs so that the maintainers wouldnt need to test
everything themself. Thus resulting in a faster fix.

- Make sure you are using the latest version.
- Check if you bug is a bug with the program, and not of outside influence.
- Check if other users have experienced the bugs before.
- Collect information regarding the bug; which includes, but not limited to:
  - Log messages
  - Which line of code the bug happened at
  - Error messages

### Code Style
All code submitted to the project must adhere to these style/limitations:
#### File Layout
- Optional comment with LICENSE and or a short explanation of the file
- Headers
- Macros
- Forward declerations
- Types
- Global variables
- Functions

### C++ Features
- Follow the .clangd instructions
- Do not use C-style variadic macros
- Use `/* */` for comments, not `//`
- Reduce the usage of preprocessor statements

### Indentation
- Do not indent class modifiers
- Do not indent cases in a switch-case statement

### Functions
- Use trailing return types
- Put the `auto` in their own line if it a function definition
- Function name and argument list on the next line
- Brackets should always be placed in a new line
- Put the 2 brackets in a single line if the content of the function is short enough `{ buzz; }`
