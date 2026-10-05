#pragma once
#include "MoodEngine.h"
#include <algorithm>
#include <array>
#include <random>

namespace moodgen::detail
{
struct Ctx
{
    Params p;
    int mode = Minor;
    int rootMidi = 48;
    std::array<int, 7> scale {};
    float bright = 0.5f, inten = 0.5f, dens = 0.5f, swing = 0.0f, richness = 0.3f;
    std::vector<int> degrees;
};

Ctx makeCtx(const Params& p);
int degreeAt(const Ctx& c, int bar);
int scalePitch(const Ctx& c, int index);
int swingShift(const Ctx& c, int step);
int humanize(std::mt19937& rng, int amount);
float rnd(std::mt19937& rng);
int rndInt(std::mt19937& rng, int lo, int hi);
Note makeNote(const Ctx& c, std::mt19937& rng, int pitch, int bar, int step, int lenSteps, int vel, int jitter);

std::vector<Note> makeChords(const Ctx& c, std::mt19937& rng);
std::vector<Note> makeBass(const Ctx& c, std::mt19937& rng);
std::vector<Note> makeMelody(const Ctx& c, std::mt19937& rng);
std::vector<Note> makeDrums(const Ctx& c, std::mt19937& rng);
}
