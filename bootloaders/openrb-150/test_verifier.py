"""Offline integrity tests using the distributed image; no hardware required."""
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent

class VerificationTests(unittest.TestCase):
    def test_accepts_only_complete_verified_image(self):
        original = (ROOT.parent / 'openrb-150_bootloader.bin').read_bytes()
        with tempfile.TemporaryDirectory() as directory:
            directory = Path(directory)
            candidate, physical = directory / 'candidate.bin', directory / 'physical.bin'
            physical.write_bytes(original + b'\xff' * 864)
            for label, data, accepted in [('valid', original, True),
                                          ('bitflip', original[:256] + bytes([original[256] ^ 1]) + original[257:], False),
                                          ('truncated', original[:-1], False),
                                          ('appended', original + b'\xff', False)]:
                with self.subTest(label=label):
                    candidate.write_bytes(data)
                    r = subprocess.run([sys.executable, str(ROOT / 'verify.py'), str(candidate), '--physical', str(physical)], capture_output=True)
                    self.assertEqual(r.returncode == 0, accepted, r.stderr.decode())
            candidate.write_bytes(original)
            physical.write_bytes(original + b'\x00' * 864)
            r = subprocess.run([sys.executable, str(ROOT / 'verify.py'), str(candidate), '--physical', str(physical)], capture_output=True)
            self.assertNotEqual(r.returncode, 0)
            reference = directory / 'wrong-reference.bin'
            reference.write_bytes(original[:-1])
            r = subprocess.run([sys.executable, str(ROOT / 'verify.py'), str(candidate), '--reference', str(reference)], capture_output=True)
            self.assertNotEqual(r.returncode, 0)

if __name__ == '__main__':
    unittest.main()
