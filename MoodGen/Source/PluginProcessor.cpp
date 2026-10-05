#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

using namespace moodgen;

namespace
{
juce::String utf8(const char* s) { return juce::String::fromUTF8(s); }

struct PreviewSound : juce::SynthesiserSound
{
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

struct PreviewVoice : juce::SynthesiserVoice
{
    bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<PreviewSound*>(s) != nullptr; }

    void startNote(int note, float velocity, juce::SynthesiserSound*, int) override
    {
        level = velocity * 0.22f;
        t = 0.0;
        phase = 0.0;
        env = 0.0f;
        releasing = false;
        drum = isPlayingChannel(10);
        noteNumber = note;
        if (drum)
            kind = note == 36 ? 0 : (note == 38 || note == 39 ? 1 : (note == 37 ? 4 : 2));
        else
            delta = juce::MidiMessage::getMidiNoteInHertz(note) / getSampleRate();
    }

    void stopNote(float, bool allowTailOff) override
    {
        if (drum)
            return;
        if (allowTailOff)
            releasing = true;
        else
            clearCurrentNote();
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>& out, int start, int num) override
    {
        const double sr = getSampleRate();
        for (int i = 0; i < num; ++i)
        {
            float s = 0.0f;
            if (drum)
            {
                const float e = std::exp((float) (-t * decayRate()));
                const float noise = rng.nextFloat() * 2.0f - 1.0f;
                switch (kind)
                {
                    case 0:
                    {
                        const double f = 48.0 + 120.0 * std::exp(-t * 28.0);
                        phase += f / sr;
                        s = std::sin((float) (phase * juce::MathConstants<double>::twoPi)) * e * 1.6f;
                        break;
                    }
                    case 1:
                    {
                        phase += 190.0 / sr;
                        s = (noise * 0.8f + std::sin((float) (phase * juce::MathConstants<double>::twoPi)) * 0.4f) * e;
                        break;
                    }
                    case 4:
                    {
                        phase += 820.0 / sr;
                        s = std::sin((float) (phase * juce::MathConstants<double>::twoPi)) * e * 0.9f;
                        break;
                    }
                    default:
                    {
                        const float hp = noise - lastNoise;
                        lastNoise = noise;
                        s = hp * e * 0.7f;
                        break;
                    }
                }
                t += 1.0 / sr;
                if (e < 0.0008f)
                {
                    clearCurrentNote();
                    break;
                }
            }
            else
            {
                phase += delta;
                if (phase >= 1.0)
                    phase -= 1.0;
                const float saw = (float) (2.0 * phase - 1.0);
                const float sine = std::sin((float) (phase * juce::MathConstants<double>::twoPi));
                s = (0.45f * saw + 0.55f * sine);
                const float target = releasing ? 0.0f : 1.0f;
                const float coef = releasing ? 0.0016f : 0.004f;
                env += (target - env) * coef * 3.0f;
                s *= env;
                if (releasing && env < 0.001f)
                {
                    clearCurrentNote();
                    break;
                }
            }

            s *= level;
            for (int ch = 0; ch < out.getNumChannels(); ++ch)
                out.addSample(ch, start + i, s);
        }
    }

private:
    double decayRate() const
    {
        switch (kind)
        {
            case 0: return 9.0;
            case 1: return 17.0;
            case 4: return 70.0;
            default: return noteNumber == 46 ? 18.0 : (noteNumber == 49 ? 2.5 : 55.0);
        }
    }

    juce::Random rng;
    float level = 0.2f, env = 0.0f, lastNoise = 0.0f;
    double t = 0.0, phase = 0.0, delta = 0.0;
    bool releasing = false, drum = false;
    int kind = 0, noteNumber = 60;
};
}

PreviewSynth::PreviewSynth()
{
    for (int i = 0; i < 24; ++i)
        synth.addVoice(new PreviewVoice());
    synth.addSound(new PreviewSound());
}

void PreviewSynth::prepare(double sr) { synth.setCurrentPlaybackSampleRate(sr); }

