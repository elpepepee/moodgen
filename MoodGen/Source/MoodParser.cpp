#include "MoodEngine.h"
#include <algorithm>
#include <cctype>
#include <cstring>

namespace moodgen
{
std::string stripAccents(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i)
    {
        const unsigned char c = (unsigned char) s[i];
        if (c == 0xC3 && i + 1 < s.size())
        {
            const unsigned char d = (unsigned char) s[++i];
            switch (d)
            {
                case 0xA1: case 0x81: out += 'a'; break;
                case 0xA9: case 0x89: out += 'e'; break;
                case 0xAD: case 0x8D: out += 'i'; break;
                case 0xB3: case 0x93: out += 'o'; break;
                case 0xBA: case 0x9A: case 0xBC: case 0x9C: out += 'u'; break;
                case 0xB1: case 0x91: out += 'n'; break;
                default: break;
            }
        }
        else if (c < 0x80)
            out += (char) std::tolower(c);
    }
    return out;
}

namespace
{
struct Rule
{
    const char* stem;
    float bright, inten, dens, swing, rich;
    int bpm;
    int mode;
};

const Rule kRules[] = {
    { "oscur", -0.35f, 0, 0, 0, 0, 0, Auto },
    { "dark", -0.35f, 0, 0, 0, 0, 0, Auto },
    { "sombri", -0.35f, 0, 0, 0, 0, 0, Auto },
    { "nocturn", -0.3f, -0.1f, 0, 0, 0, -2, Auto },
    { "noche", -0.3f, -0.1f, 0, 0, 0, -2, Auto },
    { "night", -0.3f, -0.1f, 0, 0, 0, -2, Auto },
    { "triste", -0.3f, -0.1f, 0, 0, 0, -4, Auto },
    { "sad", -0.3f, -0.1f, 0, 0, 0, -4, Auto },
    { "melancol", -0.3f, -0.1f, 0, 0, 0.1f, -4, Auto },
    { "nostalg", -0.1f, -0.15f, 0, 0, 0.2f, -2, Auto },
    { "vintage", -0.05f, -0.1f, 0, 0.05f, 0.15f, -2, Auto },
    { "retro", 0.0f, 0, 0, 0.05f, 0.1f, 0, Auto },
    { "brillant", 0.35f, 0, 0, 0, 0, 0, Auto },
    { "bright", 0.35f, 0, 0, 0, 0, 0, Auto },
    { "luminos", 0.35f, 0, 0, 0, 0, 0, Auto },
    { "feliz", 0.35f, 0.15f, 0, 0, 0, 2, Auto },
    { "happy", 0.35f, 0.15f, 0, 0, 0, 2, Auto },
    { "alegr", 0.35f, 0.15f, 0, 0, 0, 2, Auto },
    { "optimis", 0.3f, 0.15f, 0, 0, 0, 2, Auto },
    { "sunny", 0.3f, 0.1f, 0, 0, 0, 0, Auto },
    { "soleado", 0.3f, 0.1f, 0, 0, 0, 0, Auto },
    { "verano", 0.3f, 0.15f, 0, 0, 0, 2, Auto },
    { "summer", 0.3f, 0.15f, 0, 0, 0, 2, Auto },
    { "dulce", 0.2f, -0.2f, 0, 0, 0.1f, 0, Auto },
    { "sweet", 0.2f, -0.2f, 0, 0, 0.1f, 0, Auto },
    { "tierno", 0.2f, -0.25f, 0, 0, 0, 0, Auto },
    { "calm", 0, -0.35f, -0.12f, 0, 0, -5, Auto },
    { "chill", 0, -0.35f, -0.12f, 0, 0, -5, Auto },
    { "relaj", 0, -0.35f, -0.12f, 0, 0, -5, Auto },
    { "relax", 0, -0.35f, -0.12f, 0, 0, -5, Auto },
    { "suave", 0, -0.3f, -0.1f, 0, 0, -3, Auto },
    { "soft", 0, -0.3f, -0.1f, 0, 0, -3, Auto },
    { "tranquil", 0, -0.35f, -0.12f, 0, 0, -5, Auto },
    { "lofi", -0.05f, -0.3f, -0.05f, 0.1f, 0.15f, -6, Auto },
    { "intens", 0, 0.4f, 0.1f, 0, 0, 5, Auto },
    { "energ", 0, 0.4f, 0.1f, 0, 0, 5, Auto },
    { "hype", 0, 0.4f, 0.1f, 0, 0, 5, Auto },
    { "agresiv", -0.1f, 0.45f, 0.12f, 0, 0, 6, Auto },
    { "aggressive", -0.1f, 0.45f, 0.12f, 0, 0, 6, Auto },
    { "fuerte", 0, 0.35f, 0.08f, 0, 0, 3, Auto },
    { "potente", 0, 0.35f, 0.08f, 0, 0, 3, Auto },
    { "heavy", -0.1f, 0.35f, 0.08f, 0, 0, 3, Auto },
    { "euforic", 0.2f, 0.35f, 0.12f, 0, 0, 4, Auto },
    { "festiv", 0.25f, 0.3f, 0.12f, 0, 0, 4, Auto },
    { "fiesta", 0.25f, 0.3f, 0.12f, 0, 0, 4, Auto },
    { "party", 0.25f, 0.3f, 0.12f, 0, 0, 4, Auto },
    { "bailable", 0.15f, 0.25f, 0.1f, 0, 0, 3, Auto },
    { "dance", 0.15f, 0.25f, 0.1f, 0, 0, 3, Auto },
    { "club", 0.0f, 0.3f, 0.1f, 0, 0, 3, Auto },
    { "tens", -0.2f, 0.25f, 0, 0, 0, 0, Auto },
    { "suspens", -0.2f, 0.2f, -0.05f, 0, 0.1f, 0, Auto },
    { "misterios", -0.3f, 0.0f, -0.05f, 0, 0.15f, -2, Auto },
    { "mystery", -0.3f, 0.0f, -0.05f, 0, 0.15f, -2, Auto },
    { "dramatic", -0.15f, 0.3f, 0, 0, 0.1f, 0, Auto },
    { "epic", 0.05f, 0.4f, 0.05f, 0, 0.05f, 2, Auto },
    { "cinemat", -0.05f, 0.25f, -0.05f, 0, 0.15f, -2, Auto },
    { "peligros", -0.25f, 0.25f, 0, 0, 0, 0, Auto },
    { "dangerous", -0.25f, 0.25f, 0, 0, 0, 0, Auto },
    { "calle", -0.15f, 0.2f, 0, 0, 0, 0, Auto },
    { "street", -0.15f, 0.2f, 0, 0, 0, 0, Auto },
    { "sensual", -0.1f, -0.2f, 0, 0.08f, 0.25f, -3, Auto },
    { "romantic", 0.0f, -0.2f, 0, 0.06f, 0.2f, -3, Auto },
    { "intim", -0.05f, -0.3f, -0.1f, 0.05f, 0.15f, -3, Auto },
    { "sexy", -0.1f, -0.1f, 0, 0.08f, 0.2f, -2, Auto },
    { "seduct", -0.1f, -0.15f, 0, 0.08f, 0.2f, -3, Auto },
    { "smooth", 0.0f, -0.2f, 0, 0.06f, 0.25f, -2, Auto },
    { "aterciopel", -0.05f, -0.25f, 0, 0.06f, 0.25f, -3, Auto },
    { "velvet", -0.05f, -0.25f, 0, 0.06f, 0.25f, -3, Auto },
    { "sonador", 0.05f, -0.25f, -0.15f, 0, 0.25f, -3, Auto },
    { "sueno", 0.05f, -0.25f, -0.15f, 0, 0.25f, -3, Auto },
    { "dream", 0.05f, -0.25f, -0.15f, 0, 0.25f, -3, Auto },
    { "etere", 0.1f, -0.3f, -0.15f, 0, 0.25f, -4, Auto },
    { "ethereal", 0.1f, -0.3f, -0.15f, 0, 0.25f, -4, Auto },
    { "ambient", 0.0f, -0.35f, -0.2f, 0, 0.2f, -6, Auto },
    { "espacial", 0.0f, -0.25f, -0.15f, 0, 0.15f, -3, Auto },
    { "jazz", 0, 0, 0, 0.06f, 0.4f, 0, Auto },
    { "soul", 0, 0, 0, 0.05f, 0.3f, 0, Auto },
    { "sofistic", 0, 0, 0, 0, 0.35f, 0, Auto },
    { "elegan", 0, -0.05f, 0, 0, 0.25f, 0, Auto },
    { "lush", 0, 0, 0, 0, 0.35f, 0, Auto },
    { "rico", 0, 0, 0, 0, 0.3f, 0, Auto },
    { "complej", 0, 0, 0.05f, 0, 0.4f, 0, Auto },
    { "extended", 0, 0, 0, 0, 0.4f, 0, Auto },
    { "simple", 0, 0, -0.1f, 0, -0.35f, 0, Auto },
    { "minimal", 0, 0, -0.2f, 0, -0.3f, 0, Auto },
    { "basic", 0, 0, -0.1f, 0, -0.3f, 0, Auto },
    { "limpio", 0, 0, -0.1f, 0, -0.2f, 0, Auto },
    { "clean", 0, 0, -0.1f, 0, -0.2f, 0, Auto },
    { "sparse", 0, 0, -0.25f, 0, 0, 0, Auto },
    { "espaciado", 0, 0, -0.2f, 0, 0, 0, Auto },
    { "denso", 0, 0, 0.25f, 0, 0, 0, Auto },
    { "busy", 0, 0, 0.25f, 0, 0, 0, Auto },
    { "cargado", 0, 0, 0.2f, 0, 0, 0, Auto },
    { "lleno", 0, 0, 0.2f, 0, 0, 0, Auto },
    { "swing", 0, 0, 0, 0.12f, 0, 0, Auto },
    { "groove", 0, 0, 0, 0.1f, 0, 0, Auto },
    { "shuffle", 0, 0, 0, 0.14f, 0, 0, Auto },
    { "rebote", 0, 0, 0, 0.1f, 0, 0, Auto },
    { "bounce", 0, 0, 0, 0.1f, 0, 0, Auto },
    { "recto", 0, 0, 0, -0.5f, 0, 0, Auto },
    { "straight", 0, 0, 0, -0.5f, 0, 0, Auto },
    { "robot", 0, 0, 0, -0.5f, 0, 0, Auto },
    { "rapid", 0, 0, 0, 0, 0, 8, Auto },
    { "fast", 0, 0, 0, 0, 0, 8, Auto },
    { "acelerad", 0, 0.05f, 0, 0, 0, 8, Auto },
    { "lent", 0, 0, 0, 0, 0, -8, Auto },
    { "slow", 0, 0, 0, 0, 0, -8, Auto },
    { "pausad", 0, 0, 0, 0, 0, -8, Auto },
    { "menor", 0, 0, 0, 0, 0, 0, Minor },
    { "minor", 0, 0, 0, 0, 0, 0, Minor },
    { "eolic", 0, 0, 0, 0, 0, 0, Minor },
    { "aeolian", 0, 0, 0, 0, 0, 0, Minor },
    { "mayor", 0, 0, 0, 0, 0, 0, Major },
    { "major", 0, 0, 0, 0, 0, 0, Major },
    { "jonic", 0, 0, 0, 0, 0, 0, Major },
    { "doric", 0, 0, 0, 0, 0, 0, Dorian },
    { "dorian", 0, 0, 0, 0, 0, 0, Dorian },
    { "frigi", 0, 0, 0, 0, 0, 0, Phrygian },
    { "phrygian", 0, 0, 0, 0, 0, 0, Phrygian },
    { "mixolid", 0, 0, 0, 0, 0, 0, Mixolydian },
    { "mixolyd", 0, 0, 0, 0, 0, 0, Mixolydian },
    { "lidi", 0, 0, 0, 0, 0, 0, Lydian },
    { "lydian", 0, 0, 0, 0, 0, 0, Lydian },
};

bool startsWith(const std::string& s, const char* stem)
{
    return s.compare(0, std::strlen(stem), stem) == 0;
}

bool isOneOf(const std::string& s, std::initializer_list<const char*> list)
{
    for (auto* w : list)
        if (s == w)
            return true;
    return false;
}

bool isNumber(const std::string& s)
{
    return !s.empty() && std::all_of(s.begin(), s.end(), [](char c) { return std::isdigit((unsigned char) c) != 0; });
}

std::vector<std::string> tokenize(const std::string& text)
{
    const std::string clean = stripAccents(text);
    std::vector<std::string> out;
    std::string cur;
    for (char c : clean)
    {
        if (std::isalnum((unsigned char) c))
            cur += c;
        else if (!cur.empty())
        {
            out.push_back(cur);
            cur.clear();
        }
    }
    if (!cur.empty())
        out.push_back(cur);
    return out;
}
}

