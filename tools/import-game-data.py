#!/usr/bin/env python3
"""Import the verified full edition's production inputs; never execute its code.

Only needed to reproduce the initial import. Normal clone/build/edit/package uses
the committed assets tree, without the downloaded installation or this script.
"""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import stat
import tempfile
import zipfile

ARCHIVES = {
    "resources": "f0de9ae08f681e680a22f2529faba08546d01c0fe5a07c5904784df1445a912b",
    "localization": "040365d9c2489fe7f74d91794a5e3ba37266d2071d7c9bff6a23d68d02d4f58b",
}
OMIT = {
    "resources/hud/common/t3336t26698.tmp": "Unused ImageReady temporary image",
    "resources/particles_ready/souls/soul_cellar/.mayaSwatches/soul_cellar_00.tga.swatch": "Maya preview cache",
    "resources/particles_ready/souls/struct.txt": "Unused artist working-directory notes",
    "localization/deadtime.mo": "Empty compiled gettext catalog (zero translations); retain editable deadtime.po",
}
EXTENSIONS = set(".a .anim .animset .csc .dae .font .fx .game .gui .hob .ifl .jpg .loc .lua .mat .ogg .psysl .scene .tga .tgui .txt .xml .zone .cfg .po .ttf".split())


def digest(data):
    return hashlib.sha256(data).hexdigest()


def import_data(source, destination):
    if destination.exists():
        raise ValueError(f"Destination already exists: {destination}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    manifest = {"format": 1, "archives": {}, "files": [], "omitted": []}
    # Publish only after both archives and all entries have passed validation.
    with tempfile.TemporaryDirectory(prefix=".game-import-", dir=destination.parent) as tmp:
        stage = Path(tmp) / "assets"
        stage.mkdir()
        for layer, expected in ARCHIVES.items():
            archive = source / (layer + ".pak")
            with archive.open("rb") as stream:
                actual = hashlib.file_digest(stream, "sha256").hexdigest()
            if actual != expected:
                raise ValueError(f"Unverified archive: {archive}")
            manifest["archives"][layer + ".pak"] = actual
            names = set()
            with zipfile.ZipFile(archive) as z:
                for info in sorted(z.infolist(), key=lambda i: i.filename):
                    name = info.filename.rstrip("/")
                    path = PurePosixPath(name)
                    if (not name or path.is_absolute() or "\\" in name
                            or any(p in ("", ".", "..") or p.endswith((".", " "))
                                   or any(c in p for c in ':;\r\n\x00"$') for p in name.split("/"))):
                        raise ValueError(f"Unsafe archive path: {info.filename!r}")
                    mode = info.external_attr >> 16
                    if stat.S_IFMT(mode) not in (0, stat.S_IFREG, stat.S_IFDIR):
                        raise ValueError(f"Non-regular archive entry: {name}")
                    if name.casefold() in names:
                        raise ValueError(f"Duplicate archive path: {name}")
                    names.add(name.casefold())
                    if info.is_dir():
                        continue
                    if info.file_size > 128 * 1024 * 1024:
                        raise ValueError(f"Oversized entry: {name}")
                    data = z.read(info)  # Also checks the ZIP CRC.
                    key = layer + "/" + name
                    record = {"path": key, "bytes": len(data), "sha256": digest(data)}
                    if key in OMIT:
                        manifest["omitted"].append({**record, "reason": OMIT[key]})
                        continue
                    if path.suffix.lower() not in EXTENSIONS or data.startswith((b"MZ", b"\x7fELF", b"!<arch>")):
                        raise ValueError(f"Unexpected production input: {name}")
                    if path.suffix.lower() == ".a" and not data.startswith(b"\xff\xd8"):
                        raise ValueError(f"Alpha image is not JPEG: {name}")
                    target = stage / layer / path
                    target.parent.mkdir(parents=True, exist_ok=True)
                    target.write_bytes(data)
                    manifest["files"].append(record)
        if {r["path"] for r in manifest["omitted"]} != OMIT.keys():
            raise ValueError("Expected omissions missing from verified input")
        for field in ("files", "omitted"):
            manifest[field].sort(key=lambda r: r["path"])
        (stage / "provenance.json").write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        stage.rename(destination)
    print(f"Imported {len(manifest['files'])} production files; omitted {len(manifest['omitted'])} unused artifacts.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("installation", type=Path)
    parser.add_argument("--output", type=Path, default=Path("assets"))
    args = parser.parse_args()
    import_data(args.installation, args.output)
