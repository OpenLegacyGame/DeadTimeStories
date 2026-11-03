#!/usr/bin/env bash
set -euo pipefail
[[ "$(uname -s)" == Darwin ]] || { echo 'Build on macOS.' >&2; exit 1; }
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$root"
build="$root/build/macos-release"
output="$root/dist/macos"
cmake -S . -B "$build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=27.0
cmake --build "$build" --parallel 4
python3 tools/package-macos.py "$build" "$output"
