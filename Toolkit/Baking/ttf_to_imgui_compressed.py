#!/usr/bin/env python3
# ttf_to_imgui_compressed.py
#
# Minimal Python reimplementation of Dear ImGui's binary_to_compressed_c.cpp
# using the same stb_compress format, and emitting a `*_compressed_data` /
# `*_compressed_size` pair suitable for AddFontFromMemoryCompressedTTF().
#
# Only the "-u8" encoding is implemented (which is ImGui's current default).

from __future__ import annotations
import sys
from typing import List

# ---------------- Adler32 ---------------- #

def stb_adler32(adler32: int, buf: bytes) -> int:
    ADLER_MOD = 65521
    s1 = adler32 & 0xFFFF
    s2 = adler32 >> 16

    n = len(buf)
    idx = 0
    while n > 0:
        blocklen = min(n, 5552)
        for _ in range(blocklen):
            s1 += buf[idx]
            s2 += s1
            idx += 1
        s1 %= ADLER_MOD
        s2 %= ADLER_MOD
        n -= blocklen
    return (s2 << 16) + s1

# ---------------- Core compressor ---------------- #

STB_WINDOW = 0x40000       # 256k
STB_HASH_SIZE = 32768      # must be power of two

def stb_not_crap(best: int, dist: int) -> bool:
    return ((best > 2  and dist <= 0x00100) or
            (best > 5  and dist <= 0x04000) or
            (best > 7  and dist <= 0x80000))

def stb_matchlen(data: bytes, p1: int, p2: int, maxlen: int) -> int:
    i = 0
    end = maxlen
    while i < end and data[p1 + i] == data[p2 + i]:
        i += 1
    return i

