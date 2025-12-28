argc=$#;
action=$1;

if [[ $argc -lt 1 || $argc -gt 2 ]]; then
    echo "usage: ./utility.sh [help | build]\nOptions:\nbuild <preset-name>\n\tnote: See CMakePresets.json for presets.";
    exit 1;
fi

if [[ $action = "help" ]]; then
    echo "usage: ./utility.sh [help | build]\nOptions:\nbuild <preset-name>\n\tnote: See CMakePresets.json for presets.";
elif [[ $action = "build" && $argc -eq 2 ]]; then
    rm -f ./build/lexical_analyzer;
    rm -f ./build/compile_commands.json;
    cmake -S . -B build --preset $2 && cmake --build build || echo "\x1b[1;31mBuild failed!\x1b[0m";
else
    echo "usage: ./utility.sh [help | build]\nOptions:\nbuild <preset-name>\n\tnote: See CMakePresets.json for presets.";
    exit 1;
fi
