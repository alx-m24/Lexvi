#!/usr/bin/env bash
set -e

source scripts/color.sh

show_help() {
    echo "Usage: $0 <preset> <build-type>"
    echo
    echo "Presets:"
    echo "  intel       Build for Intel"
    echo "  amd         Build for AMD"
    echo "  auto        Build with Intel and AMD support"
    echo
    echo "Build types:"
    echo "  debug       Debug build"
    echo "  release     Release build"
    echo
    echo "Examples:"
    echo "  $0 intel debug"
    echo "  $0 amd release"
    echo "  $0 auto debug"
}

# Help
case "${1,,}" in
    -h|--help)
        show_help
        exit 0
        ;;
esac

preset="${1:-}"
buildType="${2:-}"

# Check preset
case "${preset,,}" in
    intel|amd|auto)
        ;;
    "")
        print_color yellow "Warning: No preset specified, defauting to auto."
        preset="auto"
        ;;
    *)
        print_color red "Error: Unknown preset '${preset}'."
        print_color yellow "Valid presets: intel, amd, auto"
        exit 1
        ;;
esac

# Check build type
case "${buildType,,}" in
    debug)
        buildType="Debug"
        ;;
    release)
        buildType="Release"
        ;;
    "")
        print_color yellow "Warning: No build type specified. Defaulting to debug"
        buildType="Debug"
        ;;
    *)
        print_color red "Error: Unknown build type '${buildType}'."
        print_color yellow "Valid build types: debug, release"
        exit 1
        ;;
esac

echo

cmake --preset "${preset}" -DCMAKE_BUILD_TYPE="${buildType}"
cmake --build --preset "${preset}"

cmake -E copy \
    "build/${preset}/compile_commands.json" \
    "build/compile_commands.json"

echo
print_color green "Successfully built Lexvi OS [${preset} / ${buildType}]"
