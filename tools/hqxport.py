#!/usr/bin/env python3
"""Derive the hqx rule tables from the original MMX assembly.

The hq2x/hq3x/hq4x filters in video/c_hqx.c are 256-entry rule tables: for each
neighbourhood pattern, an expression per output pixel. The assembly they come
from was removed in 5ff6d63d ("Remove MMX-related code"), so recover it with

    git show 5ff6d63d^:video/hq2x16.asm > hq2x16.asm

and point this at the result. With --check the generated table is compared
against what video/c_hqx.c already contains, which is how the hq4x port was
trusted: the same extraction reproduces the independently hand-written hq2x and
hq3x tables exactly, so it can be believed on hq4x.

    tools/hqxport.py hq2x16.asm --check      # 256/256 must match
    tools/hqxport.py hq4x16.asm              # print the C case table
"""
import argparse
import re
import sys

# Output row for each destination base register form the macros use.
ROWS = {'edi': 0, 'edi+ebx': 1, 'edi+ebx*2': 2, 'ecx': 2, 'ecx+ebx': 3}
# Variants that take the centre pixel implicitly in eax rather than as an
# operand; hq4x always passes it explicitly, hq2x/hq3x do not.
IMPLICIT_W5 = {'3', '4', '6', '7', '9', '10'}
# Which filter each pattern-bit stands for; matches c_hqx.c's pattern build.
FLAGS = ((1, 1), (2, 2), (3, 4), (4, 8), (6, 16), (7, 32), (8, 64), (9, 128))


def parse_pixel_macros(asm):
    """PIXELrc_xx -> (row, col, C expression)."""
    out, i = {}, 0
    while i < len(asm):
        m = re.match(r'%macro (PIXEL(\d)(\d)(?:_\w+)?) 0$', asm[i].strip())
        if not m:
            i += 1
            continue
        name, row, col = m.group(1), int(m.group(2)), int(m.group(3))
        body, i = [], i + 1
        while asm[i].strip() != '%endmacro':
            if asm[i].strip():
                body.append(asm[i].strip())
            i += 1
        text = ' '.join(body)
        dst = re.search(r'\[(edi|ecx)((?:\+ebx(?:\*2)?)?)((?:\+\d+)?)\]', text)
        if ROWS[dst.group(1) + dst.group(2)] != row:
            raise SystemExit('%s: destination row disagrees with its name' % name)
        if int((dst.group(3) or '+0')[1:]) != col * 2:
            raise SystemExit('%s: destination column disagrees with its name' % name)
        call = re.match(r'Interp(\d+) \[[^]]*\],(.*)$', text)
        if call:
            args = []
            for a in (a.strip() for a in call.group(2).split(',')):
                if a == 'eax':
                    args.append('w5')  # hq2x/hq3x keep the centre pixel here
                else:
                    n = re.fullmatch(r'\[w(\d)\]', a).group(1)
                    # c_hqx.c names the centre w5, everything else w[n].
                    args.append('w5' if n == '5' else 'w[%s]' % n)
            if call.group(1) in IMPLICIT_W5 and args[0] != 'w5':
                args.insert(0, 'w5')
            expr = 'interp%s(%s)' % (call.group(1), ', '.join(args))
        else:
            expr = 'w5'
        out[name] = (row, col, expr)
    return out


def parse_cases(asm):
    """pattern -> the ops the assembly runs for it."""
    start = next(i for i, l in enumerate(asm)
                 if re.search(r'jmp\s+\[FuncTable\+ecx\*4\]', l))
    cases, flags, ops = {}, [], []
    for line in asm[start + 1:]:
        s = line.strip()
        m = re.match(r'^\.\.@flag(\d+)$', s)
        if m:
            if ops:
                for f in flags:
                    cases[f] = list(ops)
                flags, ops = [], []
            flags.append(int(m.group(1)))
        elif s.startswith('PIXEL') or s.startswith('DiffOrNot'):
            ops.append(s)
        elif s == 'jmp .loopx_end':
            for f in flags:
                cases[f] = list(ops)
            flags, ops = [], []
    return cases


