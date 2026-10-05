#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include "PluginProcessor.h"

namespace ui
{
const juce::Colour bg(0xff0e0e10), panel(0xff131316), field(0xff17171a), line(0xff34343a),
    hover(0xff1c1c20), text(0xffececec), muted(0xff7c7c84), accent(0xff9fe8d2);

inline juce::Font font(float h, bool bold = false)
{
    return juce::Font(juce::FontOptions(h, bold ? juce::Font::bold : juce::Font::plain));
}
}

class MoodLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MoodLookAndFeel();
    void drawLinearSlider(juce::Graphics&, int x, int y, int w, int h, float pos, float minPos, float maxPos,
                          juce::Slider::SliderStyle, juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool highlighted, bool down) override;
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    void drawComboBox(juce::Graphics&, int w, int h, bool, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override { return ui::font(13.0f); }
    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;
};

class MoodPad : public juce::Component
{
public:
    explicit MoodPad(juce::AudioProcessorValueTreeState& s) : apvts(s) {}
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    static juce::String quadrantName(float brightness, float intensity);

private:
    void update(const juce::MouseEvent&);
    void setParam(const char* id, float value);
    juce::AudioProcessorValueTreeState& apvts;
};

class LockButton : public juce::Button
{
public:
    LockButton() : juce::Button("lock") { setClickingTogglesState(true); }
    void paintButton(juce::Graphics&, bool highlighted, bool down) override;
};

class ClipView : public juce::Component
{
public:
    ClipView(MoodGenProcessor& p, int t) : proc(p), track(t) {}
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override { dragged = false; }
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseEnter(const juce::MouseEvent&) override { setMouseCursor(juce::MouseCursor::DraggingHandCursor); }

private:
    MoodGenProcessor& proc;
    int track;
    bool dragged = false;
};

class DragAllBar : public juce::Component
{
public:
    explicit DragAllBar(MoodGenProcessor& p) : proc(p) {}
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override { dragged = false; }
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseEnter(const juce::MouseEvent&) override { setMouseCursor(juce::MouseCursor::DraggingHandCursor); }

private:
    MoodGenProcessor& proc;
    bool dragged = false;
};

class MoodGenEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit MoodGenEditor(MoodGenProcessor&);
    ~MoodGenEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void selectGenre(int index);
    void setAdvanced(bool on);
    void applyVisibility(bool advanced);
    void generate();
    void refreshMoodTags();
    void appendWord(const juce::String& word);
    void setParam(const char* id, float value);
    juce::Label* makeLabel(const juce::String& s, float size = 12.0f, juce::Colour c = ui::muted);

    MoodGenProcessor& proc;
    MoodLookAndFeel laf;

    std::array<juce::TextButton, 5> genreBtns;
    juce::TextButton simpleBtn { "Simple" }, advBtn { "Avanzado" };
    MoodPad pad;
    juce::TextEditor moodBox;
    juce::Label moodTags, moodName, bpmValue, densityValue, variationValue;
    std::array<juce::TextButton, 5> exampleBtns;

    juce::ComboBox keyBox, modeBox, barsBox;
    juce::Slider bpmSlider, densitySlider, variationSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyAtt, modeAtt, barsAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bpmAtt, densityAtt, variationAtt;

    juce::TextButton generateBtn { "Generar" }, previewBtn { "Escuchar" };
    std::array<std::unique_ptr<ClipView>, 4> clipViews;
    std::array<LockButton, 4> lockBtns;
    DragAllBar dragAll;

    juce::OwnedArray<juce::Label> labels;
    juce::Label *genreLabel = nullptr, *moodLabel = nullptr, *keyLabel = nullptr, *modeLabel = nullptr, *bpmLabel = nullptr,
                *barsLabel = nullptr, *densityLabel = nullptr, *variationLabel = nullptr, *hintLabel = nullptr;
    std::array<juce::Label*, 4> trackLabels {};
    bool advancedShown = true;
    int lastGenre = -1;
    double lastPpq = -1.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MoodGenEditor)
};
