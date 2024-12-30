#!/bin/bash

# Paths to external libraries
SDL2_PATH=external/sdl2
IMGUI_PATH=external/imgui
IMGPLOT_PATH=external/implot
OPENGL_PATH=external/opengl

# Check for release or debug argument
if [ "$1" == "release" ]; then
    BUILD_TYPE="Release"
    COMPILER_OPTS="-O2 -std=c++20 -DNDEBUG"
    OUTPUT_DIR="Build/Release"
else
    BUILD_TYPE="Debug"
    COMPILER_OPTS="-g -std=c++20"
    OUTPUT_DIR="Build/Debug"
fi

# Include directories
INCLUDE_OPTS="-I$SDL2_PATH/include -I$OPENGL_PATH -I$IMGUI_PATH/include -I$IMGUI_PATH/backends -I$IMGPLOT_PATH/include"

# Libraries to link
LIB_OPTS="-L$SDL2_PATH/lib -lSDL2 -framework OpenGL -framework Cocoa -framework IOKit -framework CoreFoundation"

# Create Build folder if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Clean object files for release builds
if [ "$BUILD_TYPE" == "Release" ]; then
    rm -f "$OUTPUT_DIR"/*.o
fi

# Compile and link Anyma.cpp into the output directory
clang++ -o "$OUTPUT_DIR/anyma" $COMPILER_OPTS $INCLUDE_OPTS Src/Anyma.cpp $LIB_OPTS -ISrc

# Copy Assets folder to output directory
rsync -a Assets "$OUTPUT_DIR/"

# Copy SDL2 dynamic library to output directory
cp "$SDL2_PATH/lib/libSDL2.dylib" "$OUTPUT_DIR/"

echo "Build complete: $OUTPUT_DIR/anyma"
