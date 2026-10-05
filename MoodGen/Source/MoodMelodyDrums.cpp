#include "MoodInternal.h"
#include <cmath>

namespace moodgen::detail
{
namespace
{
struct Cell { int step, len; };

const std::vector<Cell>& rhythmPool(int genre)
{
    static const std::vector<Cell> p[NumGenres] = {
        { { 0, 3 }, { 3, 2 }, { 6, 2 }, { 8, 3 }, { 11, 2 }, { 14, 2 } },
        { { 0, 2 }, { 3, 2 }, { 6, 2 }, { 8, 2 }, { 11, 2 }, { 14, 2 } },
        { { 2, 2 }, { 4, 2 }, { 7, 1 }, { 10, 2 }, { 12, 2 }, { 15, 1 } },
        { { 0, 2 }, { 2, 2 }, { 4, 4 }, { 8, 2 }, { 10, 2 }, { 12, 4 } },
        { { 0, 3 }, { 3, 1 }, { 4, 2 }, { 7, 2 }, { 9, 1 }, { 10, 2 }, { 13, 3 } },
    };
    return p[genre];
}
}

std::vector<Note> makeMelody(const Ctx& c, std::mt19937& rng)
{
    std::vector<Note> out;
    const auto& pool = rhythmPool(c.p.genre);
    int idx = 14 + 2;
    const int lo = 10, hi = 19;

    for (int bar = 0; bar < c.p.bars; ++bar)
    {
        const int d = degreeAt(c, bar);
        const bool last = (bar % 4) == 3;
        int made = 0;

        for (size_t h = 0; h < pool.size(); ++h)
        {
            const bool strong = pool[h].step % 4 == 0;
            const float chance = 0.25f + c.dens * 0.6f + (strong ? 0.2f : 0.0f);
            if (rnd(rng) > chance)
                continue;

            const float r = rnd(rng);
            const int jump = (c.p.variation > 0.6f && r < 0.15f) ? rndInt(rng, 3, 4) : 1;
            int dir = (rnd(rng) < 0.5f) ? -1 : 1;
            if (idx >= hi - 1) dir = -1;
            if (idx <= lo + 1) dir = 1;
            const int step = (r < 0.18f) ? 0 : dir * jump;
            idx = std::clamp(idx + step, lo, hi);

            int tone = idx;
            if (strong)
            {
                const int rootIdx = 14 + d;
                const int options[3] = { rootIdx, rootIdx + 2, rootIdx + 4 };
                int best = options[0];
                for (int o : options)
                    for (int octave = -7; octave <= 7; octave += 7)
                        if (std::abs(o + octave - idx) < std::abs(best - idx))
                            best = o + octave;
                tone = std::clamp(best, lo, hi);
                idx = tone;
            }

            int len = pool[h].len;
            if (rnd(rng) < 0.2f * (1.0f - c.dens))
                len += 2;
            const int vel = (int) (66 + c.inten * 28) + (strong ? 6 : 0) + rndInt(rng, -6, 6);
            out.push_back(makeNote(c, rng, scalePitch(c, tone), bar, pool[h].step, len, vel, 5));
            ++made;
        }

        if (last && made > 0)
        {
            const int rootIdx = 14 + degreeAt(c, 0);
            out.push_back(makeNote(c, rng, scalePitch(c, rootIdx), bar, 14, 2, 80, 3));
            idx = rootIdx;
        }
    }
    return out;
}

namespace
{
void add(std::vector<Note>& v, const Ctx& c, std::mt19937& rng, int pitch, int bar, int step, int vel, int len = 1, int jitter = 4)
{
    v.push_back(makeNote(c, rng, pitch, bar, step, len, vel, jitter));
}

constexpr int KICK = 36, SNARE = 38, CLAP = 39, HHC = 42, HHO = 46, RIM = 37, SHAKER = 70, CRASH = 49;
}

std::vector<Note> makeDrums(const Ctx& c, std::mt19937& rng)
{
    std::vector<Note> out;
    const float d = c.dens;
    const float v = c.p.variation;
    const int base = (int) (74 + c.inten * 30);

    for (int bar = 0; bar < c.p.bars; ++bar)
    {
        const bool fill = (bar % 4) == 3 && v > 0.25f && rnd(rng) < 0.4f + v * 0.5f;

        for (int s = 0; s < 16; ++s)
        {
            const bool off = s % 2 == 1;

            switch (c.p.genre)
            {
                case Reggaeton:
                {
                    if (s == 0 || s == 4 || s == 8 || s == 12)
                        add(out, c, rng, KICK, bar, s, base + 14, 1, 2);
                    if (s == 3 || s == 7 || s == 11 || s == 15)
                        add(out, c, rng, SNARE, bar, s, base + 6);
                    if (s == 6 || s == 14)
                        if (rnd(rng) < 0.3f + d * 0.5f)
                            add(out, c, rng, RIM, bar, s, base - 22);
                    if (rnd(rng) < 0.55f + d * 0.4f)
                        add(out, c, rng, HHC, bar, s, base - 26 + (off ? 0 : 8), 1, 5);
                    if (s == 15 && rnd(rng) < 0.3f * v)
                        add(out, c, rng, HHO, bar, s, base - 18);
                    break;
                }
                case House:
                {
                    if (s % 4 == 0)
                        add(out, c, rng, KICK, bar, s, base + 16, 1, 1);
                    if (s == 4 || s == 12)
                        add(out, c, rng, CLAP, bar, s, base + 4);
                    if (s % 4 == 2)
                        add(out, c, rng, HHO, bar, s, base - 14, 1, 2);
                    if (off && rnd(rng) < 0.35f + d * 0.5f)
                        add(out, c, rng, HHC, bar, s, base - 30, 1, 4);
                    if (rnd(rng) < d * 0.5f)
                        add(out, c, rng, SHAKER, bar, s, base - 36, 1, 5);
                    break;
                }
                case JazzSoul:
                {
                    if (s == 0 || (s == 10 && rnd(rng) < 0.5f + v * 0.3f))
                        add(out, c, rng, KICK, bar, s, base - 8, 1, 6);
                    if (s == 4 || s == 12)
                        add(out, c, rng, RIM, bar, s, base - 14, 1, 6);
                    if (s % 4 == 0 || s % 4 == 3)
                        add(out, c, rng, HHC, bar, s, base - 28, 1, 6);
                    if (s == 6 || s == 14)
                        if (rnd(rng) < 0.25f + d * 0.4f)
                            add(out, c, rng, SNARE, bar, s, base - 40, 1, 6);
                    break;
                }
                case Pop:
                {
                    if (s == 0 || s == 8 || (s == 10 && rnd(rng) < 0.4f + v * 0.3f))
                        add(out, c, rng, KICK, bar, s, base + 10, 1, 2);
                    if (s == 4 || s == 12)
                        add(out, c, rng, SNARE, bar, s, base + 8, 1, 3);
                    if (s % 2 == 0)
                        add(out, c, rng, HHC, bar, s, base - 26 + (s % 4 == 0 ? 8 : 0), 1, 4);
                    else if (rnd(rng) < d * 0.5f)
                        add(out, c, rng, HHC, bar, s, base - 38, 1, 4);
                    break;
                }
                default:
                {
                    if (s == 0 || (s == 7 && rnd(rng) < 0.5f) || (s == 10 && rnd(rng) < 0.6f + v * 0.2f))
                        add(out, c, rng, KICK, bar, s, base + 4, 1, 5);
                    if (s == 4 || s == 12)
                        add(out, c, rng, SNARE, bar, s, base, 1, 6);
                    if (s == 6 && rnd(rng) < 0.3f * v + 0.1f)
                        add(out, c, rng, SNARE, bar, s, base - 42, 1, 6);
                    if (rnd(rng) < 0.5f + d * 0.4f)
                        add(out, c, rng, HHC, bar, s, base - 30 + (off ? -6 : 4), 1, 6);
                    break;
                }
            }

            if (fill && s >= 12)
                add(out, c, rng, SNARE, bar, s, base - 6 + (s - 12) * 6, 1, 2);
        }

        if (bar % 4 == 0 && c.inten > 0.65f && rnd(rng) < 0.6f)
            add(out, c, rng, CRASH, bar, 0, base - 6, 4, 1);
    }
    return out;
}
}
