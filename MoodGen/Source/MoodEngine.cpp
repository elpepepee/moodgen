#include "MoodInternal.h"
#include <cmath>

namespace moodgen
{
namespace
{
const int kScales[NumModes][7] = {
    { 0, 2, 3, 5, 7, 8, 10 },
    { 0, 2, 3, 5, 7, 8, 10 },
    { 0, 2, 4, 5, 7, 9, 11 },
    { 0, 2, 3, 5, 7, 9, 10 },
    { 0, 1, 3, 5, 7, 8, 10 },
    { 0, 2, 4, 5, 7, 9, 10 },
    { 0, 2, 4, 6, 7, 9, 11 },
};

const int kProgressions[NumGenres][2][4][4] = {
    { { { 0, 3, 6, 2 }, { 0, 5, 3, 6 }, { 5, 3, 0, 6 }, { 0, 2, 3, 6 } },
      { { 0, 5, 3, 4 }, { 3, 4, 2, 5 }, { 1, 4, 0, 5 }, { 0, 2, 3, 4 } } },
    { { { 0, 6, 5, 6 }, { 0, 6, 5, 4 }, { 0, 5, 2, 6 }, { 0, 3, 6, 5 } },
      { { 0, 4, 5, 3 }, { 0, 5, 3, 4 }, { 0, 3, 0, 4 }, { 5, 3, 0, 4 } } },
    { { { 0, 5, 2, 6 }, { 0, 0, 5, 6 }, { 0, 3, 5, 6 }, { 0, 3, 0, 6 } },
      { { 0, 3, 5, 4 }, { 3, 4, 2, 5 }, { 0, 4, 5, 3 }, { 1, 4, 0, 0 } } },
    { { { 0, 3, 6, 5 }, { 0, 5, 2, 6 }, { 0, 4, 5, 3 }, { 5, 3, 0, 6 } },
      { { 0, 4, 5, 3 }, { 5, 3, 0, 4 }, { 0, 5, 3, 4 }, { 3, 0, 4, 5 } } },
    { { { 0, 3, 6, 2 }, { 0, 5, 1, 4 }, { 3, 6, 2, 5 }, { 0, 3, 1, 4 } },
      { { 1, 4, 0, 5 }, { 2, 5, 1, 4 }, { 0, 5, 1, 4 }, { 3, 2, 1, 4 } } },
};

const float kSwingBase[NumGenres] = { 0.12f, 0.0f, 0.04f, 0.0f, 0.2f };
const float kRichBias[NumGenres] = { 0.35f, -0.2f, 0.1f, -0.1f, 0.6f };
const int kBpm[NumGenres] = { 80, 95, 124, 110, 90 };

bool isDarkMode(int mode)
{
    return mode == Minor || mode == Dorian || mode == Phrygian;
}
}

const char* genreName(int g)
{
    static const char* n[] = { "R&B", "Reguet\xC3\xB3n", "House", "Pop", "Jazz soul" };
    return n[std::clamp(g, 0, (int) NumGenres - 1)];
}

const char* trackName(int t)
{
    static const char* n[] = { "Acordes", "Bajo", "Melod\xC3\xAD" "a", "Bater\xC3\xAD" "a" };
    return n[std::clamp(t, 0, (int) NumTracks - 1)];
}

int defaultBpm(int genre) { return kBpm[std::clamp(genre, 0, (int) NumGenres - 1)]; }

int clipLengthTicks(const Params& p) { return p.bars * 16 * kStepTicks; }

int resolveMode(const Params& p)
{
    if (p.mode != Auto)
        return p.mode;
    if (p.profile.mode != Auto)
        return p.profile.mode;

    const float b = p.profile.hasBrightness ? p.profile.brightness : p.brightness;
    const bool jazzy = p.genre == JazzSoul || p.genre == RnB;
    if (b < 0.15f)
        return p.genre == Reggaeton ? Phrygian : Minor;
    if (b < 0.35f)
        return Minor;
    if (b < 0.5f)
        return jazzy ? Dorian : Minor;
    if (b < 0.7f)
        return (jazzy || p.genre == House) ? Mixolydian : Major;
    return p.genre == JazzSoul ? Lydian : Major;
}

namespace detail
{
float rnd(std::mt19937& rng) { return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng); }

int rndInt(std::mt19937& rng, int lo, int hi)
{
    return std::uniform_int_distribution<int>(lo, hi)(rng);
}

int humanize(std::mt19937& rng, int amount)
{
    return amount <= 0 ? 0 : rndInt(rng, -amount, amount);
}

int scalePitch(const Ctx& c, int index)
{
    return c.rootMidi + c.scale[(size_t) (index % 7)] + 12 * (index / 7);
}

int degreeAt(const Ctx& c, int bar) { return c.degrees[(size_t) (bar % 4)]; }

int swingShift(const Ctx& c, int step)
{
    return (step % 2 == 1) ? (int) std::lround(c.swing * kStepTicks) : 0;
}

Note makeNote(const Ctx& c, std::mt19937& rng, int pitch, int bar, int step, int lenSteps, int vel, int jitter)
{
    Note n;
    n.pitch = std::clamp(pitch, 0, 127);
    n.startTick = std::max(0, (bar * 16 + step) * kStepTicks + swingShift(c, step) + humanize(rng, jitter));
    n.lengthTicks = std::max(30, lenSteps * kStepTicks - 20);
    n.velocity = std::clamp(vel, 1, 127);
    return n;
}

Ctx makeCtx(const Params& p)
{
    Ctx c;
    c.p = p;
    c.p.genre = std::clamp(p.genre, 0, (int) NumGenres - 1);
    c.mode = resolveMode(c.p);
    c.rootMidi = 48 + std::clamp(p.root, 0, 11);
    for (size_t i = 0; i < 7; ++i)
        c.scale[i] = kScales[c.mode][i];

    c.bright = p.profile.hasBrightness ? p.profile.brightness : p.brightness;
    c.inten = p.profile.hasIntensity ? p.profile.intensity : p.intensity;
    c.dens = std::clamp(p.density + p.profile.densityDelta + (c.inten - 0.5f) * 0.3f, 0.05f, 1.0f);
    c.swing = std::clamp(kSwingBase[c.p.genre] + p.profile.swingDelta, 0.0f, 0.4f);
    c.richness = std::clamp(0.3f + p.profile.richness + kRichBias[c.p.genre], 0.0f, 1.0f);

    std::mt19937 hr(p.harmonySeed * 2246822519u + 3266489917u);
    const int family = isDarkMode(c.mode) ? 0 : 1;
    const int pick = rndInt(hr, 0, 3);
    for (int i = 0; i < 4; ++i)
        c.degrees.push_back(kProgressions[c.p.genre][family][pick][i]);
    return c;
}

namespace
{
std::vector<int> voiceChord(const std::vector<int>& tones, const std::vector<int>& prev)
{
    std::vector<int> best;
    float bestCost = 1e9f;
    for (int relax = 0; relax < 2 && best.empty(); ++relax)
        for (size_t inv = 0; inv < tones.size(); ++inv)
            for (int shift = -12; shift <= 12; shift += 12)
            {
                std::vector<int> cand = tones;
                for (size_t k = 0; k < inv; ++k)
                    cand[k] += 12;
                std::sort(cand.begin(), cand.end());
                float mean = 0;
                for (auto& n : cand)
                {
                    n += shift;
                    mean += (float) n;
                }
                mean /= (float) cand.size();
                if (relax == 0 && (cand.front() < 50 || cand.back() > 79))
                    continue;
                float cost = 0.3f * std::abs(mean - 63.0f);
                if (prev.empty())
                    cost = std::abs(mean - 63.0f) * (float) cand.size();
                else
                    for (size_t j = 0; j < std::min(prev.size(), cand.size()); ++j)
                        cost += (float) std::abs(cand[j] - prev[j]);
                if (cost < bestCost)
                {
                    bestCost = cost;
                    best = cand;
                }
            }
    return best;
}

struct Hit { int step, len; };

const std::vector<Hit>& chordPattern(int genre)
{
    static const std::vector<Hit> p[NumGenres] = {
        { { 0, 6 }, { 6, 4 }, { 10, 6 } },
        { { 0, 2 }, { 3, 2 }, { 6, 2 }, { 11, 2 }, { 14, 2 } },
        { { 2, 2 }, { 6, 2 }, { 10, 2 }, { 14, 2 } },
        { { 0, 8 }, { 8, 8 } },
        { { 0, 5 }, { 6, 3 }, { 10, 2 }, { 12, 4 } },
    };
    return p[genre];
}
}

std::vector<Note> makeChords(const Ctx& c, std::mt19937& rng)
{
    std::vector<Note> out;
    std::vector<int> prev;
    const int count = c.richness < 0.25f ? 3 : (c.richness < 0.55f ? 4 : (c.richness < 0.9f ? 5 : 6));
    const bool strum = c.p.genre == RnB || c.p.genre == JazzSoul;

    for (int bar = 0; bar < c.p.bars; ++bar)
    {
        const int d = degreeAt(c, bar);
        std::vector<int> tones;
        for (int k = 0; k < count; ++k)
            tones.push_back(scalePitch(c, d + 2 * k));
        if (count >= 5 && (c.p.genre == RnB || c.p.genre == JazzSoul))
            tones.erase(tones.begin() + 2);
        const auto voiced = voiceChord(tones, prev);
        prev = voiced;

        const auto& pat = chordPattern(c.p.genre);
        for (size_t h = 0; h < pat.size(); ++h)
        {
            if (h > 0 && rnd(rng) > 0.55f + c.dens * 0.45f)
                continue;
            const int vel = (int) (62 + c.inten * 26);
            for (size_t k = 0; k < voiced.size(); ++k)
            {
                Note n = makeNote(c, rng, voiced[k], bar, pat[h].step, pat[h].len, vel + rndInt(rng, -5, 5), 4);
                if (strum)
                    n.startTick += (int) k * 7;
                out.push_back(n);
            }
        }
    }
    return out;
}

namespace
{
struct BassHit { int step, len, kind; };

const std::vector<BassHit>& bassPattern(int genre)
{
    static const std::vector<BassHit> p[NumGenres] = {
        { { 0, 5, 0 }, { 6, 3, 1 }, { 10, 2, 0 }, { 12, 4, 0 } },
        { { 0, 3, 0 }, { 3, 3, 0 }, { 8, 3, 0 }, { 11, 3, 0 } },
        { { 0, 2, 0 }, { 2, 2, 0 }, { 6, 2, 0 }, { 10, 2, 0 }, { 14, 2, 0 } },
        { { 0, 4, 0 }, { 4, 3, 0 }, { 8, 4, 0 }, { 12, 3, 1 } },
        { { 0, 4, 0 }, { 4, 4, 1 }, { 8, 4, 0 }, { 12, 4, 3 } },
    };
    return p[genre];
}

int foldBass(int pitch)
{
    while (pitch > 47) pitch -= 12;
    while (pitch < 36) pitch += 12;
    return pitch;
}
}

std::vector<Note> makeBass(const Ctx& c, std::mt19937& rng)
{
    std::vector<Note> out;
    for (int bar = 0; bar < c.p.bars; ++bar)
    {
        const int d = degreeAt(c, bar);
        const int nextD = degreeAt(c, bar + 1);
        const int root = foldBass(scalePitch(c, d));
        const auto& pat = bassPattern(c.p.genre);
        for (size_t h = 0; h < pat.size(); ++h)
        {
            if (h > 0 && rnd(rng) > 0.7f + c.dens * 0.3f)
                continue;
            int kind = pat[h].kind;
            if (kind == 0 && rnd(rng) < c.p.variation * c.inten * 0.15f)
                kind = 2;
            int pitch = root;
            if (kind == 1) pitch = foldBass(scalePitch(c, d + 4));
            if (kind == 2) pitch = root + 12;
            if (kind == 3) pitch = foldBass(scalePitch(c, nextD) - 1);
            const int vel = (int) (78 + c.inten * 35) + rndInt(rng, -6, 6);
            out.push_back(makeNote(c, rng, pitch, bar, pat[h].step, pat[h].len, vel, 3));
        }
    }
    return out;
}
}

std::vector<Note> generate(const Params& p, int track, unsigned seed)
{
    const auto c = detail::makeCtx(p);
    std::mt19937 rng(seed * 2654435761u + (unsigned) track * 40503u + 17u);

    std::vector<Note> notes;
    switch (track)
    {
        case Chords: notes = detail::makeChords(c, rng); break;
        case Bass: notes = detail::makeBass(c, rng); break;
        case Melody: notes = detail::makeMelody(c, rng); break;
        default: notes = detail::makeDrums(c, rng); break;
    }

    const int total = clipLengthTicks(p);
    std::vector<Note> out;
    for (auto n : notes)
    {
        if (n.startTick >= total)
            continue;
        n.lengthTicks = std::min(n.lengthTicks, total - n.startTick);
        if (n.lengthTicks >= 20)
            out.push_back(n);
    }
    std::stable_sort(out.begin(), out.end(), [](const Note& a, const Note& b) {
        return a.startTick < b.startTick;
    });
    return out;
}
}
