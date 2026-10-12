#!/usr/bin/env bash
set -euo pipefail
export LC_ALL=C

root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${SXCC_BUILD_DIR:-$root/build/sxcc}"
shopt -s nullglob
sources=("$root"/scripts/*.sxc)
if ((${#sources[@]} == 0)); then
    printf 'No .sxc scripts found in %s/scripts\n' "$root" >&2
    exit 1
fi

cmake -S "$root/tools/sxcc" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release
cmake --build "$build_dir" --parallel "${CMAKE_BUILD_PARALLEL_LEVEL:-2}"
compiler="$(cd -- "$build_dir/bin" && pwd)/sxcc"
tmp="$(mktemp -d)"
trap 'rm -rf -- "$tmp"' EXIT
cd -- "$tmp"

for source in "${sources[@]}"; do
    name="${source##*/}"
    name="${name%.sxc}"
    "$compiler" -c "$source" -o "$name.pcsx"
    "$compiler" --extract-segments "$name.pcsx" -o "$name"
done

sha256sum -- *.bindings *.instructions *.strings | sort -k2 > actual.sha256
diff -u -- "$root/scripts/originals.sha256" actual.sha256
printf 'Verified %d scripts (%d SHA-256 checks).\n' "${#sources[@]}" "$((${#sources[@]} * 3))"
