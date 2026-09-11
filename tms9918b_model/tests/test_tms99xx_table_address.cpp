/*
 * TMS9918B reference model - TMS99xx table address check against openMSX
 *
 * Copyright 2026 Saverio Russo
 * SPDX-License-Identifier: Apache-2.0
 */
// Exhaustive check of TMS99xxTableAddress against a literal transcription of
// openMSX (VDP.cc window masks + CharacterConverter index construction), and
// a count of where the current GearSF7000 formulas disagree.
// g++ -std=c++17 -O2 -I../src test_tms99xx_table_address.cpp -o test && ./test
#include "TMS99xxTableAddress.h"
#include <cstdio>
#include <cstdint>
#include <initializer_list>
namespace A = TMS99xxTableAddress;
using u32 = std::uint32_t;
static constexpr u32 SIZE = 0x3FFF;

// --- openMSX, literal -------------------------------------------------------
static u32 readNP(u32 baseMask, u32 index) { return baseMask & SIZE & index; }
static u32 area(u32 baseMask, u32 indexMask, u32 index) { return baseMask & SIZE & (indexMask | index); }

static u32 omName(int mode, u32 r2, int line, int col) {
    u32 base = (r2 << 10) | 0x3FF;
    if (mode & 1) return readNP(base, ((line / 8) * 40 + col + 0xC00) | (~0u << 12));
    return (area(base, ~0u << 10, (line / 8) * 32) + col) & SIZE;
}
static u32 omPattern(int mode, u32 r3, u32 r4, int line, u32 name, bool leakChip) {
    u32 base = (r4 << 11) | 0x7FF;
    switch (mode) {
    case 0: case 1: return (area(base, ~0u << 11, 0) + (line & 7) + name * 8) & SIZE;      // Graphic1, Text1
    case 4: return readNP(base, (name * 8) | ((~0u << 11) | ((line / 4) & 7)));             // Multi
    case 3: return readNP(base, ((~0u << 13) | (line & 7)) | ((((line & 0xC0) << 2) | name) * 8)); // Text1Q
    case 6: return readNP(base, ((((line * 4) & ~0xFFu) | name) * 8) | ((~0u << 13) | ((line / 4) & 7))); // MultiQ
    case 2: {
        if (leakChip) base = (r4 << 11) | ((r3 & 0x1F) << 6) | ~(~0u << 6);
        u32 quarter8 = ((((line / 8) * 32) & ~0xFFu) * 8);
        return readNP(base, (~0u << 13) | quarter8 | (line & 7) | (name * 8));
    }
    }
    return 0;
}
static u32 omColour(int mode, u32 r3, int line, u32 name) {
    u32 base = (r3 << 6) | 0x3F;
    if (mode == 0) return (area(base, ~0u << 6, 0) + (name >> 3)) & SIZE;
    u32 quarter8 = ((((line / 8) * 32) & ~0xFFu) * 8);
    return readNP(base, (~0u << 13) | quarter8 | (line & 7) | (name * 8));
}

// --- GearSF7000 as it is today (IssueTimedVideoFetch) -----------------------
static u32 gsName(int mode, u32 r2, int line, int col) {
    const int tileY = line >> 3; int columns = (mode & 1) ? 40 : 32;
    if (mode == 3) return (((r2 << 10) & 0x3000) + (((tileY * columns) + col + 0xC00) & 0x0FFF)) & SIZE;
    return ((r2 << 10) + tileY * columns + col) & SIZE;
}
static u32 gsPattern(int mode, u32 r4, int line, u32 name) {
    const int tileY = line >> 3, row = line & 7;
    if (mode == 2) {
        int nameInRegion = name + ((tileY & 0x18) << 5);
        return (((r4 << 11) & 0x2000) + ((nameInRegion & (((r4 & 3) << 8) | 0xFF)) << 3) + row) & SIZE;
    }
    if (mode == 3 || mode == 6) {
        int quarter = mode == 3 ? ((line & 0xC0) << 2) : ((line * 4) & ~0xFF);
        int rowInCell = mode == 3 ? row : ((line >> 2) & 7);
        return (((r4 << 11) & 0x2000) + (((quarter | name)) << 3) + rowInCell) & SIZE;
    }
    if (mode == 4) return ((r4 << 11) + (name << 3) + ((line >> 2) & 7)) & SIZE;
    return ((r4 << 11) + (name << 3) + row) & SIZE;
}

int main() {
    long long fail = 0, diffName3 = 0, totName3 = 0;
    long long diffPat[8] = {0}, totPat[8] = {0};
    // names
    for (int mode : {0, 1, 2, 3, 4, 6})
        for (u32 r2 = 0; r2 < 16; ++r2)
            for (int line = 0; line < 192; line += 8)
                for (int col = 0; col < ((mode & 1) ? 40 : 32); ++col) {
                    u32 ref = omName(mode, r2, line, col);
                    if (A::Name(mode, r2, line, col) != ref) ++fail;
                    if (mode == 3) { ++totName3; if (gsName(mode, r2, line, col) != ref) ++diffName3; }
                }
    // patterns (TMS99x8A) and colours
    for (int mode : {0, 1, 2, 3, 4, 6})
        for (u32 r4 = 0; r4 < 8; ++r4)
            for (u32 r3 = 0; r3 < 256; r3 += (mode == 2 ? 1 : 255))
                for (int line = 0; line < 192; ++line)
                    for (u32 name = 0; name < 256; ++name) {
                        u32 ref = omPattern(mode, r3, r4, line, name, true);
                        if (A::Pattern(mode, r3, r4, line, name, A::Chip::TMS99x8A) != ref) ++fail;
                        if (mode == 2 && A::Pattern(mode, r3, r4, line, name, A::Chip::TMS91x8) != omPattern(mode, r3, r4, line, name, false)) ++fail;
                        ++totPat[mode]; if (gsPattern(mode, r4, line, name) != ref) ++diffPat[mode];
                    }
    for (int mode : {0, 2})
        for (u32 r3 = 0; r3 < 256; ++r3)
            for (int line = 0; line < 192; ++line)
                for (u32 name = 0; name < 256; ++name)
                    if (A::Colour(mode, r3, line, name) != omColour(mode, r3, line, name)) ++fail;

    std::printf("TMS99xxTableAddress vs openMSX: %lld differences\n", fail);
    std::printf("\nPrevious GearSF7000 formulas vs openMSX (TMS9918A):\n");
    std::printf("  names, mode 3 (Text1Q):   %5.1f%% of cases differ\n", 100.0 * diffName3 / totName3);
    const char* names[8] = {"G1", "Text1", "G2", "Text1Q", "Multicolor", "", "MultiQ", ""};
    for (int m : {0, 1, 2, 3, 4, 6})
        std::printf("  patterns, mode %d %-10s %5.1f%% of cases differ\n", m, names[m], 100.0 * diffPat[m] / totPat[m]);
    return fail ? 1 : 0;
}