void PreviewSynth::render(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi)
{
    synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());
}

void PreviewSynth::allNotesOff() { synth.allNotesOff(0, true); }

const juce::StringArray& MoodGenProcessor::genreNames()
{
    static const juce::StringArray n { "R&B", utf8("Reguet\xC3\xB3n"), "House", "Pop", "Jazz soul" };
    return n;
}

const juce::StringArray& MoodGenProcessor::keyNames()
{
    static const juce::StringArray n { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return n;
}

const juce::StringArray& MoodGenProcessor::modeNames()
{
    static const juce::StringArray n { "Auto", "Menor", "Mayor", utf8("D\xC3\xB3rico"), "Frigio", "Mixolidio", "Lidio" };
    return n;
}

const juce::StringArray& MoodGenProcessor::barNames()
{
    static const juce::StringArray n { "2", "4", "8" };
    return n;
}

juce::AudioProcessorValueTreeState::ParameterLayout MoodGenProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    l.add(std::make_unique<AudioParameterChoice>(ParameterID { "genre", 1 }, "Genero", genreNames(), 0));
    l.add(std::make_unique<AudioParameterChoice>(ParameterID { "key", 1 }, "Tono", keyNames(), 0));
    l.add(std::make_unique<AudioParameterChoice>(ParameterID { "mode", 1 }, "Modo", modeNames(), 0));
    l.add(std::make_unique<AudioParameterChoice>(ParameterID { "bars", 1 }, "Compases", barNames(), 1));
    l.add(std::make_unique<AudioParameterFloat>(ParameterID { "bpm", 1 }, "BPM", NormalisableRange<float>(60.0f, 180.0f, 1.0f), 80.0f));
    l.add(std::make_unique<AudioParameterFloat>(ParameterID { "brightness", 1 }, "Brillo", NormalisableRange<float>(0.0f, 1.0f), 0.3f));
    l.add(std::make_unique<AudioParameterFloat>(ParameterID { "intensity", 1 }, "Intensidad", NormalisableRange<float>(0.0f, 1.0f), 0.35f));
    l.add(std::make_unique<AudioParameterFloat>(ParameterID { "density", 1 }, "Densidad", NormalisableRange<float>(0.0f, 1.0f), 0.55f));
    l.add(std::make_unique<AudioParameterFloat>(ParameterID { "variation", 1 }, "Variacion", NormalisableRange<float>(0.0f, 1.0f), 0.4f));
    l.add(std::make_unique<AudioParameterBool>(ParameterID { "advanced", 1 }, "Avanzado", false));
    return l;
}

MoodGenProcessor::MoodGenProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "MOODGEN", createLayout())
{
    for (auto& l : locked)
        l.store(false);
    apvts.state.setProperty("moodText", "", nullptr);
    generateClips();
}

void MoodGenProcessor::prepareToPlay(double sr, int)
{
    sampleRateHz = sr;
    preview.prepare(sr);
}

bool MoodGenProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

juce::AudioProcessorEditor* MoodGenProcessor::createEditor() { return new MoodGenEditor(*this); }

juce::String MoodGenProcessor::getMoodText() const { return apvts.state.getProperty("moodText").toString(); }

void MoodGenProcessor::setMoodText(const juce::String& text) { apvts.state.setProperty("moodText", text, nullptr); }

Params MoodGenProcessor::currentParams() const
{
    auto get = [this](const char* id) { return apvts.getRawParameterValue(id)->load(); };
    Params p;
    p.genre = (int) get("genre");
    p.root = (int) get("key");
    p.mode = (int) get("mode");
    p.brightness = get("brightness");
    p.intensity = get("intensity");
    p.density = get("density");
    p.variation = get("variation");
    p.bars = 2 << (int) get("bars");
    p.harmonySeed = harmonySeed;
    if (get("advanced") > 0.5f)
        p.profile = parseMood(getMoodText().toStdString());
    return p;
}

