#include "PluginEditor.h"
#include <cmath>

using namespace ui;

namespace
{
juce::String utf8(const char* s) { return juce::String::fromUTF8(s); }
}

MoodLookAndFeel::MoodLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, bg);
    setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    setColour(juce::TextButton::buttonOnColourId, accent);
    setColour(juce::TextButton::textColourOffId, muted);
    setColour(juce::TextButton::textColourOnId, bg);
    setColour(juce::ComboBox::textColourId, text);
    setColour(juce::ComboBox::backgroundColourId, field);
    setColour(juce::ComboBox::outlineColourId, line);
    setColour(juce::ComboBox::arrowColourId, muted);
    setColour(juce::PopupMenu::backgroundColourId, field);
    setColour(juce::PopupMenu::textColourId, text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, line);
    setColour(juce::PopupMenu::highlightedTextColourId, text);
    setColour(juce::TextEditor::backgroundColourId, field);
    setColour(juce::TextEditor::textColourId, text);
    setColour(juce::TextEditor::outlineColourId, line);
    setColour(juce::TextEditor::focusedOutlineColourId, accent);
    setColour(juce::TextEditor::highlightColourId, accent.withAlpha(0.35f));
    setColour(juce::TextEditor::highlightedTextColourId, text);
    setColour(juce::CaretComponent::caretColourId, accent);
    setColour(juce::Label::textColourId, text);
}

void MoodLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float, float,
                                       juce::Slider::SliderStyle, juce::Slider&)
{
    const float cy = (float) y + (float) h * 0.5f;
    g.setColour(line);
    g.fillRoundedRectangle((float) x, cy - 1.5f, (float) w, 3.0f, 1.5f);
    g.setColour(accent);
    g.fillRoundedRectangle((float) x, cy - 1.5f, juce::jmax(0.0f, pos - (float) x), 3.0f, 1.5f);
    g.setColour(bg);
    g.fillEllipse(pos - 8.0f, cy - 8.0f, 16.0f, 16.0f);
    g.setColour(accent);
    g.fillEllipse(pos - 6.0f, cy - 6.0f, 12.0f, 12.0f);
}

void MoodLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced(0.5f);
    const bool primary = (bool) b.getProperties().getWithDefault("primary", false);
    const float corner = primary ? 8.0f : r.getHeight() * 0.5f;

    if (primary)
    {
        g.setColour(down ? accent.darker(0.2f) : (highlighted ? accent.brighter(0.15f) : accent));
        g.fillRoundedRectangle(r, corner);
    }
    else if (b.getToggleState())
    {
        g.setColour(accent);
        g.fillRoundedRectangle(r, corner);
    }
    else
    {
        if (highlighted || down)
        {
            g.setColour(hover);
            g.fillRoundedRectangle(r, corner);
        }
        g.setColour(line);
        g.drawRoundedRectangle(r, corner, 1.0f);
    }
}

juce::Font MoodLookAndFeel::getTextButtonFont(juce::TextButton& b, int)
{
    const bool primary = (bool) b.getProperties().getWithDefault("primary", false);
    return font(primary ? 15.0f : 13.0f, primary || b.getToggleState());
}

void MoodLookAndFeel::drawComboBox(juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    auto r = juce::Rectangle<float>(0.0f, 0.0f, (float) w, (float) h).reduced(0.5f);
    g.setColour(field);
    g.fillRoundedRectangle(r, 8.0f);
    g.setColour(line);
    g.drawRoundedRectangle(r, 8.0f, 1.0f);
    juce::Path p;
    p.addTriangle((float) w - 20.0f, (float) h * 0.5f - 2.0f, (float) w - 10.0f, (float) h * 0.5f - 2.0f,
                  (float) w - 15.0f, (float) h * 0.5f + 3.0f);
    g.setColour(muted);
    g.fillPath(p);
}

void MoodLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(8, 1, box.getWidth() - 30, box.getHeight() - 2);
    label.setFont(getComboBoxFont(box));
}

