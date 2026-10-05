#pragma once
#include <string>
#include <vector>

namespace moodgen
{
constexpr int kPPQ = 960;
constexpr int kStepTicks = kPPQ / 4;

enum Genre { RnB = 0, Reggaeton, House, Pop, JazzSoul, NumGenres };
enum Track { Chords = 0, Bass, Melody, Drums, NumTracks };
enum Mode { Auto = 0, Minor, Major, Dorian, Phrygian, Mixolydian, Lydian, NumModes };

struct Note
{
    int pitch = 60;
    int startTick = 0;
    int lengthTicks = kStepTicks;
    int velocity = 90;
};

struct MoodProfile
{
    bool hasBrightness = false, hasIntensity = false;
    float brightness = 0.5f;
    float intensity = 0.5f;
    float densityDelta = 0.0f;
    float swingDelta = 0.0f;
    float richness = 0.0f;
    int mode = Auto;
    int bpmDelta = 0;
    int bpmAbs = 0;
    std::vector<std::string> tags;
};

struct Params
{
    int genre = RnB;
    int root = 0;
    int mode = Auto;
    float brightness = 0.3f;
    float intensity = 0.35f;
    float density = 0.55f;
    float variation = 0.4f;
    int bars = 4;
    unsigned harmonySeed = 1;
    MoodProfile profile;
};

std::string stripAccents(const std::string& s);
MoodProfile parseMood(const std::string& text);
int defaultBpm(int genre);
int resolveMode(const Params& p);
int clipLengthTicks(const Params& p);
std::vector<Note> generate(const Params& p, int track, unsigned seed);
const char* genreName(int genre);
const char* trackName(int track);
}