def emit(op, pix, names, cols, indent):
    if op.startswith('DiffOrNot'):
        parts = [p.strip() for p in op[len('DiffOrNot'):].split(',')]
        a, b, rest = parts[0], parts[1], parts[2:]
        half = len(rest) // 2
        lines = ['%sif (differs(w[%s], w[%s])) {' % (indent, a[1:], b[1:])]
        for q in rest[:half]:
            lines += emit(q, pix, names, cols, indent + '    ')
        lines.append('%s} else {' % indent)
        for q in rest[half:]:
            lines += emit(q, pix, names, cols, indent + '    ')
        lines.append('%s}' % indent)
        return lines
    row, col, expr = pix[op]
    return ['%s%s = %s;' % (indent, names[row * cols + col], expr)]


def c_case_table(pix, cases, names, cols, indent='        '):
    groups = {}
    for p in range(256):
        groups.setdefault(tuple(cases[p]), []).append(p)
    lines = []
    for body, pats in groups.items():
        if not body:
            continue
        lines += ['    case %d:' % p for p in pats]
        for op in body:
            lines += emit(op, pix, names, cols, indent)
        lines.append('        break;')
    return lines


def existing_table(path, first, last, names):
    """The rule table already in c_hqx.c, as pattern -> normalised lines."""
    src = open(path).read()
    sw = src[src.index(first):src.index(last)]
    sw = sw[sw.index('switch (pattern) {'):]
    out, cur, acc = {}, [], []
    for line in sw.splitlines():
        s = line.strip()
        m = re.match(r'case (\d+):$', s)
        if m:
            cur.append(int(m.group(1)))
        elif s == 'break;':
            for p in cur:
                out[p] = list(acc)
            cur, acc = [], []
        elif s and not s.startswith('case') and s not in ('switch (pattern) {', '}'):
            acc.append(s)
    return out


def normalise(lines):
    out = []
    for l in lines:
        l = re.sub(r'\s+', ' ', l).strip()
        if l and l not in ('{', '}'):
            out.append(l)
    return out


SHAPES = {
    2: (['p00', 'p01', 'p10', 'p11'], 'static void hq2x_quad', 'static void hq2x_pixel('),
    3: (['p00', 'p01', 'p02', 'p10', 'p11', 'p12', 'p20', 'p21', 'p22'],
        'static void hq3x_nine', 'static void hq3x_pixel('),
    4: (['out[%d]' % i for i in range(16)],
        'static void hq4x_sixteen', 'static void hq4x_pixel('),
}


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('asm', help='hq2x16.asm, hq3x16.asm or hq4x16.asm')
    ap.add_argument('--check', metavar='C_FILE', nargs='?', const='video/c_hqx.c',
                    help='compare against the table already in this file')
    args = ap.parse_args()

    scale = int(re.search(r'hq(\d)x', args.asm).group(1))
    names, first, last = SHAPES[scale]
    asm = open(args.asm).read().splitlines()
    pix, cases = parse_pixel_macros(asm), parse_cases(asm)
    if len(cases) != 256:
        raise SystemExit('extracted %d patterns, expected 256' % len(cases))

    if args.check:
        have = existing_table(args.check, first, last, names)
        bad = []
        for p in range(256):
            if p not in have:
                continue
            want = [l for op in cases[p] for l in emit(op, pix, names, scale, '')]
            if normalise(have[p]) != normalise(want):
                bad.append(p)
        print('hq%dx: %d patterns compared, %d mismatches' % (scale, len(have), len(bad)))
        if bad:
            p = bad[0]
            print('first mismatch, pattern %d:' % p)
            print('  in %s:' % args.check)
            for l in normalise(have[p]):
                print('    ' + l)
            print('  from %s:' % args.asm)
            for l in normalise([l for op in cases[p]
                                for l in emit(op, pix, names, scale, '')]):
                print('    ' + l)
        return 1 if bad else 0

    print('\n'.join(c_case_table(pix, cases, names, scale)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