juce::String MoodPad::quadrantName(float brightness, float intensity)
{
    const bool bright = brightness >= 0.5f, intense = intensity >= 0.5f;
    if (!bright && !intense) return "Nocturno";
    if (!bright && intense) return "Tenso";
    if (bright && !intense) return "Dulce";
    return utf8("Euf\xC3\xB3rico");
}

void MoodPad::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour(panel);
    g.fillRoundedRectangle(r, 10.0f);
    g.setColour(line);
    g.drawRoundedRectangle(r.reduced(0.5f), 10.0f, 1.0f);

    g.setColour(juce::Colour(0xff24242a));
    g.drawLine(r.getCentreX(), 1.0f, r.getCentreX(), r.getHeight() - 1.0f);
    g.drawLine(1.0f, r.getCentreY(), r.getWidth() - 1.0f, r.getCentreY());

    g.setColour(muted);
    g.setFont(11.0f);
    g.drawText("oscuro", 10, 0, 70, getHeight(), juce::Justification::centredLeft);
    g.drawText("brillante", getWidth() - 80, 0, 70, getHeight(), juce::Justification::centredRight);
    g.drawText("intenso", 0, 6, getWidth(), 14, juce::Justification::centredTop);
    g.drawText("calmado", 0, getHeight() - 20, getWidth(), 14, juce::Justification::centredBottom);

    const float b = apvts.getRawParameterValue("brightness")->load();
    const float in = apvts.getRawParameterValue("intensity")->load();
    const float px = juce::jlimit(10.0f, r.getWidth() - 10.0f, b * r.getWidth());
    const float py = juce::jlimit(10.0f, r.getHeight() - 10.0f, (1.0f - in) * r.getHeight());
    g.setColour(accent.withAlpha(0.16f));
    g.fillEllipse(px - 13.0f, py - 13.0f, 26.0f, 26.0f);
    g.setColour(accent);
    g.fillEllipse(px - 8.0f, py - 8.0f, 16.0f, 16.0f);
}

void MoodPad::setParam(const char* id, float value)
{
    if (auto* p = apvts.getParameter(id))
        p->setValueNotifyingHost(p->convertTo0to1(value));
}

void MoodPad::update(const juce::MouseEvent& e)
{
    const float b = juce::jlimit(0.0f, 1.0f, e.position.x / (float) getWidth());
    const float in = 1.0f - juce::jlimit(0.0f, 1.0f, e.position.y / (float) getHeight());
    setParam("brightness", b);
    setParam("intensity", in);
    repaint();
}

void MoodPad::mouseDown(const juce::MouseEvent& e)
{
    for (auto* id : { "brightness", "intensity" })
        if (auto* p = apvts.getParameter(id))
            p->beginChangeGesture();
    update(e);
}

void MoodPad::mouseDrag(const juce::MouseEvent& e) { update(e); }

void MoodPad::mouseUp(const juce::MouseEvent&)
{
    for (auto* id : { "brightness", "intensity" })
        if (auto* p = apvts.getParameter(id))
            p->endChangeGesture();
}

void LockButton::paintButton(juce::Graphics& g, bool highlighted, bool)
{
    const bool on = getToggleState();
    const auto c = on ? accent : (highlighted ? ui::text : muted);
    const float cx = (float) getWidth() * 0.5f, cy = (float) getHeight() * 0.5f;
    const float lift = on ? 0.0f : -3.5f;

    g.setColour(c);
    g.fillRoundedRectangle(cx - 7.0f, cy + 1.0f, 14.0f, 10.0f, 2.0f);

    juce::Path sh;
    sh.startNewSubPath(cx - 4.5f, cy + 1.0f);
    sh.lineTo(cx - 4.5f, cy - 3.0f + lift);
    sh.addCentredArc(cx, cy - 3.0f + lift, 4.5f, 4.5f, 0.0f, -juce::MathConstants<float>::halfPi,
                     juce::MathConstants<float>::halfPi, false);
    if (on)
        sh.lineTo(cx + 4.5f, cy + 1.0f);
    g.strokePath(sh, juce::PathStrokeType(1.7f));
}

