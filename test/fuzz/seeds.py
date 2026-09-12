#!/usr/bin/env python3
"""Synthesise seed files for the loaders that have no easy way to record one.

    seeds.py OUTDIR ROM

Each seed is well-formed enough to get past the loader's first checks, so a
mutation of it lands inside the parser rather than on the magic number."""
import struct
import sys
import zipfile
import zlib


def zmv(rom_crc):
    """An empty movie that starts from power-on, current version unknown so
    the loader takes its version-mismatch branch and reads a state."""
    hdr = b'ZMV' + struct.pack('<H', 0)          # version: never matches
    hdr += struct.pack('<IIIII', rom_crc, 0, 0, 0, 0)
    hdr += b'\x3c'                               # average fps
    hdr += struct.pack('<IHH', 0, 0, 4)          # combos, chapters, author_len
    hdr += b'\x00\x00\x00'                       # zst_size (3 bytes)
    hdr += struct.pack('<H', 0x8000)             # initial_input: pad 1
    hdr += b'\xc0'                               # flag: start method clear_all
    state = zlib.compress(bytes(4096))
    body = struct.pack('<I', len(state))[:3] + state
    tail = struct.pack('<H', 0) + b'fuzz'        # external chapters, author
    return hdr + body + tail


def cht():
    """Three cheats, in the 28-byte entry format the GUI writes."""
    out = bytearray()
    for i in range(3):
        e = bytearray(28)
        e[0] = 0x80                              # enabled
        e[1] = 0x99                              # value
        e[2:4] = struct.pack('<H', 0x0100 + i)   # address
        e[4] = 0x7E                              # bank
        e[6:8] = b'\xfe\xfc'                     # format marker
        e[8:20] = b'CHEAT%d      ' % i
        out += e
    return bytes(out)


def cmb():
    hdr = bytearray(23)
    hdr[:5] = b'ZCOMB'
    hdr[22] = 2                                  # entries
    e = bytearray(66)
    e[:20] = b'COMBO               '
    e[62:64] = struct.pack('<H', 0x0041)
    return bytes(hdr) + bytes(e) * 2


def ips():
    out = b'PATCH'
    out += struct.pack('>I', 0x8000)[1:] + struct.pack('>H', 4) + b'\xea\xea\xea\xea'
    out += struct.pack('>I', 0x8100)[1:] + struct.pack('>H', 0) + struct.pack('>H', 16) + b'\x00'
    return out + b'EOF'


def main():
    out, rom = sys.argv[1], sys.argv[2]
    with open(rom, 'rb') as f:
        data = f.read()
    crc = zlib.crc32(data) & 0xFFFFFFFF
    open(out + '/seed.zmv', 'wb').write(zmv(crc))
    open(out + '/seed.cht', 'wb').write(cht())
    open(out + '/seed.cmb', 'wb').write(cmb())
    open(out + '/seed.ips', 'wb').write(ips())
    with zipfile.ZipFile(out + '/seed.zip', 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('game.smc', data[:65536 * 4])
    with zipfile.ZipFile(out + '/stored.zip', 'w', zipfile.ZIP_STORED) as z:
        z.writestr('game.smc', data[:65536 * 4])


if __name__ == '__main__':
    main()
