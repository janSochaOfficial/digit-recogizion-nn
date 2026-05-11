# Laboratory XXX
This project template is made for Programming Laungages class in 2026.

Template made by Jan Socha.
## Usage:
### First use
When first using this template run `./scripts/setup.sh` to setup directory structure and create any missing files. This script also create a git repository for verion controll.
### Development
For standard development use `cmake --build build/debug` for compiling in debug mode. For formating and clang-tidy I recomend VSC pugins, but targets `tidy`, `tidy-fix`, `format` and `format-check` are avaliable by `cmake --build build/debug --target [target]`
### Release 
For release build, make sure that tidy and format are correct, then run `cmake --build build/release -j$(nproc)`. The symbolic link to all of the executables will be avaliable at `bin/` folder
## Folder structure
```
.
├── bin -- links from release folder
│   ├── task1 -> /build/release/folder/task1
│   ├── task2 -> /build/release/folder/task1
│   └── task3 -> /build/release/folder/task1 
├── .clangd -- clang options for VSC extentions
├── .clang-format -- formatter options
├── .clang-tidy -- tidy options
├── CMakeLists.txt 
├── .gitignore
├── README.md -- current file
├── scripts -- new scripts planned for the future
│   └── setup.sh -- setup script from eariel
└── src -- source files in format 'task*.cpp'
    ├── task1.cpp
    ├── task2.cpp
    └── task3.cpp

```
