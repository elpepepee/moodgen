#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>
#include "MoodEngine.h"

class PreviewSynth
{
public:
    PreviewSynth();
    void prepare(double sampleRate);
    void render(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi);
    void allNotesOff();

private:
    juce::Synthesiser synth;
};

class MoodGenProcessor : public juce::AudioProcessor
{
public:
    MoodGenProcessor();
    ~MoodGenProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "MoodGen"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    static const juce::StringArray& genreNames();
    static const juce::StringArray& keyNames();
    static const juce::StringArray& modeNames();
    static const juce::StringArray& barNames();

    juce::AudioProcessorValueTreeState apvts;

    moodgen::Params currentParams() const;
    juce::String getMoodText() const;
    void setMoodText(const juce::String& text);

    void generateClips();
    std::vector<moodgen::Note> getNotes(int track) const;
    int getClipBars() const { return clipBars.load(); }

    void setLocked(int track, bool shouldLock) { locked[(size_t) track].store(shouldLock); }
    bool isLocked(int track) const { return locked[(size_t) track].load(); }

    void setPreviewing(bool on) { previewing.store(on); }
    bool isPreviewing() const { return previewing.load(); }
    double getPreviewPositionQuarters() const { return displayPpq.load(); }

    juce::File exportMidi(int track);

private:
    void emitMidi(juce::MidiBuffer& midi, int numSamples, double ppq, double bpm);
    void sendAllNotesOff(juce::MidiBuffer& midi, int sample);

    mutable juce::CriticalSection clipLock;
    std::array<std::vector<moodgen::Note>, moodgen::NumTracks> clips;
    std::array<std::atomic<bool>, moodgen::NumTracks> locked;
    std::atomic<int> clipBars { 4 };
    std::atomic<bool> previewing { false };
    std::atomic<double> displayPpq { 0.0 };

    unsigned harmonySeed = 1;
    int lastMoodHash = 0;
    bool haveClips = false;

    PreviewSynth preview;
    double sampleRateHz = 44100.0;
    double internalPpq = 0.0;
    double expectedPpq = -1.0;
    bool wasActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MoodGenProcessor)
};
