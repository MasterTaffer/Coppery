# Coppery game engine 

**0.1.0**

2D OpenGL game engine


## Building
Fetch the following header only libraries:

- rapidxml
- stb_image
- tinydir
- cpptoml

Also make sure to have the following libraries

- Lua 5.3
- SFML
- GLFW3
- GLEW
- OpenGL
- AngelScript 2.31.2 & official AngelScript addons

Get [OpenGL Loader Generator](https://bitbucket.org/alfonse/glloadgen/wiki/Home) and run it

    lua LoadGen.lua -style=pointer_c -spec=gl -version=4.1 -profile=compatibility compatibility_4_1
    
Put the resulting files (gl_compatibility_4_1.c & gl_compatibility_4_1.h) into src/GL directory.

And compile either with the aid of CMake or manually


## And then what?

Toss the binary to the [bin](bin) folder. See the [readme](bin/readme.md) as well.

## aminc

[aminc](aminc) is a folderful of header only stuff.

## GHMAS

[GHMAS](src/GHMAS) has a bunch of AngelScript extensions. If you're interested in cool AngelScript
extensions: these are for you! See [the example game](bin/data/angelscript) and the [AngelScript tests](bin/data/angelscript)
for more insight (maybe).

**BinaryStreamBuilder:**

Extension to the CScriptBuilder add on, adds support for loading and saving bytecode with
CScriptBuilder metadata.

**Coroutines:**

Support for cooperatively multitasked co-routines.

**Context pool:**

Perhaps the simplest and most naïve context pool implementation.

**Entity:**

A Big Thing. Implements pseudo-entity-component-system to AngelScript. See
example game source at [bin/data/angelscript](bin/data/angelscript). 

**Random:**

Seeded random generator for AngelScript.

**Thread:**

Support for threads in AngelScript. Very dangerous, considering that I
haven't tested them exhaustively.


## Where's the documentation

Soon, maybe.

