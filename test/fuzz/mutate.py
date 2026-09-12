#!/usr/bin/env python3
"""Byte-level mutation of one file, deterministic in the seed number.

    mutate.py IN OUT SEED

A handful of edits per call: flipped bits, bytes set to boundary values,
short runs overwritten, a chunk cut or duplicated. Enough to reach the length
fields and magic numbers the loaders read, which is where a parser that trusts
its input breaks."""
import random
import sys


def mutate(data, seed):
    rnd = random.Random(seed)
    data = bytearray(data)
    for _ in range(rnd.randint(1, 8)):
        if not data:
            break
        choice = rnd.random()
        i = rnd.randrange(len(data))
        if choice < 0.4:
            data[i] ^= 1 << rnd.randrange(8)
        elif choice < 0.7:
            data[i] = rnd.choice((0, 1, 0x7F, 0x80, 0xFE, 0xFF))
        elif choice < 0.85:
            n = min(len(data) - i, rnd.randint(2, 64))
            data[i:i + n] = bytes(rnd.choice((0, 0xFF, rnd.randrange(256))) for _ in range(n))
        elif choice < 0.93:
            n = min(len(data) - i, rnd.randint(1, 4096))
            del data[i:i + n]
        else:
            n = min(len(data) - i, rnd.randint(1, 4096))
            data[i:i] = data[i:i + n]
    return bytes(data)


if __name__ == '__main__':
    src, dst, seed = sys.argv[1], sys.argv[2], int(sys.argv[3])
    with open(src, 'rb') as f:
        blob = f.read()
    with open(dst, 'wb') as f:
        f.write(mutate(blob, seed))
