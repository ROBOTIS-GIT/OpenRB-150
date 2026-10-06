# OpenRB-150 bootloader

Bootloader source and build tools for OpenRB-150 (ATSAMD21G18A). The build must
match the existing [`../openrb-150_bootloader.bin`](../openrb-150_bootloader.bin)
exactly: 7,328 bytes, SHA-256
`401ef170d729494e4f154fd38a4f689e0fcde199363947f282193ec7881ff9d6`.

## Build

Requires Linux x86_64, Python 3.12+ and GNU make.

```sh
cd bootloaders/openrb-150
python3 prepare_tools.py
python3 build.py
python3 -m unittest discover -s . -p 'test_*.py'
```

`prepare_tools.py` downloads the official Arduino tool archives and checks the
sizes and SHA-256 hashes in `tools-lock.json`. It requires a new destination;
`--destination DIR` chooses another, and `--archive-cache DIR` reuses archives
only after the same checks. The pinned dependencies are GNU Arm Embedded
7-2017q4, CMSIS 4.5.0 and CMSIS-Atmel 1.2.0.

`build.py` builds in a fresh `out` directory and checks the complete binary and
raw disassembly against the distributed image. `--output DIR` chooses another
fresh directory. Build outputs include ELF, BIN, HEX, map, symbols and logs;
these are generated locally and are not tracked. Existing tool installations
can be supplied with `--gcc-root`, `--cmsis-root` and `--atmel-root`. CMSIS roots
contain `CMSIS/Include` and `CMSIS/Device/ATMEL`, respectively.

## Source and board configuration

The source baseline is ArduinoCore-samd
[`ed21e120116845636abb51fc9a3a4abcbb391d22`](https://github.com/arduino/ArduinoCore-samd/tree/ed21e120116845636abb51fc9a3a4abcbb391d22/bootloaders/zero).
OpenRB-150 configuration uses USB product `OpenRB`, VID/PID `2f5d:2202`,
SERCOM0 TX PA10/PAD2 and RX PA11/PAD3 at 115200 baud, and the PB08 LED.
Other Arduino board configurations and unused drivers are omitted.

The image uses GCC 7.2.1, `-O1` without LTO, and a fixed version timestamp
`Feb 14 2022 00:27:50`. Preserve the LED's two volatile reads and the fixed
timestamp for binary compatibility. Initial SP is `0x20007c00`, reset vector
`0x649`, application base `0x2000`, and double-tap RAM address `0x20007ffc`.
Binary equality is the compatibility criterion; it does not establish the
original source text, author intent, or a tested DYNAMIXEL bootloading protocol.

## Upstream patch

[`upstream.patch`](upstream.patch) records the changes to the retained target
source files from ArduinoCore-samd commit
`ed21e120116845636abb51fc9a3a4abcbb391d22`, directory `bootloaders/zero`.
The board baseline is `board_definitions_arduino_mkrzero.h`, renamed to
`board_definitions_openrb150.h`. The patch covers the six changed files,
including the rename and dated modification notices. The other 15 retained
source files are unchanged. Unused upstream board configurations and drivers
are omitted from the selected source set. Python tools and dependency notices
are maintained separately.

To verify the patch, run the following from this directory with a local
ArduinoCore-samd checkout containing the pinned commit:

```sh
upstream_checkout=/path/to/ArduinoCore-samd
upstream_revision=ed21e120116845636abb51fc9a3a4abcbb391d22
patch_work=$(mktemp -d)
for file in src/*; do
  name=${file##*/}
  if [ "$name" = board_definitions_openrb150.h ]; then
    name=board_definitions_arduino_mkrzero.h
  fi
  git -C "$upstream_checkout" show "$upstream_revision:bootloaders/zero/$name" > "$patch_work/$name" || exit 1
done
git -C "$patch_work" apply --check "$PWD/upstream.patch"
git -C "$patch_work" apply "$PWD/upstream.patch"
diff -ru src "$patch_work"
```

To regenerate the patch, first repeat the baseline extraction through `done`,
skipping the apply commands. Then initialize its index and copy the current
target files over it:

```sh
git -C "$patch_work" init -q
git -C "$patch_work" add .
rm "$patch_work/board_definitions_arduino_mkrzero.h"
cp src/* "$patch_work/"
git -C "$patch_work" add -N board_definitions_openrb150.h
git -C "$patch_work" diff --no-ext-diff --no-textconv --find-renames --full-index > upstream.patch
```

After regeneration, verify application against a fresh baseline and run
`build.py` to confirm the complete distributed binary still matches.

## Licensing and distribution

The imported bootloader source and its modifications retain LGPL-2.1-or-later
notices and the accompanying [LICENSE](LICENSE). Original Arduino and Atmel
copyright notices are preserved, and changed upstream files carry dated change
notices. The repository's Apache-2.0 license does not replace those terms;
independent Python build/test tools and this documentation follow the repository
license. No upstream copyright ownership is claimed.

[THIRD_PARTY_NOTICES.txt](THIRD_PARTY_NOTICES.txt) and
[LICENSE.newlib](LICENSE.newlib) accompany binary distributions. They preserve
notices for the pinned CMSIS, Atmel device headers and linked runtime components.
The Atmel device-header terms restrict use to Atmel microcontroller products;
OpenRB-150 uses ATSAMD21G18A. The GCC runtime exception does not change the
bootloader's LGPL obligations. Compiler executables are downloaded dependencies,
not part of this source package.

When distributing a binary, accompany it with the applicable notices, license
texts and corresponding source/build material, or another compliant source
access/offer arrangement under LGPL sections 4 and 6. Preserve recipients'
rights to modify and rebuild/relink the covered work. For a release, retain the
pinned associated header sources alongside the build material; the dependency
archives contain those headers with their original notices. Do not describe
the whole bootloader as Apache-only or remove upstream attribution.

These tools do not flash hardware or modify fuses. Verification of a preserved
8 KiB image is optional: `python3 verify.py out/src/openrb-150_bootloader.bin
--physical /path/to/bootloader-8k.bin`. The expected padding is 864 bytes of `FF`.
