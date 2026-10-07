#!/usr/bin/env python3
"""Download hash-pinned official Arduino tool archives into a new directory."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import tarfile
import urllib.request

ROOT = Path(__file__).resolve().parent
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--destination', type=Path, default=ROOT / 'tools')
p.add_argument('--archive-cache', type=Path, help='reuse existing archives only after full hash/size checks')
a = p.parse_args()
dest = a.destination.resolve()
if dest.exists():
    p.error('destination exists; choose a new directory')
dest.mkdir(parents=True)
lock = json.loads((ROOT / 'tools-lock.json').read_text())
for name, item in lock.items():
    archive = dest / item['archiveFileName']
    cached = a.archive_cache / item['archiveFileName'] if a.archive_cache else None
    if cached and cached.is_file():
        archive = cached
        print('Verifying cached ' + str(archive), flush=True)
    else:
        print('Downloading ' + item['url'], flush=True)
        with urllib.request.urlopen(item['url'], timeout=60) as response, archive.open('wb') as target:
            while chunk := response.read(1024 * 1024):
                target.write(chunk)
    if archive.stat().st_size != int(item['size']):
        raise SystemExit('archive size mismatch: ' + name)
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    if digest != item['checksum'].split(':', 1)[1]:
        raise SystemExit('archive SHA-256 mismatch: ' + name)
    target = dest / name
    target.mkdir()
    with tarfile.open(archive) as tf:
        members = tf.getmembers()
        top = {PurePosixPath(m.name).parts[0] for m in members if PurePosixPath(m.name).parts}
        if len(top) != 1:
            raise SystemExit('expected a single archive root: ' + name)
        selected = []
        for member in members:
            parts = PurePosixPath(member.name).parts
            if member.name.startswith('/') or '..' in parts:
                raise SystemExit('unsafe archive path')
            if len(parts) < 2:
                continue
            member.name = str(PurePosixPath(*parts[1:]))
            if member.islnk():
                link_parts = PurePosixPath(member.linkname).parts
                if member.linkname.startswith('/') or '..' in link_parts or not link_parts or link_parts[0] not in top:
                    raise SystemExit('unsafe archive hardlink')
                member.linkname = str(PurePosixPath(*link_parts[1:]))
            selected.append(member)
        # Strip the root consistently for hardlinks as well as member paths.
        # data filter rejects escaping symlinks, special devices and permissions.
        tf.extractall(target, members=selected, filter='data')
    print(name + ': SHA-256 verified and extracted', flush=True)
print('Dependencies ready: ' + str(dest))