void ClipView::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour(field);
    g.fillRoundedRectangle(r, 6.0f);

    const int bars = juce::jmax(1, proc.getClipBars());
    const int beats = bars * 4;
    for (int i = 1; i < beats; ++i)
    {
        const float x = r.getWidth() * (float) i / (float) beats;
        g.setColour(juce::Colour(i % 4 == 0 ? 0xff30303a : 0xff222228));
        g.drawLine(x, 2.0f, x, r.getHeight() - 2.0f, 1.0f);
    }

    const auto notes = proc.getNotes(track);
    if (!notes.empty())
    {
        int lo = 127, hi = 0;
        for (const auto& n : notes)
        {
            lo = juce::jmin(lo, n.pitch);
            hi = juce::jmax(hi, n.pitch);
        }
        const float range = (float) juce::jmax(12, hi - lo);
        const float total = (float) (bars * 4 * moodgen::kPPQ);
        for (const auto& n : notes)
        {
            const float x = (float) n.startTick / total * r.getWidth();
            const float w = juce::jmax(2.0f, (float) n.lengthTicks / total * r.getWidth() - 1.0f);
            const float y = 4.0f + (1.0f - (float) (n.pitch - lo) / range) * (r.getHeight() - 11.0f);
            g.setColour(accent.withAlpha(0.4f + 0.55f * (float) n.velocity / 127.0f));
            g.fillRoundedRectangle(x, y, w, 3.0f, 1.0f);
        }
    }

    const double ppq = proc.getPreviewPositionQuarters();
    if (proc.isPreviewing() || ppq > 0.0)
    {
        const double len = (double) beats;
        const float x = (float) (std::fmod(ppq, len) / len) * r.getWidth();
        g.setColour(text.withAlpha(0.55f));
        g.drawLine(x, 1.0f, x, r.getHeight() - 1.0f, 1.5f);
    }
}

void ClipView::mouseDrag(const juce::MouseEvent& e)
{
    if (dragged || e.getDistanceFromDragStart() < 6)
        return;
    dragged = true;
    const auto f = proc.exportMidi(track);
    if (f.existsAsFile())
        juce::DragAndDropContainer::performExternalDragDropOfFiles({ f.getFullPathName() }, false, this);
}

void DragAllBar::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced(0.5f);
    g.setColour(accent.withAlpha(0.08f));
    g.fillRoundedRectangle(r, 8.0f);
    g.setColour(accent.withAlpha(0.6f));
    g.drawRoundedRectangle(r, 8.0f, 1.0f);
    g.setColour(accent);
    g.setFont(font(14.0f, true));
    g.drawText("Arrastrar todo a FL Studio", getLocalBounds(), juce::Justification::centred);
}

void DragAllBar::mouseDrag(const juce::MouseEvent& e)
{
    if (dragged || e.getDistanceFromDragStart() < 6)
        return;
    dragged = true;
    const auto f = proc.exportMidi(-1);
    if (f.existsAsFile())
        juce::DragAndDropContainer::performExternalDragDropOfFiles({ f.getFullPathName() }, false, this);
}

juce::Label* MoodGenEditor::makeLabel(const juce::String& s, float size, juce::Colour c)
{
    auto* l = labels.add(new juce::Label());
    l->setText(s, juce::dontSendNotification);
    l->setFont(font(size));
    l->setColour(juce::Label::textColourId, c);
    l->setBorderSize(juce::BorderSize<int>(0));
    l->setInterceptsMouseClicks(false, false);
    addAndMakeVisible(l);
    return l;
}

void MoodGenEditor::setParam(const char* id, float value)
{
    if (auto* p = proc.apvts.getParameter(id))
        p->setValueNotifyingHost(p->convertTo0to1(value));
}

