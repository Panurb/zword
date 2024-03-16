# README #

### How do I get set up? ###

MSVC:
* cmake . -B build -G "Visual Studio 17 2022" -A Win32
* cd build
* cmake --build . --target install --config Release

MinGW:
* Install mingw, add to path
* cmake . -G "MinGW Makefiles"
* cmake --build . --target install
