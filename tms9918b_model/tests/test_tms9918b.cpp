/*
 * TMS9918B reference model - TMS9918B model checks
 *
 * Copyright 2026 Saverio Russo
 * SPDX-License-Identifier: Apache-2.0
 *
 * g++ -std=c++17 -O2 -I../src -Isrc/tms99xx test_tms9918b.cpp \
 *     ../src/TMS9918BCalendar.cpp ../src/TMS9918BAnalysis.cpp \
 *     ../src/tms99xx/TMS99xxVramSlotSchedule.cpp ../src/tms99xx/TMS99xxVramSequencer.cpp
 */

#include "TMS9918BAddress.h"
#include "TMS9918BAnalysis.h"
#include "TMS9918BCalendar.h"
#include "TMS9918BModes.h"
#include "TMS9918BTiming.h"
#include "TMS99xxSlotGrid.h"
#include "TMS99xxVramSequencer.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <map>

using namespace TMS9918B;

static int g_failures = 0;
#define CHECK(cond, ...) do { if (!(cond)) { std::printf("FAIL %s:%d: ", __FILE__, __LINE__); std::printf(__VA_ARGS__); std::printf("\n"); ++g_failures; } } while (0)

static const Cycle* Find(const Calendar& c, Kind kind, std::uint8_t index)
{
    for (const Cycle& cy : c.cycles)
        if (cy.kind == kind && cy.index == index)
            return &cy;
    return nullptr;
}

// ----- DRAM data sheet compliance --------------------------------------------------
static void CheckDram()
{
    for (const DramSpec* d : {&MB8118_12, &MCM4517_12})
    {
        for (std::uint8_t n = 1; n <= 4; ++n)
            for (const Check& k : VerifyRead(*d, n))
                CHECK(k.Ok(), "%s read x%u: %s needs %.1f ns, has %.1f ns", d->name, n, k.parameter.c_str(), k.required, k.actual);
        for (const Check& k : VerifyWrite(*d))
            CHECK(k.Ok(), "%s write: %s needs %.1f ns, has %.1f ns", d->name, k.parameter.c_str(), k.required, k.actual);
    }
    // 150 ns parts are outside the TMS9918B VRAM requirement.
    CHECK(!AllOk(VerifyRead(MCM4517_15, 1)), "MCM4517-15 must not meet the 120 ns requirement");
}

// ----- calendar structure -----------------------------------------------------------
static void CheckStructure(const Calendar& c)
{
    for (std::size_t i = 0; i < c.cycles.size(); ++i)
    {
        const Cycle& a = c.cycles[i];
        if (i + 1 < c.cycles.size())
            CHECK(a.End() <= c.cycles[i + 1].start, "%s: cycle at %u overlaps the next", c.name, a.start);
        else
            CHECK(a.End() <= c.cycles[0].start + UnitsPerLine, "%s: last cycle overlaps the next line", c.name);
        CHECK(a.bytes >= 1 && a.bytes <= 4, "%s: cycle at %u has %u bytes", c.name, a.start, a.bytes);
    }
}

