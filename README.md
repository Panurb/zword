# README #

### How do I get set up? ###

MSVC:
* cmake . -B build -G "Visual Studio 17 2022" -A x64
* cd build
* cmake --build . --target install --config Release

Emscripten:
* emcmake cmake . -B build
* cd build
* cmake --build .
* emrun notk.html

### Uploading to Itch

EXE:
* Copy notk.exe, SDL2.dll, SDL2_image.dll, SDL2_mixer.dll, SDL2_ttf.dll, data to bin
* butler push bin panurb/notk:windows --userversion 0.2

HTML:
* Copy notk.data, notk.html, notk.js, notk.wasm to bin
* Rename notk.html to index.html
* butler push bin panurb/notk:html --userversion 0.2
