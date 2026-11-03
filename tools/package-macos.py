"""Package a native macOS build and its Homebrew runtime dependencies."""
import json
import plistlib
import re
import shutil
import subprocess
import sys
from pathlib import Path


def run(*args):
    return subprocess.check_output([str(arg) for arg in args], text=True)


root = Path(__file__).resolve().parent.parent
build, output = (Path(arg).resolve() for arg in sys.argv[1:])
build.relative_to(root / 'build')
output.relative_to(root / 'dist')
version = re.search(r'project\(DeadTimeStories VERSION ([\d.]+)',
                    (root / 'CMakeLists.txt').read_text()).group(1)
stage = build / 'package-stage'
if stage.exists():
    shutil.rmtree(stage)
app = stage / 'Deadtime Stories.app'
contents = app / 'Contents'
macos = contents / 'MacOS'
resources = contents / 'Resources'
macos.mkdir(parents=True)
resources.mkdir()
shutil.copy2(build / 'deadtime', macos / 'deadtime')
shutil.copytree(build / 'data', resources / 'data')
# The compatibility library searches beside the executable for this exact name.
sdl_prefix = (Path(run('pkg-config', '--variable=libdir', 'sdl2').strip()) / 'libSDL2.dylib').resolve()
extra_origins = []
if 'sdl2-compat' in sdl_prefix.parts:
    sdl3 = Path(run('brew', '--prefix', 'sdl3').strip()) / 'lib/libSDL3.dylib'
    shutil.copy2(sdl3, macos / 'libSDL3.dylib')
    extra_origins.append(sdl3.resolve())
minimum = run('vtool', '-show-build', macos / 'deadtime')
minimum = re.search(r'minos\s+([\d.]+)', minimum).group(1)
plist = dict(CFBundleIdentifier='org.openlegacygame.deadtime',
             CFBundleName='Deadtime Stories', CFBundleDisplayName='亡灵故事',
             CFBundleExecutable='deadtime', CFBundlePackageType='APPL',
             CFBundleVersion=version, CFBundleShortVersionString=version,
             LSMinimumSystemVersion=minimum, NSHighResolutionCapable=True)
with (contents / 'Info.plist').open('wb') as stream:
    plistlib.dump(plist, stream)
origins = build / 'library-origins.txt'
print(run('cmake', f'-DAPP={app}', f'-DORIGINS={origins}',
          '-DLIBRARY_DIR=' + run('brew', '--prefix').strip() + '/lib',
          '-P', root / 'tools/BundleMacOS.cmake'))
cellar = Path(run('brew', '--cellar').strip()).resolve()
packages = {}
for original in [Path(line).resolve() for line in origins.read_text().splitlines()] + extra_origins:
    if not original.is_relative_to(cellar):
        continue
    formula, revision, *_ = original.relative_to(cellar).parts
    packages[formula] = cellar / formula / revision
licenses = resources / 'licenses'
licenses.mkdir()
for formula, prefix in sorted(packages.items()):
    destination = licenses / formula
    destination.mkdir()
    for candidate in prefix.rglob('*'):
        if candidate.is_file() and (candidate.name.upper().startswith(('LICENSE', 'COPYING', 'COPYRIGHT'))
                                   or 'licenses' in candidate.relative_to(prefix).parts):
            target = destination / candidate.relative_to(prefix)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(candidate, target)
    for recipe in (prefix / '.brew').glob('*.rb'):
        shutil.copy2(recipe, destination / recipe.name)
(licenses / 'runtime-packages.json').write_text(run('brew', 'info', '--json=v2', *sorted(packages)))
(licenses / 'README.txt').write_text(
    'Third-party libraries retain their own licenses. Formula recipes and package metadata\n'
    'identify upstream sources and exact versions. Libraries are dynamically linked and may\n'
    'be replaced with compatible modified builds. Original game and translation content\n'
    'retain their respective rights.\n')
for library in contents.rglob('*.dylib'):
    if library.is_symlink():
        continue
    run('codesign', '--force', '--sign', '-', library)
run('codesign', '--force', '--sign', '-', app)
run('codesign', '--verify', '--deep', '--strict', app)
print(run(macos / 'deadtime', '--audit'))
# Disable relocation so upgrades target the installed application consistently.
components = build / 'components.plist'
run('pkgbuild', '--analyze', '--root', stage, components)
with components.open('rb') as stream:
    entries = plistlib.load(stream)
for entry in entries:
    entry['BundleIsRelocatable'] = False
with components.open('wb') as stream:
    plistlib.dump(entries, stream)
architecture = run('uname', '-m').strip()
output.mkdir(parents=True, exist_ok=True)
package = output / f'DeadTimeStories-{version}-macos-{architecture}-installer.pkg'
print(run('pkgbuild', '--root', stage, '--component-plist', components,
          '--identifier', 'org.openlegacygame.deadtime', '--version', version,
          '--install-location', '/Applications', package))
print(package)
