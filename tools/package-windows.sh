#!/usr/bin/env bash
set -euo pipefail
if [[ "${MSYSTEM:-}" != UCRT64 || $# -gt 3 ]]; then
  echo 'Usage (MSYS2 UCRT64): tools/package-windows.sh [assets-or-legacy-PAK-dir] [build/windows-release] [dist/windows]' >&2
  exit 1
fi
for tool in cmake makensis cygpath pacman sha256sum; do command -v "$tool" >/dev/null; done
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$root"
data=$(realpath "$(cygpath -u "${1:-assets}")")
build=$(realpath -m "$(cygpath -u "${2:-build/windows-release}")")
output=$(realpath -m "$(cygpath -u "${3:-dist/windows}")")
# Keep generated and replaceable directories inside their ignored roots.
[[ "$build" == "$root/build/"* && "$output" == "$root/dist/"* ]] || {
  echo 'Build and output directories must be under this repository build/ and dist/.' >&2; exit 1;
}
if [[ -d "$data/resources" && -d "$data/localization" ]]; then
  asset_source=$data
  data="$build/data"
  data_options=("-DDEADTIME_ASSET_DIR=$(cygpath -m "$asset_source")" -DDEADTIME_DATA_DIR=)
else
  asset_source=
  data_options=("-DDEADTIME_DATA_DIR=$(cygpath -m "$data")")
fi
cmake -S . -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DDEADTIME_WINDOWS_BUNDLE=ON "${data_options[@]}"
cmake --build "$build" --parallel 4
"$build/deadtime.exe" --assets "$(cygpath -m "$data")" --audit
stage="$build/package-stage/DeadTimeStories"
rm -rf -- "$stage"
mkdir -p "$stage" "$output"
cmake --install "$build" --prefix "$(cygpath -m "$stage")"

# Ship the installed runtime packages' own notices, together with exact versions.
# No compiler, system DLL, tests, original executable or downloaded launcher is staged.
runtime_files=()
while IFS= read -r -d '' dll; do
  runtime_files+=("/ucrt64/bin/$(basename "$dll")")
done < <(find "$stage" -maxdepth 1 -type f -iname '*.dll' -print0)
[[ ${#runtime_files[@]} -gt 0 ]] || { echo 'No runtime libraries staged.' >&2; exit 1; }
owners=$(pacman -Qqo "${runtime_files[@]}")
mapfile -t packages < <(printf '%s\n' "$owners" | LC_ALL=C sort -u)
[[ ${#packages[@]} -gt 0 ]] || { echo 'Cannot identify runtime library packages.' >&2; exit 1; }
mkdir -p "$stage/licenses"
pacman -Q "${packages[@]}" | LC_ALL=C sort > "$stage/licenses/runtime-packages.txt"
license_files=$(pacman -Qlq "${packages[@]}")
while IFS= read -r file; do
  [[ "$file" == /ucrt64/share/licenses/* && -f "$file" ]] || continue
  destination="$stage/licenses/${file#/ucrt64/share/licenses/}"
  mkdir -p "$(dirname "$destination")"
  cp -- "$file" "$destination"
done < <(printf '%s\n' "$license_files" | LC_ALL=C sort -u)
cp tools/runtime-libraries.txt "$stage/licenses/README.txt"

version=$(cat "$build/release-version.txt")
name="DeadTimeStories-$version-windows-x64"
uninstall="$build/package-stage/uninstall-files.nsh"
{
  while IFS= read -r file; do
    relative=${file#"$stage/"}; relative=${relative//\//\\}; relative=${relative//\$/\$\$}
    printf 'Push "$INSTDIR\\%s"\nCall un.DeletePayloadFile\n' "$relative"
  done < <(find "$stage" -type f | LC_ALL=C sort)
  while IFS= read -r directory; do
    [[ "$directory" != "$stage" ]] || continue
    relative=${directory#"$stage/"}; relative=${relative//\//\\}; relative=${relative//\$/\$\$}
    printf 'RMDir "$INSTDIR\\%s"\n' "$relative"
  done < <(find "$stage" -depth -type d)
} > "$uninstall"

# Both release artifacts contain this same staged payload.
MSYS2_ARG_CONV_EXCL='*' makensis -V2 -WX -INPUTCHARSET UTF8 \
  "-DPAYLOAD_DIR=$(cygpath -w "$stage")" "-DVERSION=$version" \
  "-DGAME_ICON=$(cygpath -w "$root/assets/windows/deadtime.ico")" \
  "-DUNINSTALL_FILES=$(cygpath -w "$uninstall")" \
  "-DOUTPUT_FILE=$(cygpath -w "$output/$name-installer.exe")" \
  "$(cygpath -w "$root/tools/windows-installer.nsi")"
(cd "$build/package-stage" && cmake -E tar cf "$(cygpath -m "$output/$name-portable.zip")" --format=zip DeadTimeStories)
(cd "$output" && sha256sum "$name-portable.zip" "$name-installer.exe" > "$name-SHA256SUMS.txt")
echo "Release artifacts: $output/$name-portable.zip and $output/$name-installer.exe"
