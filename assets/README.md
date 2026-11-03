# Game source inputs

`resources/` contains the original readable Lua, XML, layouts, models, shader
source, images and audio. `localization/` overlays Chinese strings and fonts.
These are the production inputs of the private study project. They are not
granted an open-source license; original game and translation rights remain
with their respective owners.

For a small path through the original implementation, start with
[`front_gate.loc`](resources/zones/jessie_home/front_gate.loc): the apple object
supplies its position, texture and relative script path. Its
[`apple.lua`](resources/zones/jessie_home/front_gate/apple/apple.lua) defines
the mouse callback, inventory change and object state. The runtime binds those
Lua calls in [`src/runtime.cpp`](../src/runtime.cpp); the image and hit testing
are implemented by [`src/player.cpp`](../src/player.cpp).

For hidden-object scenes, read
[`hob_plants.hob`](resources/zones/jessie_home/hob_plants.hob),
[`hobs/_config.xml`](resources/hobs/_config.xml), and
[`complete_round0.lua`](resources/zones/jessie_home/hob_plants/complete_round0.lua).
These define targets/positions/tool requirements, the shared configuration, and
the reward/story callback respectively. They run through the new engine's
[`hob.cpp`](../src/hob.cpp) and [`hob_feedback.cpp`](../src/hob_feedback.cpp).
Readable Lua supplies game behavior; the original native engine's APIs still
need their C++ implementations for that behavior to execute.

The initial import contains 6,600 files (including 526 `.lua` files), with
unaltered bytes/encodings. The original engine uses `.a` for JPEG alpha images;
all 2,397 such files are JPEG data, not compiled static libraries. Binary images,
audio and fonts are source media, while generated PAKs, executables and installers
remain outside Git.

[`windows/deadtime.ico`](windows/deadtime.ico) contains the seven original
32-bit images (16 through 256 pixels) from icon group 101 of the verified game
executable. The Chinese installer contains the same image bytes. The ICO only
rebuilds the icon directory around those images. CMake and NSIS embed it in the
new game, installer and uninstaller. This separate artwork is outside the
6,600-file PAK import.

`provenance.json` records the verified full-edition archive hashes, each imported
file's initial SHA-256/size and four explicit omissions: an unused ImageReady
temporary image, a Maya preview cache, artist working-directory notes, and an
empty compiled gettext catalog. The editable `.po` is retained. The manifest
documents the import baseline; it does not prohibit subsequent source edits or
claim permission to redistribute the original work.

Run the engine with `--assets assets` to read these directories directly. Edits
to existing files are visible on the next read; new/deleted paths require a
restart. Directory and ZIP mounts share case-insensitive lookup and localization
overlay order. Links and ambiguous case-colliding names are rejected.

A normal CMake build packs both layers into `build/data/*.pak` and the Windows
release pipeline uses those generated archives. No original installation is
needed after cloning. For an edit/rebuild test, change a Lua or layout file,
rebuild, and run the output; adding and removing files also invalidates the pack
target. Repacking preserves file bytes, sorts names and fixes modification timestamps. ZIP metadata such as source
access times can vary; reproducibility checks compare the resource names and
file bytes rather than requiring identical ZIP headers.

To reproduce the initial import separately, use Python 3.11+:

```sh
python3 tools/import-game-data.py /path/to/verified-full-install --output .local/import-check
```

The importer accepts only the two audited archive hashes. It validates names,
entry types, CRCs, size limits and input formats, and publishes the destination
only after validating both layers. It does not run the downloaded executable.