static void CheckTilesAndBitmap(const Calendar& c, bool tiles)
{
    int scans = 0;
    for (const Cycle& cy : c.cycles)
    {
        if (cy.kind == Kind::SpriteScanY) ++scans;
        const bool picture = cy.kind == Kind::NameAttr || cy.kind == Kind::PatternRow || cy.kind == Kind::BitmapQuad ||
                             cy.kind == Kind::PaletteMapPair || cy.kind == Kind::SpriteScanY;
        if (picture)
            CHECK(cy.start >= GraphicsWindowStart && cy.End() <= GraphicsWindowEnd, "%s: %s outside the window", c.name, CycleLabel(cy).c_str());
    }
    CHECK(scans == 16, "%s: %d Y reads per line (16 expected)", c.name, scans);

    for (std::uint8_t cell = 0; cell < 32; ++cell)
    {
        const std::uint16_t pixel = static_cast<std::uint16_t>(GraphicsFirstPixel + 32u * cell);
        if (tiles)
        {
            const Cycle* n = Find(c, Kind::NameAttr, cell);
            const Cycle* p = Find(c, Kind::PatternRow, cell);
            CHECK(n && p, "%s: cell %u missing", c.name, cell);
            if (!n || !p) continue;
            CHECK(p->start >= n->Sample(1) + DependencyUnits, "%s: cell %u pattern %u units after attribute", c.name, cell, p->start - n->Sample(1));
            CHECK(p->Sample(1) + PixelLoadLeadUnits <= pixel, "%s: cell %u pattern late (%u vs pixel %u)", c.name, cell, p->Sample(1), pixel);
        }
        else
        {
            const Cycle* b = Find(c, Kind::BitmapQuad, static_cast<std::uint8_t>(cell & ~1u));
            const Cycle* m = Find(c, Kind::PaletteMapPair, static_cast<std::uint8_t>(cell & ~1u));
            CHECK(b && m, "%s: cell %u missing", c.name, cell);
            if (!b || !m) continue;
            const std::uint8_t k = static_cast<std::uint8_t>(cell & 1u);
            CHECK(b->Sample(static_cast<std::uint8_t>(2 * k + 1)) + PixelLoadLeadUnits <= pixel, "%s: cell %u bitmap late", c.name, cell);
            CHECK(m->Sample(k) + PixelLoadLeadUnits <= pixel, "%s: cell %u palette late", c.name, cell);
        }
    }

    for (std::uint8_t ch = 0; ch < 8; ++ch)
    {
        const Cycle* s = Find(c, Kind::SpriteXNameColour, ch);
        const Cycle* l = Find(c, Kind::SpriteLeft, ch);
        const Cycle* r = Find(c, Kind::SpriteRight, ch);
        CHECK(s && l && r, "%s: channel %u incomplete", c.name, ch);
        if (!s || !l || !r) continue;
        CHECK(l->start >= s->Sample(2) + DependencyUnits && r->start >= s->Sample(2) + DependencyUnits,
              "%s: channel %u halves too close to name/colour", c.name, ch);
        CHECK(r->Sample(0) + PixelLoadLeadUnits <= UnitsPerLine + GraphicsFirstPixel, "%s: channel %u right half late", c.name, ch);
        CHECK(s->start >= GraphicsWindowEnd, "%s: channel %u fetched before the scan ended", c.name, ch);
    }
}

static void CheckText40(const Calendar& c)
{
    for (std::uint8_t col = 0; col < 40; ++col)
    {
        const Cycle* cc = Find(c, Kind::TextCharColour, static_cast<std::uint8_t>(col & ~1u));
        const Cycle* p = Find(c, Kind::TextPattern, col);
        CHECK(cc && p, "%s: column %u missing", c.name, col);
        if (!cc || !p) continue;
        const std::uint8_t charByte = static_cast<std::uint8_t>(2 * (col & 1u));
        const std::uint16_t pixel = static_cast<std::uint16_t>(TextFirstPixel + 24u * col);
        CHECK(p->start >= cc->Sample(charByte) + DependencyUnits, "%s: column %u pattern too close to the name", c.name, col);
        CHECK(p->Sample(0) + PixelLoadLeadUnits <= pixel, "%s: column %u pattern late", c.name, col);
        CHECK(cc->Sample(static_cast<std::uint8_t>(charByte + 1)) + PixelLoadLeadUnits <= pixel, "%s: column %u colour late", c.name, col);
        CHECK(cc->start >= TextWindowStart && p->End() <= TextWindowEnd, "%s: column %u outside the window", c.name, col);
    }
    for (std::uint8_t ch = 0; ch < 2; ++ch)
    {
        const Cycle* s = Find(c, Kind::SpriteXNameColour, ch);
        const Cycle* l = Find(c, Kind::SpriteLeft, ch);
        const Cycle* r = Find(c, Kind::SpriteRight, ch);
        CHECK(s && l && r, "%s: cursor %u incomplete", c.name, ch);
        if (s && l && r)
            CHECK(l->start >= s->Sample(2) + DependencyUnits && r->start >= s->Sample(2) + DependencyUnits &&
                  r->Sample(0) + PixelLoadLeadUnits <= UnitsPerLine + TextFirstPixel, "%s: cursor %u timing", c.name, ch);
    }
}