MoodGenEditor::MoodGenEditor(MoodGenProcessor& p)
    : juce::AudioProcessorEditor(&p), proc(p), pad(p.apvts), dragAll(p)
{
    setLookAndFeel(&laf);
    setSize(460, 860);

    genreLabel = makeLabel(utf8("G\xC3\xA9nero"));
    moodLabel = makeLabel("Mood");
    keyLabel = makeLabel("Tono");
    modeLabel = makeLabel("Modo");
    bpmLabel = makeLabel("BPM");
    barsLabel = makeLabel("Compases");
    densityLabel = makeLabel("Densidad");
    variationLabel = makeLabel(utf8("Variaci\xC3\xB3n"));
    hintLabel = makeLabel("Arrastra cada pista al Piano Roll de FL Studio", 12.0f, muted);
    hintLabel->setJustificationType(juce::Justification::centred);

    for (int i = 0; i < 5; ++i)
    {
        genreBtns[(size_t) i].setButtonText(MoodGenProcessor::genreNames()[i]);
        genreBtns[(size_t) i].onClick = [this, i] { selectGenre(i); };
        addAndMakeVisible(genreBtns[(size_t) i]);
    }

    simpleBtn.onClick = [this] { setAdvanced(false); };
    advBtn.onClick = [this] { setAdvanced(true); };
    addAndMakeVisible(simpleBtn);
    addAndMakeVisible(advBtn);

    addAndMakeVisible(pad);

    moodName.setFont(font(12.0f));
    moodName.setColour(juce::Label::textColourId, accent);
    addAndMakeVisible(moodName);

    moodBox.setMultiLine(true, true);
    moodBox.setReturnKeyStartsNewLine(false);
    moodBox.setFont(font(14.0f));
    moodBox.setTextToShowWhenEmpty("Describe el mood con tus palabras: sensual, nocturno, muy lento, jazzy, 72 bpm...", muted);
    moodBox.setText(proc.getMoodText(), false);
    moodBox.onTextChange = [this] {
        proc.setMoodText(moodBox.getText());
        refreshMoodTags();
    };
    moodBox.onReturnKey = [this] { generate(); };
    addAndMakeVisible(moodBox);

    moodTags.setFont(font(12.0f));
    moodTags.setColour(juce::Label::textColourId, accent);
    moodTags.setJustificationType(juce::Justification::topLeft);
    moodTags.setBorderSize(juce::BorderSize<int>(0));
    addAndMakeVisible(moodTags);

    const juce::StringArray words { "sensual", "nocturno", "feliz", "chill", utf8("en\xC3\xA9rgico") };
    for (int i = 0; i < 5; ++i)
    {
        exampleBtns[(size_t) i].setButtonText(words[i]);
        exampleBtns[(size_t) i].onClick = [this, w = words[i]] { appendWord(w); };
        addAndMakeVisible(exampleBtns[(size_t) i]);
    }

    keyBox.addItemList(MoodGenProcessor::keyNames(), 1);
    modeBox.addItemList(MoodGenProcessor::modeNames(), 1);
    barsBox.addItemList(MoodGenProcessor::barNames(), 1);
    for (auto* c : { &keyBox, &modeBox, &barsBox })
        addAndMakeVisible(c);
    keyAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, "key", keyBox);
    modeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, "mode", modeBox);
    barsAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, "bars", barsBox);

    for (auto* s : { &bpmSlider, &densitySlider, &variationSlider })
    {
        s->setSliderStyle(juce::Slider::LinearHorizontal);
        s->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(s);
    }
    bpmAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.apvts, "bpm", bpmSlider);
    densityAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.apvts, "density", densitySlider);
    variationAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.apvts, "variation", variationSlider);

    for (auto* l : { &bpmValue, &densityValue, &variationValue })
    {
        l->setFont(font(12.0f));
        l->setColour(juce::Label::textColourId, text);
        l->setJustificationType(juce::Justification::centredRight);
        l->setBorderSize(juce::BorderSize<int>(0));
        addAndMakeVisible(l);
    }

    generateBtn.getProperties().set("primary", true);
    generateBtn.setColour(juce::TextButton::textColourOffId, bg);
    generateBtn.onClick = [this] { generate(); };
    addAndMakeVisible(generateBtn);

    previewBtn.onClick = [this] {
        proc.setPreviewing(!proc.isPreviewing());
        previewBtn.setToggleState(proc.isPreviewing(), juce::dontSendNotification);
        previewBtn.setButtonText(proc.isPreviewing() ? "Detener" : "Escuchar");
    };
    addAndMakeVisible(previewBtn);

    for (int t = 0; t < 4; ++t)
    {
        clipViews[(size_t) t] = std::make_unique<ClipView>(proc, t);
        addAndMakeVisible(*clipViews[(size_t) t]);
        lockBtns[(size_t) t].setToggleState(proc.isLocked(t), juce::dontSendNotification);
        lockBtns[(size_t) t].onClick = [this, t] { proc.setLocked(t, lockBtns[(size_t) t].getToggleState()); };
        addAndMakeVisible(lockBtns[(size_t) t]);
        trackLabels[(size_t) t] = makeLabel(utf8(moodgen::trackName(t)), 13.0f, juce::Colour(0xffc4c4ca));
    }

    addAndMakeVisible(dragAll);

    applyVisibility(proc.apvts.getRawParameterValue("advanced")->load() > 0.5f);
    refreshMoodTags();
    timerCallback();
    startTimerHz(20);
}