MoodProfile parseMood(const std::string& text)
{
    MoodProfile p;
    const auto tokens = tokenize(text);
    float br = 0, in = 0, dens = 0, sw = 0, rich = 0, bpm = 0;
    bool brSet = false, inSet = false;
    float mult = 1.0f;
    int pending = 0;

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        const std::string& t = tokens[i];

        if (isNumber(t) && i + 1 < tokens.size() && tokens[i + 1] == "bpm")
        {
            const int v = std::stoi(t);
            if (v >= 40 && v <= 220)
                p.bpmAbs = v;
            continue;
        }
        if (isOneOf(t, { "muy", "very", "super", "bien", "extra", "ultra", "demasiado", "mas", "more", "really", "hyper" }))
        {
            mult = 1.6f;
            pending = 2;
            continue;
        }
        if (isOneOf(t, { "poco", "algo", "slightly", "little", "bit", "ligeramente", "medio", "semi", "light" }))
        {
            mult = 0.5f;
            pending = 2;
            continue;
        }
        if (isOneOf(t, { "no", "not", "sin", "without", "nada", "never", "menos", "less" }))
        {
            mult = -1.0f;
            pending = 2;
            continue;
        }

        const Rule* hit = nullptr;
        for (const auto& r : kRules)
            if (startsWith(t, r.stem))
            {
                hit = &r;
                break;
            }

        if (hit != nullptr)
        {
            if (hit->mode != Auto)
                p.mode = hit->mode;
            else
            {
                br += hit->bright * mult;
                in += hit->inten * mult;
                dens += hit->dens * mult;
                sw += hit->swing * mult;
                rich += hit->rich * mult;
                bpm += (float) hit->bpm * mult;
                if (hit->bright != 0)
                    brSet = true;
                if (hit->inten != 0)
                    inSet = true;
            }
            std::string tag = (mult < 0 ? "no " : "") + t;
            if (std::find(p.tags.begin(), p.tags.end(), tag) == p.tags.end())
                p.tags.push_back(tag);
            mult = 1.0f;
            pending = 0;
        }
        else if (pending > 0 && --pending == 0)
            mult = 1.0f;
    }

    p.hasBrightness = brSet;
    p.hasIntensity = inSet;
    p.brightness = std::clamp(0.5f + br, 0.0f, 1.0f);
    p.intensity = std::clamp(0.5f + in, 0.0f, 1.0f);
    p.densityDelta = std::clamp(dens, -0.4f, 0.4f);
    p.swingDelta = std::clamp(sw, -0.5f, 0.3f);
    p.richness = std::clamp(rich, -0.5f, 1.0f);
    p.bpmDelta = (int) std::clamp(bpm, -20.0f, 20.0f);
    return p;
}
}