static void CheckText64(const Calendar& c)
{
    for (std::uint8_t col = 0; col < 64; ++col)
    {
        const Cycle* n = Find(c, Kind::TextNameQuad, static_cast<std::uint8_t>(col & ~3u));
        const Cycle* p = Find(c, Kind::TextPattern, col);
        CHECK(n && p, "%s: column %u missing", c.name, col);
        if (!n || !p) continue;
        const std::uint16_t pixel = static_cast<std::uint16_t>(GraphicsFirstPixel + 16u * col);
        CHECK(p->start >= n->Sample(static_cast<std::uint8_t>(col & 3u)) + DependencyUnits, "%s: column %u pattern too close to its name", c.name, col);
        CHECK(p->Sample(0) + PixelLoadLeadUnits <= pixel, "%s: column %u pattern late", c.name, col);
        CHECK(n->start >= GraphicsWindowStart && p->End() <= GraphicsWindowEnd, "%s: column %u outside the window", c.name, col);
    }
    for (std::uint8_t ch = 0; ch < 2; ++ch)
    {
        const Cycle* s = Find(c, Kind::SpriteXNameColour, ch);
        const Cycle* l = Find(c, Kind::SpriteLeft, ch);
        const Cycle* r = Find(c, Kind::SpriteRight, ch);
        CHECK(s && l && r, "%s: cursor %u incomplete", c.name, ch);
        if (s && l && r)
            CHECK(l->start >= s->Sample(2) + DependencyUnits && r->start >= s->Sample(2) + DependencyUnits &&
                  r->Sample(0) + PixelLoadLeadUnits <= UnitsPerLine + GraphicsFirstPixel, "%s: cursor %u timing", c.name, ch);
    }
}

// ----- page-mode alignment of every burst address ------------------------------------------
static void CheckAlignment()
{
    for (unsigned reg = 0; reg < 256; reg += 3)
    {
        const auto r = static_cast<std::uint8_t>(reg);
        for (std::uint16_t row = 0; row < 24; ++row)
            for (std::uint16_t col = 0; col < 32; ++col)
                CHECK(BurstInsidePage(NameAttrAddress(r, row, col), 2), "name|attr R2 %02X", reg);
        for (std::uint16_t name9 = 0; name9 < 512; name9 += 7)
            for (std::uint16_t row = 0; row < 8; ++row)
                CHECK(BurstInsidePage(Graphics1XPatternAddress(r, name9, row), 2), "Graphics1X pattern R4 %02X", reg);
        for (std::uint16_t line = 0; line < 192; line += 3)
        {
            for (unsigned name = 0; name < 256; name += 11)
                CHECK(BurstInsidePage(Graphics2FatPatternAddress(r, line, static_cast<std::uint8_t>(name)), 2), "Graphics2Fat pattern");
            for (std::uint16_t pair = 0; pair < 16; ++pair)
            {
                CHECK(BurstInsidePage(BitmapQuadAddress(r, line, pair), 4), "bitmap quad");
                CHECK(BurstInsidePage(BitmapQQuadAddress(r, line, pair), 4), "bitmapQ quad");
                CHECK(BurstInsidePage(PaletteMapPairAddress(r, line >> 3, pair), 2), "palette map pair");
            }
        }
        for (std::uint16_t row = 0; row < 24; ++row)
        {
            for (std::uint16_t pair = 0; pair < 20; ++pair)
                CHECK(BurstInsidePage(TextCharColourAddress(r, row, pair), 4), "text char|colour");
            for (std::uint16_t group = 0; group < 16; ++group)
                CHECK(BurstInsidePage(Text64NameQuadAddress(r, row, group), 4), "text64 names");
        }
        for (unsigned s = 0; s < 32; ++s)
        {
            const std::uint16_t a = SatXNameColourAddress(r, static_cast<std::uint8_t>(s));
            CHECK(((a & 0x7Fu) + 3u) <= PageBytes && (a & 3u) == 1u, "SAT X|name|colour burst");
        }
    }
}

