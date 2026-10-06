#!/usr/bin/env python3
"""Build in an isolated output folder, then verify against the distributed image."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--tools', type=Path, default=ROOT / 'tools', help='prepare_tools.py destination')
p.add_argument('--gcc-root', type=Path)
p.add_argument('--cmsis-root', type=Path, help='directory containing CMSIS/Include')
p.add_argument('--atmel-root', type=Path, help='directory containing CMSIS/Device/ATMEL')
p.add_argument('--output', type=Path, default=ROOT / 'out')
a = p.parse_args()
gcc = (a.gcc_root or a.tools / 'arm-none-eabi-gcc').resolve()
cmsis = (a.cmsis_root or a.tools / 'CMSIS').resolve()
atmel = (a.atmel_root or a.tools / 'CMSIS-Atmel').resolve()
for f in [gcc / 'bin/arm-none-eabi-gcc', cmsis / 'CMSIS/Include/core_cm0plus.h', atmel / 'CMSIS/Device/ATMEL/sam.h']:
    if not f.is_file():
        p.error(f'missing dependency: {f}')
version = subprocess.check_output([str(gcc / 'bin/arm-none-eabi-gcc'), '--version'], text=True)
if '7.2.1 20170904' not in version or '7-2017-q4-major' not in version:
    p.error('requires GNU Arm Embedded 7-2017-q4-major, GCC 7.2.1 20170904')
out = a.output.resolve()
if out.exists():
    p.error(f'output already exists: {out}; choose a fresh directory')
out.mkdir(parents=True)
shutil.copytree(ROOT / 'src', out / 'src')
cmd = ['make', '-C', str(out / 'src'), 'ARM_GCC_PATH=' + str(gcc / 'bin/arm-none-eabi-'),
       f'INCLUDES=-I"{cmsis / "CMSIS/Include"}" -I"{atmel / "CMSIS/Device/ATMEL"}"']
env = os.environ.copy()
for key in ['MAKEFLAGS', 'MFLAGS', 'GCC_EXEC_PREFIX', 'COMPILER_PATH', 'LIBRARY_PATH', 'CPATH', 'C_INCLUDE_PATH',
            'DEBUG', 'SECURE_BY_DEFAULT', 'BOARD_ID', 'NAME', 'SAM_BA_INTERFACES', 'AVRSTUDIO_EXE_PATH', 'OS']:
    env.pop(key, None)
env['LC_ALL'] = 'C'
r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env)
(out / 'build.log').write_bytes(r.stdout)
(out / 'compiler.txt').write_text(version)
if r.returncode:
    sys.stderr.buffer.write(r.stdout)
    sys.exit(r.returncode)
subprocess.run([sys.executable, str(ROOT / 'verify.py'), str(out / 'src/openrb-150_bootloader.bin'),
                '--report', str(out / 'verification.json')], check=True)
elf = out / 'src/build/openrb-150_bootloader.elf'
(out / 'elf-disassembly.txt').write_bytes(subprocess.check_output([str(gcc / 'bin/arm-none-eabi-objdump'), '-d', str(elf)]))
for name, binary in [('reference', ROOT.parent / 'openrb-150_bootloader.bin'), ('built', out / 'src/openrb-150_bootloader.bin')]:
    data = subprocess.check_output([str(gcc / 'bin/arm-none-eabi-objdump'), '-D', '-b', 'binary', '-m', 'arm', '-M', 'force-thumb', str(binary)])
    # Only the filename header differs for byte-identical inputs.
    normalized = data.split(b'Disassembly of section .data:', 1)[1]
    (out / (name + '-raw-disassembly.txt')).write_bytes(normalized)
if (out / 'reference-raw-disassembly.txt').read_bytes() != (out / 'built-raw-disassembly.txt').read_bytes():
    raise SystemExit('raw disassembly differs')
print(f'Build, strict binary comparison and raw disassembly comparison passed: {out}')