MoodGenEditor::~MoodGenEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void MoodGenEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    g.setColour(text);
    g.setFont(font(17.0f, true));
    g.drawText("moodgen", 16, 14, 100, 24, juce::Justification::centredLeft);
    g.setColour(accent);
    g.drawText(".", 16 + 83, 14, 12, 24, juce::Justification::centredLeft);
    g.setColour(muted);
    g.setFont(font(12.0f));
    g.drawText("VST3", getWidth() - 16 - 60, 14, 60, 24, juce::Justification::centredRight);
}

void MoodGenEditor::resized()
{
    genreLabel->setBounds(16, 52, 200, 14);
    const int widths[5] = { 64, 92, 68, 56, 84 };
    int x = 16;
    for (int i = 0; i < 5; ++i)
    {
        genreBtns[(size_t) i].setBounds(x, 70, widths[i], 28);
        x += widths[i] + 8;
    }

    moodLabel->setBounds(16, 118, 60, 14);
    moodName.setBounds(64, 118, 190, 14);
    simpleBtn.setBounds(268, 112, 84, 26);
    advBtn.setBounds(360, 112, 84, 26);

    const juce::Rectangle<int> region(16, 146, 428, 146);
    pad.setBounds(region);
    moodBox.setBounds(region.getX(), region.getY(), region.getWidth(), 66);
    moodTags.setBounds(region.getX(), region.getY() + 72, region.getWidth(), 34);
    const int ew[5] = { 70, 76, 56, 56, 76 };
    int ex = region.getX();
    for (int i = 0; i < 5; ++i)
    {
        exampleBtns[(size_t) i].setBounds(ex, region.getY() + 116, ew[i], 26);
        ex += ew[i] + 6;
    }

    keyLabel->setBounds(16, 310, 200, 14);
    keyBox.setBounds(16, 326, 206, 28);
    modeLabel->setBounds(238, 310, 200, 14);
    modeBox.setBounds(238, 326, 206, 28);

    bpmLabel->setBounds(16, 366, 120, 14);
    bpmValue.setBounds(150, 366, 72, 14);
    bpmSlider.setBounds(16, 384, 206, 28);
    barsLabel->setBounds(238, 366, 200, 14);
    barsBox.setBounds(238, 382, 206, 28);

    densityLabel->setBounds(16, 422, 120, 14);
    densityValue.setBounds(150, 422, 72, 14);
    densitySlider.setBounds(16, 440, 206, 28);
    variationLabel->setBounds(238, 422, 120, 14);
    variationValue.setBounds(372, 422, 72, 14);
    variationSlider.setBounds(238, 440, 206, 28);

    generateBtn.setBounds(16, 492, 316, 44);
    previewBtn.setBounds(344, 492, 100, 44);

    for (int t = 0; t < 4; ++t)
    {
        const int y = 558 + t * 56;
        trackLabels[(size_t) t]->setBounds(16, y + 18, 70, 18);
        clipViews[(size_t) t]->setBounds(92, y + 4, 310, 48);
        lockBtns[(size_t) t].setBounds(412, y + 16, 32, 24);
    }

    hintLabel->setBounds(16, 790, 428, 14);
    dragAll.setBounds(16, 810, 428, 38);
}