// ----- CPU timing, TMS compatibility ----------------------------------------------
static double SequencerLoss(TMS99xxVramSlotSchedule::Schedule schedule, int periodPhases)
{
    TMS99xxVramSequencer q;
    q.Reset();
    q.SetCpuSchedule(schedule);
    q.SetVideoSchedule(schedule);
    TMS99xxVramSequencer::Request r;
    r.kind = TMS99xxVramSequencer::RequestKind::CpuWrite;
    r.write = true;
    q.Advance(1000);
    const int writes = 60000;
    for (int i = 0; i < writes; ++i) { q.QueueRequest(r); q.Advance(periodPhases); }
    q.Advance(4 * TMS99xxSlotGrid::PhasesPerLine);
    return 100.0 * q.GetStatistics().latchReplacements / writes;
}

static void CheckCpuTiming()
{
    using S = TMS99xxVramSlotSchedule;
    const std::pair<CalendarId, S::Schedule> tms[] = {{CalendarId::TmsGraphics, S::Schedule::Graphics},
                                                        {CalendarId::TmsMulticolor, S::Schedule::Multicolor},
                                                        {CalendarId::TmsText, S::Schedule::Text}};
    for (const auto& t : tms)
    {
        const Calendar& c = GetCalendar(t.first);
        std::vector<std::uint16_t> expected;
        for (std::uint16_t s = 0; s < 171; ++s)
            if (S::IsCpuSlot(t.second, s))
                expected.push_back(static_cast<std::uint16_t>(TMS99xxSlotGrid::RasPhase(s) * UnitsPerPhase));
        std::sort(expected.begin(), expected.end());
        CHECK(expected == CpuRasUnits(c), "%s: CPU RAS units differ from the TMS9918A", c.name);
        for (int period : {16, 21, 24, 28})
        {
            const double model = LossPercent(CpuRasUnits(c), 6u * period);
            const double seq = SequencerLoss(t.second, 3 * period);
            CHECK(model - seq < 0.5 && seq - model < 0.5, "%s %d T: model %.2f%% vs sequencer %.2f%%", c.name, period, model, seq);
        }
    }
    CHECK(WorstLatency(GetCalendar(CalendarId::TmsGraphics), Region::ActiveArea).tStates == 29, "TMS Graphics 29 T");
    CHECK(WorstLatency(GetCalendar(CalendarId::TmsText), Region::ActiveArea).tStates == 12, "TMS Text 12 T");
    CHECK(WorstLatency(GetCalendar(CalendarId::TmsMulticolor), Region::ActiveArea).tStates == 13, "TMS Multicolor 13 T");

    const Calendar& tiles = GetCalendar(CalendarId::Tiles);
    CHECK(WorstLatency(tiles, Region::WholeLine).tStates <= 18, "Tiles worst latency");
    CHECK(LossPercent(CpuRasUnits(tiles), 6u * 21) == 0.0, "Tiles: OTIR loss-free");
    CHECK(WorstLatency(GetCalendar(CalendarId::Bitmap), Region::WholeLine).tStates <= 15, "Bitmap worst latency");
    CHECK(WorstLatency(GetCalendar(CalendarId::Text40), Region::WholeLine).tStates <= 16, "Text40 worst latency");
    CHECK(LossPercent(CpuRasUnits(GetCalendar(CalendarId::Text40)), 6u * 16) == 0.0, "Text40: OUTI chain loss-free");
    const Calendar& t64 = GetCalendar(CalendarId::Text64);
    CHECK(WorstLatency(t64, Region::WholeLine).tStates <= 14, "Text64 worst latency %u T", WorstLatency(t64, Region::WholeLine).tStates);
    for (int period = 16; period <= 40; ++period)
        CHECK(LossPercent(CpuRasUnits(t64), 6u * period, 20000) == 0.0, "Text64: %d T loop loses writes", period);
}

