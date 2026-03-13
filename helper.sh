#!/bin/sh

action="$1";

if [[ $action = "help" ]]; then
    printf "USAGE:\\n\\thelper.sh [build | rebuild] <preset-name> <generator>\\n";
    exit 0;
elif [[ $action = "build" ]]; then
    cmake -S . -B build --preset "$2" -G "$3" && cmake --build build && cp ./build/compile_commands.json .;
elif [[ $action = "rebuild" ]]; then
    rm -rf ./build && cmake --fresh -S . -B build --preset "$2" -G "$3" && cmake --build build && cp ../build/compile_commands.json .;
else
    printf "USAGE:\\n\\thelper.sh rebuild <preset-name> <generator>\\n";
    exit 1;
fi