void MoodGenEditor::applyVisibility(bool adv)
{
    advancedShown = adv;
    pad.setVisible(!adv);
    moodBox.setVisible(adv);
    moodTags.setVisible(adv);
    for (auto& b : exampleBtns)
        b.setVisible(adv);
    simpleBtn.setToggleState(!adv, juce::dontSendNotification);
    advBtn.setToggleState(adv, juce::dontSendNotification);
    repaint();
}

void MoodGenEditor::setAdvanced(bool on)
{
    setParam("advanced", on ? 1.0f : 0.0f);
    applyVisibility(on);
    if (on)
        moodBox.grabKeyboardFocus();
}

void MoodGenEditor::selectGenre(int index)
{
    setParam("genre", (float) index);
    setParam("bpm", (float) moodgen::defaultBpm(index));
    generate();
}

void MoodGenEditor::generate()
{
    proc.generateClips();
    for (auto& v : clipViews)
        v->repaint();
}

void MoodGenEditor::appendWord(const juce::String& word)
{
    const auto t = moodBox.getText().trim();
    moodBox.setText(t.isEmpty() ? word : t + ", " + word, true);
}

void MoodGenEditor::refreshMoodTags()
{
    const auto m = moodgen::parseMood(moodBox.getText().toStdString());
    juce::StringArray parts;
    for (const auto& t : m.tags)
        parts.add(juce::String::fromUTF8(t.c_str()));
    if (m.hasBrightness)
        parts.add("brillo " + juce::String(juce::roundToInt(m.brightness * 100.0f)) + "%");
    if (m.hasIntensity)
        parts.add("intensidad " + juce::String(juce::roundToInt(m.intensity * 100.0f)) + "%");
    if (m.mode != moodgen::Auto)
        parts.add("modo " + MoodGenProcessor::modeNames()[m.mode].toLowerCase());
    if (m.bpmAbs > 0)
        parts.add(juce::String(m.bpmAbs) + " bpm");

    if (moodBox.getText().trim().isEmpty())
        moodTags.setText("Escribe como quieras: el plugin entiende espa" + utf8("\xC3\xB1") + "ol e ingl" + utf8("\xC3\xA9") + "s.",
                         juce::dontSendNotification);
    else if (parts.isEmpty())
        moodTags.setText("No reconoci palabras de mood. Prueba: oscuro, dulce, tenso, lento, jazzy...", juce::dontSendNotification);
    else
        moodTags.setText("Entendido: " + parts.joinIntoString("  /  "), juce::dontSendNotification);
}

void MoodGenEditor::timerCallback()
{
    auto val = [this](const char* id) { return proc.apvts.getRawParameterValue(id)->load(); };

    const int genre = (int) val("genre");
    if (genre != lastGenre)
    {
        lastGenre = genre;
        for (int i = 0; i < 5; ++i)
            genreBtns[(size_t) i].setToggleState(i == genre, juce::dontSendNotification);
    }

    const bool adv = val("advanced") > 0.5f;
    if (adv != advancedShown)
        applyVisibility(adv);

    moodName.setText(adv ? juce::String("Libre") : MoodPad::quadrantName(val("brightness"), val("intensity")),
                     juce::dontSendNotification);
    bpmValue.setText(juce::String((int) val("bpm")), juce::dontSendNotification);
    densityValue.setText(juce::String(juce::roundToInt(val("density") * 100.0f)) + "%", juce::dontSendNotification);
    variationValue.setText(juce::String(juce::roundToInt(val("variation") * 100.0f)) + "%", juce::dontSendNotification);

    const bool prev = proc.isPreviewing();
    previewBtn.setToggleState(prev, juce::dontSendNotification);
    previewBtn.setButtonText(prev ? "Detener" : "Escuchar");

    for (int t = 0; t < 4; ++t)
        lockBtns[(size_t) t].setToggleState(proc.isLocked(t), juce::dontSendNotification);

    if (!adv)
        pad.repaint();

    const double ppq = proc.getPreviewPositionQuarters();
    if (ppq != lastPpq)
    {
        lastPpq = ppq;
        for (auto& v : clipViews)
            v->repaint();
    }
}