// ----- mode decode: every TMS mode, documented or not, and every extended mode ------------------
static void CheckModes()
{
    int combos = 0;
    for (int bits = 0; bits < 64; ++bits)
    {
        const std::uint8_t r0 = (bits & 1) ? R0_M3 : 0;
        const std::uint8_t r1 = static_cast<std::uint8_t>(R1_BL | ((bits & 2) ? R1_M1 : 0) | ((bits & 4) ? R1_M2 : 0));
        const std::uint8_t r11 = static_cast<std::uint8_t>(((bits & 8) ? R11_XE : 0) | ((bits & 16) ? R11_MX : 0));
        const std::uint8_t r12 = (bits & 32) ? R12_T64 : 0;
        const int m = ModeNumber(r0, r1);
        const VideoMode mode = DecodeMode(r0, r1, r11, r12);
        const ModeInfo info = Info(mode);
        ++combos;
        if (!(bits & 8) || !(bits & 16))
        {
            CHECK(static_cast<int>(mode) == m, "TMS mode %d decoded as %s", m, info.name);
            CHECK(TmsAddressMode(mode) == m, "TMS address mode for %s", info.name);
            TMS99xxVramSlotSchedule::Schedule gear = (m & 1) ? TMS99xxVramSlotSchedule::Schedule::Text
                : ((m & 4) ? TMS99xxVramSlotSchedule::Schedule::Multicolor : TMS99xxVramSlotSchedule::Schedule::Graphics);
            const CalendarId expected = gear == TMS99xxVramSlotSchedule::Schedule::Text ? CalendarId::TmsText
                : gear == TMS99xxVramSlotSchedule::Schedule::Multicolor ? CalendarId::TmsMulticolor : CalendarId::TmsGraphics;
            CHECK(info.calendar == expected, "%s uses the wrong calendar", info.name);
            CHECK(!info.extended, "%s marked extended", info.name);
        }
        else
        {
            CHECK(info.extended, "extended combination decoded as %s", info.name);
            if ((m & 5) == 5)
                CHECK(info.calendar == CalendarId::TmsText && info.spritesPerLine == 0, "extended bars %s", info.name);
            else if (m & 1)
                CHECK((r12 ? info.calendar == CalendarId::Text64 : info.calendar == CalendarId::Text40) && info.spritesPerLine == 2,
                      "extended text %s", info.name);
            else
                CHECK(info.spritesPerLine == 8 && info.linePairSpriteScan, "extended graphics %s", info.name);
        }
    }
    CHECK(combos == 64, "combinations");
    CHECK(LineCalendar(VideoMode::Graphics1X, true, -2) == CalendarId::Tiles, "pre-scan line");
    CHECK(LineCalendar(VideoMode::Graphics1X, true, 192) == CalendarId::TmsRefresh, "border line");
    CHECK(LineCalendar(VideoMode::Text1Q, false, 10) == CalendarId::TmsRefresh, "blanked display");
    // Undocumented TMS addressing through TMS99xxTableAddress: Text 1Q names use 40 columns and M3 banking.
    CHECK(TMS99xxTableAddress::Pattern(3, 0x00, 0x07, 70, 0x41) != TMS99xxTableAddress::Pattern(3, 0x00, 0x07, 3, 0x41),
          "Text 1Q third banking");
}

