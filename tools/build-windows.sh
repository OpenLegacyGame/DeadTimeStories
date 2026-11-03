#!/usr/bin/env bash
set -euo pipefail
if [[ "${MSYSTEM:-}" != UCRT64 ]]; then
  echo 'Run this script inside the MSYS2 UCRT64 environment.' >&2
  exit 1
fi
cd "$(dirname "${BASH_SOURCE[0]}")/.."
cmake -S . -B build/windows -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/windows --parallel 4
