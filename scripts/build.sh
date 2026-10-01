#!/usr/bin/env bash
set -e

source scripts/color.sh

VENDOR_SRC_DIR="src/vendor"

show_help() {
    echo "Usage: $0 <vendor> <platform> [build-type]"
    echo "       $0 all [build-type]"
    echo
    echo "Examples:"
    echo "  $0 intel 600 debug"
    echo "  $0 amd xxx release"
    echo "  $0 all debug          # builds every discovered vendor/platform pair"
    echo
    echo "Available vendor/platform combinations:"
    discover_variants | sed 's/^/  /'
}

# Walks src/vendor/<vendor>/<platform>/ and prints "vendor platform" pairs,
# one per line. Skips any "common" directory (vendor-shared code, not a platform).
discover_variants() {
    for vendor_dir in "${VENDOR_SRC_DIR}"/*/; do
        [[ -d "${vendor_dir}" ]] || continue
        local vendor
        vendor="$(basename "${vendor_dir}")"
        for platform_dir in "${vendor_dir}"*/; do
            [[ -d "${platform_dir}" ]] || continue
            local platform
            platform="$(basename "${platform_dir}")"
            [[ "${platform}" == "common" ]] && continue
            echo "${vendor} ${platform}"
        done
    done
}

variant_exists() {
    local vendor="$1" platform="$2"
    discover_variants | grep -qx "${vendor} ${platform}"
}

normalize_build_type() {
    case "${1,,}" in
        debug)   echo "Debug" ;;
        release) echo "Release" ;;
        "")      echo "Debug" ;;
        *)       print_color red "Error: Unknown build type '$1'."; exit 1 ;;
    esac
}

build_bootloader() {
    echo "Building bootloader..."
    cmake --workflow --preset bootloader
}

build_variant() {
    local vendor="$1" platform="$2" buildType="$3"
    local binDir="build/${vendor}-${platform}"

    echo "Building ${vendor} ${platform} [${buildType}]..."
    cmake -B "${binDir}" -S . -G Ninja \
        -DCMAKE_BUILD_TYPE="${buildType}" \
        -DLEXVI_VENDOR="${vendor}" \
        -DLEXVI_PLATFORM="${platform}"
    cmake --build "${binDir}"
}

case "${1,,}" in
    -h|--help) show_help; exit 0 ;;
esac

if [[ -z "${1:-}" ]]; then
    print_color yellow "Warning: No vendor specified, defaulting to all."
    set -- all
fi

echo

built_dirs=("build/bootloader")

if [[ "${1,,}" == "all" ]]; then
    buildType="$(normalize_build_type "${2:-}")"
    build_bootloader

    mapfile -t VARIANTS < <(discover_variants)
    if [[ ${#VARIANTS[@]} -eq 0 ]]; then
        print_color red "Error: No vendor/platform directories found under ${VENDOR_SRC_DIR}"
        exit 1
    fi

    for pair in "${VARIANTS[@]}"; do
        read -r vendor platform <<< "${pair}"
        build_variant "${vendor}" "${platform}" "${buildType}"
        built_dirs+=("build/${vendor}-${platform}")
    done
else
    vendor="${1,,}"
    platform="${2:-}"
    buildType="$(normalize_build_type "${3:-}")"

    if [[ -z "${platform}" ]]; then
        print_color red "Error: Platform required. Usage: $0 <vendor> <platform> [build-type]"
        exit 1
    fi

    if ! variant_exists "${vendor}" "${platform}"; then
        print_color red "Error: No sources at ${VENDOR_SRC_DIR}/${vendor}/${platform}"
        print_color yellow "Available combinations:"
        discover_variants | sed 's/^/  /'
        exit 1
    fi

    build_bootloader
    build_variant "${vendor}" "${platform}" "${buildType}"
    built_dirs+=("build/${vendor}-${platform}")
fi

# Merge compile_commands.json from every tree actually built, so clangd
# resolves bootloader + whichever kernel variant(s) without switching.
python3 -c "
import json, sys
merged = []
for f in sys.argv[1:]:
    try:
        merged += json.load(open(f))
    except FileNotFoundError:
        pass
json.dump(merged, open('build/compile_commands.json', 'w'), indent=2)
" "${built_dirs[@]/%//compile_commands.json}"

echo
print_color green "Successfully built Lexvi OS"