void MoodGenProcessor::generateClips()
{
    juce::Random rnd;
    const unsigned base = (unsigned) rnd.nextInt();

    auto p = currentParams();
    const bool barsChanged = haveClips && p.bars != clipBars.load();
    const bool chordsLocked = locked[Chords].load() && !barsChanged && haveClips;
    if (!chordsLocked)
    {
        harmonySeed = base ^ 0x9e3779b9u;
        p.harmonySeed = harmonySeed;
    }

    if (apvts.getRawParameterValue("advanced")->load() > 0.5f)
    {
        const int h = getMoodText().hashCode();
        if (h != lastMoodHash)
        {
            lastMoodHash = h;
            int bpm = 0;
            if (p.profile.bpmAbs > 0)
                bpm = p.profile.bpmAbs;
            else if (p.profile.bpmDelta != 0)
                bpm = defaultBpm(p.genre) + p.profile.bpmDelta;
            if (bpm > 0)
                if (auto* prm = apvts.getParameter("bpm"))
                    prm->setValueNotifyingHost(prm->convertTo0to1((float) juce::jlimit(60, 180, bpm)));
        }
    }

    std::array<std::vector<Note>, NumTracks> fresh;
    std::array<bool, NumTracks> doIt {};
    for (int t = 0; t < NumTracks; ++t)
    {
        doIt[(size_t) t] = barsChanged || !haveClips || !locked[(size_t) t].load();
        if (doIt[(size_t) t])
            fresh[(size_t) t] = generate(p, t, base + (unsigned) t * 131u);
    }

    {
        const juce::ScopedLock sl(clipLock);
        for (int t = 0; t < NumTracks; ++t)
            if (doIt[(size_t) t])
                clips[(size_t) t] = std::move(fresh[(size_t) t]);
        clipBars.store(p.bars);
    }
    haveClips = true;
}

std::vector<Note> MoodGenProcessor::getNotes(int track) const
{
    const juce::ScopedLock sl(clipLock);
    return clips[(size_t) juce::jlimit(0, (int) NumTracks - 1, track)];
}

void MoodGenProcessor::sendAllNotesOff(juce::MidiBuffer& midi, int sample)
{
    for (int ch : { 1, 2, 3, 10 })
        midi.addEvent(juce::MidiMessage::allNotesOff(ch), sample);
}

void MoodGenProcessor::emitMidi(juce::MidiBuffer& midi, int n, double ppq, double bpm)
{
    const juce::ScopedTryLock sl(clipLock);
    if (!sl.isLocked())
        return;

    const double lenQ = (double) clipBars.load() * 4.0;
    const double perSample = bpm / 60.0 / sampleRateHz;
    const double span = (double) n * perSample;
    double p0 = std::fmod(ppq, lenQ);
    if (p0 < 0.0)
        p0 += lenQ;

    auto offsetOf = [&](double q, int& offset) {
        q = std::fmod(q, lenQ);
        double d = q - p0;
        if (d < -1e-9)
            d += lenQ;
        if (d < 0.0)
            d = 0.0;
        if (d >= span)
            return false;
        offset = juce::jlimit(0, n - 1, (int) (d / perSample));
        return true;
    };

    for (int pass = 0; pass < 2; ++pass)
        for (int t = 0; t < NumTracks; ++t)
        {
            const int ch = (t == Drums) ? 10 : t + 1;
            for (const auto& note : clips[(size_t) t])
            {
                const double s = (double) note.startTick / kPPQ;
                const double e = (double) (note.startTick + note.lengthTicks) / kPPQ;
                int off = 0;
                if (pass == 0 && offsetOf(e, off))
                    midi.addEvent(juce::MidiMessage::noteOff(ch, note.pitch), off);
                if (pass == 1 && offsetOf(s, off))
                    midi.addEvent(juce::MidiMessage::noteOn(ch, note.pitch, (juce::uint8) note.velocity), off);
            }
        }
}

void MoodGenProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    midi.clear();

    const int n = buffer.getNumSamples();
    bool hostPlaying = false;
    double ppq = 0.0;
    double bpm = apvts.getRawParameterValue("bpm")->load();

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm())
                bpm = *b;
            if (auto q = pos->getPpqPosition(); q.hasValue() && pos->getIsPlaying())
            {
                hostPlaying = true;
                ppq = *q;
            }
        }

    const bool wantPreview = previewing.load();
    bool active = hostPlaying || wantPreview;
    if (wantPreview && !hostPlaying)
        ppq = internalPpq;
    if (bpm < 20.0)
        bpm = 80.0;

    if (!active)
    {
        if (wasActive)
        {
            sendAllNotesOff(midi, 0);
            preview.allNotesOff();
            wasActive = false;
        }
        expectedPpq = -1.0;
        internalPpq = 0.0;
        displayPpq.store(0.0);
        return;
    }

    const double perSample = bpm / 60.0 / sampleRateHz;
    if (expectedPpq >= 0.0 && std::abs(ppq - expectedPpq) > perSample * 2.0 + 1e-6)
    {
        sendAllNotesOff(midi, 0);
        preview.allNotesOff();
    }

    emitMidi(midi, n, ppq, bpm);

    if (wantPreview)
        preview.render(buffer, midi);

    const double span = (double) n * perSample;
    expectedPpq = ppq + span;
    if (wantPreview && !hostPlaying)
        internalPpq += span;
    displayPpq.store(ppq);
    wasActive = true;
}

juce::File MoodGenProcessor::exportMidi(int track)
{
    auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("MoodGen");
    dir.createDirectory();

    const auto p = currentParams();
    const int bpm = (int) apvts.getRawParameterValue("bpm")->load();
    const juce::String genre = juce::String(genreNames()[p.genre]).retainCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ")
                                   .replace(" ", "");
    const juce::String key = keyNames()[p.root].replace("#", "s");
    const juce::String trackLabel = track < 0 ? juce::String("Todo")
                                              : juce::String(trackName(track)).retainCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    auto file = dir.getChildFile("MoodGen_" + genre + "_" + key + "_" + juce::String(bpm) + "bpm_" + trackLabel + ".mid");

    juce::MidiFile mf;
    mf.setTicksPerQuarterNote(kPPQ);

    auto addTrack = [&](int t, bool withMeta) {
        juce::MidiMessageSequence seq;
        if (withMeta)
        {
            seq.addEvent(juce::MidiMessage::tempoMetaEvent(60000000 / juce::jmax(1, bpm)), 0.0);
            seq.addEvent(juce::MidiMessage::timeSignatureMetaEvent(4, 4), 0.0);
        }
        seq.addEvent(juce::MidiMessage::textMetaEvent(3, juce::String::fromUTF8(trackName(t))), 0.0);
        const int ch = (t == Drums) ? 10 : t + 1;
        for (const auto& note : getNotes(t))
        {
            seq.addEvent(juce::MidiMessage::noteOn(ch, note.pitch, (juce::uint8) note.velocity), (double) note.startTick);
            seq.addEvent(juce::MidiMessage::noteOff(ch, note.pitch), (double) (note.startTick + note.lengthTicks));
        }
        seq.updateMatchedPairs();
        mf.addTrack(seq);
    };

    if (track < 0)
        for (int t = 0; t < NumTracks; ++t)
            addTrack(t, t == 0);
    else
        addTrack(track, true);

    file.deleteFile();
    if (auto out = file.createOutputStream())
        mf.writeTo(*out);
    return file;
}

void MoodGenProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty("harmonySeed", (juce::int64) harmonySeed, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void MoodGenProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml(*xml);
            apvts.replaceState(tree);
            harmonySeed = (unsigned) (juce::int64) tree.getProperty("harmonySeed", 1);
            lastMoodHash = getMoodText().hashCode();
            for (auto& l : locked)
                l.store(false);
            generateClips();
        }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MoodGenProcessor(); }