// ----- 80 columns (6x8) do not fit, 64 columns (8x8) do ----------------------------------------------------------------------
static void CheckText80Feasibility()
{
    // 80 columns x 12 units: every 4 columns need 4 names (one burst) and 4 pattern bytes.
    const unsigned need = CycleUnits(4) + 4u * CycleUnits(1);
    CHECK(need > 48u, "Text80 would fit (%u units per 4 columns)", need);
    CHECK(need + CycleUnits(1) <= 64u, "Text64 group does not fit");
}

// ----- memory maps ------------------------------------------------------------------------------------
struct Range { const char* name; unsigned first; unsigned size; };

static void CheckMap(const char* title, std::initializer_list<Range> ranges)
{
    unsigned used = 0;
    for (const Range& a : ranges)
    {
        CHECK(a.first + a.size <= 0x4000, "%s: %s outside 16 KB", title, a.name);
        used += a.size;
        for (const Range& b : ranges)
            if (&a < &b)
                CHECK(a.first + a.size <= b.first || b.first + b.size <= a.first, "%s: %s overlaps %s", title, a.name, b.name);
    }
    std::printf("  %-26s %5u bytes used, %5u free\n", title, used, 0x4000 - used);
}

static void CheckMemoryMaps()
{
    std::printf("\nVRAM maps (16 KB):\n");
    CheckMap("Graphics1X", {{"patterns (512)", Graphics1XPatternAddress(0x00, 0, 0), 512 * 16},
                            {"sprite patterns + BANK", SpritePatternAddress(0x05, false, 0, 0, false), 2 * 2048},
                            {"name|attr", NameAttrAddress(0x0E, 0, 0), 768 * 2},
                            {"SAT", SatYAddress(0x7C, 0), 128}});
    CheckMap("Graphics2Fat", {{"patterns (768)", Graphics2FatPatternAddress(0x03, 0, 0), 768 * 16},
                              {"sprite patterns", SpritePatternAddress(0x06, false, 0, 0, false), 2048},
                              {"name|attr", NameAttrAddress(0x0E, 0, 0), 768 * 2},
                              {"SAT", SatYAddress(0x7C, 0), 128}});
    CheckMap("Bitmap / BitmapQ", {{"bitmap", BitmapQuadAddress(0x00, 0, 0), 192 * 64},
                                  {"sprite patterns", SpritePatternAddress(0x06, false, 0, 0, false), 2048},
                                  {"palette map", PaletteMapPairAddress(0x0E, 0, 0), 768},
                                  {"SAT", SatYAddress(0x76, 0), 128}});
    CheckMap("Text40X", {{"patterns", TextPatternAddress(0x00, 0, 0), 2048},
                         {"cursor patterns", SpritePatternAddress(0x01, false, 0, 0, false), 256},
                         {"char|colour", TextCharColourAddress(0x0C, 0, 0), 960 * 2},
                         {"SAT", SatYAddress(0x76, 0), 128}});
    CheckMap("Text64", {{"patterns", TextPatternAddress(0x00, 0, 0), 2048},
                        {"cursor patterns", SpritePatternAddress(0x01, false, 0, 0, false), 256},
                        {"names", Text64NameQuadAddress(0x0C, 0, 0), 64 * 24},
                        {"SAT", SatYAddress(0x76, 0), 128}});
    CHECK(BitmapQQuadAddress(0x03, 191, 15) == 0x2FFC, "BitmapQ third 2 ends at 0x2FFF");
}

int main()
{
    CheckDram();
    for (CalendarId id : AllCalendars())
        CheckStructure(GetCalendar(id));
    CheckTilesAndBitmap(GetCalendar(CalendarId::Tiles), true);
    CheckTilesAndBitmap(GetCalendar(CalendarId::Bitmap), false);
    CheckText40(GetCalendar(CalendarId::Text40));
    CheckText64(GetCalendar(CalendarId::Text64));
    CheckAlignment();
    CheckCpuTiming();
    CheckText80Feasibility();
    CheckModes();
    CheckMemoryMaps();
    std::printf(g_failures ? "\n%d checks FAILED\n" : "\nall checks passed\n", g_failures);
    return g_failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