def stb_compress(data: bytes) -> bytes:
    out = bytearray()

    def stb_out(v: int) -> None:
        out.append(v & 0xFF)

    def stb_out2(v: int) -> None:
        stb_out(v >> 8)
        stb_out(v)

    def stb_out3(v: int) -> None:
        stb_out(v >> 16)
        stb_out(v >> 8)
        stb_out(v)

    def stb_out4(v: int) -> None:
        stb_out(v >> 24)
        stb_out(v >> 16)
        stb_out(v >> 8)
        stb_out(v)

    def outliterals(start: int, numlit: int) -> None:
        nonlocal out
        while numlit > 65536:
            outliterals(start, 65536)
            start += 65536
            numlit -= 65536

        if numlit == 0:
            return
        elif numlit <= 32:
            stb_out(0x20 + numlit - 1)
        elif numlit <= 2048:
            stb_out2(0x800 + numlit - 1)
        else:  # numlit <= 65536
            stb_out3(0x70000 + numlit - 1)

        out.extend(data[start:start+numlit])

    # ---------------- header ---------------- #
    length = len(data)
    chash: List[int | None] = [None] * STB_HASH_SIZE
    mask = STB_HASH_SIZE - 1

    stb_out(0x57)
    stb_out(0xBC)
    stb_out2(0)            # 16-bit zero
    stb_out4(0)            # high 32 bits of 64-bit length
    stb_out4(length)       # low 32 bits of length
    stb_out4(STB_WINDOW)

    running_adler = 1

    start = 0
    q = start
    end = length
    literals = 0
    lit_start = start

    def SCRAMBLE(h: int) -> int:
        return (h + (h >> 16)) & mask

    while q < start + length and q + 12 < end:
        if q + 65536 > end:
            match_max = end - q
        else:
            match_max = 65536

        best = 2
        dist = 0

        # hashes (translated macros)
        h = (data[q+0] << 14) + (data[q+1] << 7) + data[q+2]
        h1 = SCRAMBLE(h)
        t_index = chash[h1]
        if t_index is not None:
            if dist != q - t_index:
                m = stb_matchlen(data, t_index, q, match_max)
                if m > best and stb_not_crap(m, q - t_index):
                    best = m
                    dist = q - t_index

        h = ((h << 14) + (h >> 18) + (data[q+3] << 7) + data[q+4]) & 0xFFFFFFFF
        h2 = SCRAMBLE(h)
        h = ((h << 14) + (h >> 18) + (data[q+5] << 7) + data[q+6]) & 0xFFFFFFFF
        t_index = chash[h2]
        if t_index is not None:
            # p==1 in original macro => skip dist!=q-t check
            m = stb_matchlen(data, t_index, q, match_max)
            if m > best and stb_not_crap(m, q - t_index):
                best = m
                dist = q - t_index

        h3 = SCRAMBLE(((h << 14) + (h >> 18) + (data[q+7] << 7) + data[q+8]) & 0xFFFFFFFF)
        h = (((h << 14) + (h >> 18) + (data[q+9] << 7) + data[q+10]) & 0xFFFFFFFF)
        t_index = chash[h3]
        if t_index is not None:
            m = stb_matchlen(data, t_index, q, match_max)
            if m > best and stb_not_crap(m, q - t_index):
                best = m
                dist = q - t_index

        h4 = SCRAMBLE(((h << 14) + (h >> 18) + (data[q+11] << 7) + data[q+12]) & 0xFFFFFFFF)
        t_index = chash[h4]
        if t_index is not None:
            m = stb_matchlen(data, t_index, q, match_max)
            if m > best and stb_not_crap(m, q - t_index):
                best = m
                dist = q - t_index

        chash[h1] = chash[h2] = chash[h3] = chash[h4] = q

        if best < 3:
            q += 1
            continue

        # same cases as stb_compress_chunk
        if best > 2 and best <= 0x80 and dist <= 0x100:
            outliterals(lit_start, q - lit_start)
            lit_start = q = q + best
            stb_out(0x80 + best - 1)
            stb_out(dist - 1)
        elif best > 5 and best <= 0x100 and dist <= 0x4000:
            outliterals(lit_start, q - lit_start)
            lit_start = q = q + best
            stb_out2(0x4000 + dist - 1)
            stb_out(best - 1)
        elif best > 7 and best <= 0x100 and dist <= 0x80000:
            outliterals(lit_start, q - lit_start)
            lit_start = q = q + best
            stb_out3(0x180000 + dist - 1)
            stb_out(best - 1)
        elif best > 8 and best <= 0x10000 and dist <= 0x80000:
            outliterals(lit_start, q - lit_start)
            lit_start = q = q + best
            stb_out3(0x100000 + dist - 1)
            stb_out2(best - 1)
        elif best > 9 and dist <= 0x1000000:
            if best > 65536:
                best = 65536
            outliterals(lit_start, q - lit_start)
            lit_start = q = q + best
            if best <= 0x100:
                stb_out(0x06)
                stb_out3(dist - 1)
                stb_out(best - 1)
            else:
                stb_out(0x04)
                stb_out3(dist - 1)
                stb_out2(best - 1)
        else:
            q += 1

    # finish chunk
    if q - start < length:
        q = start + length

    literals = q - lit_start
    running_adler = stb_adler32(running_adler, data[start:q])

    # dump remaining literals
    if literals:
        outliterals(q - literals, literals)

    # end opcode + checksum
    stb_out2(0x05FA)
    stb_out4(running_adler)

    return bytes(out)

# ---------------- Emit as u8 C array ---------------- #

def emit_u8(symbol: str, compressed: bytes, static: bool = True) -> None:
    static_str = "static " if static else ""
    size = len(compressed)
    out = sys.stdout

    print(f"{static_str}const unsigned int {symbol}_compressed_size = {size};", file=out)
    print(f"{static_str}const unsigned char {symbol}_compressed_data[{size}] =", file=out)
    print("{", file=out)
    line = "    "
    for b in compressed:
        chunk = f"{b},"
        if len(line) + len(chunk) > 180:
            print(line, file=out)
            line = "    " + chunk
        else:
            line += chunk
    if line.strip():
        print(line, file=out)
    print("};", file=out)

def main(argv: list[str]) -> int:
    if len(argv) < 3:
        print(f"Usage: {argv[0]} <input.ttf> <SymbolName>", file=sys.stderr)
        return 1

    input_path = argv[1]
    symbol = argv[2]

    with open(input_path, "rb") as f:
        data = f.read()

    compressed = stb_compress(data)

    emit_u8(symbol, compressed, static=True)
    return 0

if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
