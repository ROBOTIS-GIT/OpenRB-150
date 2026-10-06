#!/usr/bin/env python3
"""Strict, read-only image verification; no normalization, patching or hardware access."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

PAYLOAD_SHA = '401ef170d729494e4f154fd38a4f689e0fcde199363947f282193ec7881ff9d6'
PHYSICAL_SHA = 'a046d2e126b32f35797354e3025c76f832ff4bedcc193f06387ed35f39ee9da7'
sha = lambda b: hashlib.sha256(b).hexdigest()
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('candidate', type=Path)
p.add_argument('--reference', type=Path, default=Path(__file__).resolve().parent.parent / 'openrb-150_bootloader.bin')
p.add_argument('--physical', type=Path, help='optional independently preserved physical 8 KiB readback')
p.add_argument('--report', type=Path)
a = p.parse_args()
candidate, reference = a.candidate.read_bytes(), a.reference.read_bytes()
if len(reference) != 7328 or sha(reference) != PAYLOAD_SHA:
    raise SystemExit('reference is not the verified official payload')
if candidate != reference:
    raise SystemExit('candidate does not exactly equal all 7328 reference bytes')
padded = candidate + b'\xff' * (8192 - len(candidate))
if sha(padded) != PHYSICAL_SHA:
    raise SystemExit('padded candidate does not equal the recorded physical digest')
if a.physical and a.physical.read_bytes() != padded:
    raise SystemExit('physical readback does not exactly equal padded candidate')
stack, reset = struct.unpack_from('<II', candidate)
if (stack, reset) != (0x20007c00, 0x649):
    raise SystemExit('unexpected vectors')
report = dict(payload_bytes=len(candidate), payload_sha256=sha(candidate), padded_bytes=len(padded),
              padded_sha256=sha(padded), exact_payload_equal=True,
              physical_file_compared=bool(a.physical), initial_stack=hex(stack), reset_vector=hex(reset),
              application_base='0x2000', usb_vid='0x2f5d', usb_pid='0x2202')
text = json.dumps(report, indent=2) + '\n'
if a.report:
    a.report.write_text(text)
print(text, end='')
