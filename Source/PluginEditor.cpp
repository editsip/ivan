#include "PluginEditor.h"

IvanAudioProcessorEditor::IvanAudioProcessorEditor(IvanAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&lookAndFeel);
    setSize(kWidth, kHeight);
    setResizable(true, true);
    setResizeLimits(700, 450, 1400, 900);

    auto& apvts = processorRef.getAPVTS();

    // --- Channel buttons ---
    auto setupChannelBtn = [this](juce::TextButton& btn, int ch) {
        btn.setClickingTogglesState(false);
        btn.setColour(juce::TextButton::buttonColourId, juce::Colour(IvanLookAndFeel::bgSecondary));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colour(IvanLookAndFeel::textSecondary));
        btn.onClick = [this, ch] {
            processorRef.getAPVTS().getParameter("channel")->setValueNotifyingHost(float(ch) / 2.0f);
            updateChannelButtons(ch);
        };
        addAndMakeVisible(btn);
    };
    setupChannelBtn(cleanBtn, 0);
    setupChannelBtn(crunchBtn, 1);
    setupChannelBtn(leadBtn, 2);
    updateChannelButtons(0);

    // --- View buttons ---
    auto setupViewBtn = [this](juce::TextButton& btn, int view) {
        btn.setClickingTogglesState(false);
        btn.setColour(juce::TextButton::buttonColourId, juce::Colour(IvanLookAndFeel::bgPrimary));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colour(IvanLookAndFeel::textTertiary));
        btn.onClick = [this, view] { switchView(view); };
        addAndMakeVisible(btn);
    };
    setupViewBtn(perfViewBtn, 0);
    setupViewBtn(fullViewBtn, 1);
    setupViewBtn(hoodViewBtn, 2);
    switchView(0);

    // --- Performance knobs ---
    inputKnob     = std::make_unique<KnobComponent>(apvts, "inputGain",   "input",     "dB");
    gainKnob      = std::make_unique<KnobComponent>(apvts, "preampGain",  "gain",      "%");
    masterKnob    = std::make_unique<KnobComponent>(apvts, "master",      "master",    "%");
    outputKnob    = std::make_unique<KnobComponent>(apvts, "outputLevel", "output",    "dB");
    bassKnob      = std::make_unique<KnobComponent>(apvts, "bass",        "bass",      "%");
    midKnob       = std::make_unique<KnobComponent>(apvts, "mid",         "mid",       "%");
    trebleKnob    = std::make_unique<KnobComponent>(apvts, "treble",      "treble",    "%");
    presenceKnob  = std::make_unique<KnobComponent>(apvts, "presence",    "presence",  "%");
    resonanceKnob = std::make_unique<KnobComponent>(apvts, "resonance",   "resonance", "%");

    for (auto* k : {inputKnob.get(), gainKnob.get(), masterKnob.get(), outputKnob.get(),
                    bassKnob.get(), midKnob.get(), trebleKnob.get(), presenceKnob.get(), resonanceKnob.get()})
        addAndMakeVisible(k);

    // --- Full panel: combo boxes ---
    auto setupCombo = [this](juce::ComboBox& box, const juce::StringArray& items) {
        box.addItemList(items, 1);
        addAndMakeVisible(box);
    };
    setupCombo(toneStackModelBox, {"Marshall", "Fender", "Vox", "Mesa", "Hiwatt"});
    setupCombo(cabTypeBox, {"Open 1x12", "Closed 2x12", "Closed 4x12", "Open 2x12", "Open 4x10", "Closed 1x15", "Iso Box"});
    setupCombo(speakerTypeBox, {"Greenback", "V30", "Blue", "G12H-30", "Jensen P12R", "JBL D120", "Governor", "Retro 30"});
    setupCombo(micTypeBox, {"SM57", "MD421", "R121", "U87", "e609", "C414"});

    toneStackAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "toneStackModel", toneStackModelBox);
    cabTypeAtt   = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "cabType", cabTypeBox);
    speakerTypeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "speakerType", speakerTypeBox);
    micTypeAtt   = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "micType", micTypeBox);

    micDistKnob   = std::make_unique<KnobComponent>(apvts, "micDistance",    "distance",    "cm");
    micAngleKnob  = std::make_unique<KnobComponent>(apvts, "micAngle",      "angle",       "deg");
    micPosKnob    = std::make_unique<KnobComponent>(apvts, "micPosition",   "cone/edge",   "%");
    roomKnob      = std::make_unique<KnobComponent>(apvts, "roomAmount",    "room",        "%");
    compThreshKnob = std::make_unique<KnobComponent>(apvts, "compThreshold","comp thresh",  "dB");
    compRatioKnob = std::make_unique<KnobComponent>(apvts, "compRatio",     "ratio",       ":1");
    compMixKnob   = std::make_unique<KnobComponent>(apvts, "compMix",       "comp mix",    "%");
    gateThreshKnob = std::make_unique<KnobComponent>(apvts, "gateThreshold","gate thresh",  "dB");

    for (auto* k : {micDistKnob.get(), micAngleKnob.get(), micPosKnob.get(), roomKnob.get(),
                    compThreshKnob.get(), compRatioKnob.get(), compMixKnob.get(), gateThreshKnob.get()})
        addAndMakeVisible(k);

    // --- Hood: tube selectors ---
    setupCombo(preampTube1Box, {"12AX7", "12AU7", "12AT7", "EF86"});
    setupCombo(preampTube2Box, {"12AX7", "12AU7", "12AT7", "EF86"});
    setupCombo(powerTubeBox,   {"6V6", "EL84", "6L6", "EL34", "KT88", "6550"});
    setupCombo(oversamplingBox, {"Draft (2x)", "Normal (4x)", "HQ (8x)"});

    tube1Att    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "preampTube1", preampTube1Box);
    tube2Att    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "preampTube2", preampTube2Box);
    powerTubeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "powerTube", powerTubeBox);
    osAtt       = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "oversampling", oversamplingBox);

    biasKnob    = std::make_unique<KnobComponent>(apvts, "bias",     "bias",      "%");
    sagKnob     = std::make_unique<KnobComponent>(apvts, "sag",      "sag",       "%");
    varicKnob   = std::make_unique<KnobComponent>(apvts, "variac",   "variac",    "%");
    mismatchKnob = std::make_unique<KnobComponent>(apvts, "mismatch","mismatch",  "%");
    classABKnob = std::make_unique<KnobComponent>(apvts, "classAB",  "class A/AB","");

    for (auto* k : {biasKnob.get(), sagKnob.get(), varicKnob.get(), mismatchKnob.get(), classABKnob.get()})
        addAndMakeVisible(k);

    // Metering labels
    gateLabel.setText("gate: closed", juce::dontSendNotification);
    gateLabel.setFont(juce::Font(10.0f));
    gateLabel.setColour(juce::Label::textColourId, juce::Colour(IvanLookAndFeel::textTertiary));
    addAndMakeVisible(gateLabel);

    grLabel.setText("GR: 0 dB", juce::dontSendNotification);
    grLabel.setFont(juce::Font(10.0f));
    grLabel.setColour(juce::Label::textColourId, juce::Colour(IvanLookAndFeel::textTertiary));
    addAndMakeVisible(grLabel);

    startTimerHz(30);
}

IvanAudioProcessorEditor::~IvanAudioProcessorEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void IvanAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(IvanLookAndFeel::bgPrimary));

    auto bounds = getLocalBounds();

    // Header bar
    auto header = bounds.removeFromTop(44);
    g.setColour(juce::Colour(IvanLookAndFeel::bgSecondary));
    g.fillRect(header);
    g.setColour(juce::Colour(IvanLookAndFeel::borderSubtle));
    g.drawHorizontalLine(header.getBottom() - 1, 0, float(getWidth()));

    // Logo
    g.setColour(juce::Colour(IvanLookAndFeel::textPrimary));
    g.setFont(juce::Font(18.0f).boldened());
    g.drawText("ivan", 16, 0, 60, 44, juce::Justification::centredLeft);
    g.setColour(juce::Colour(IvanLookAndFeel::textTertiary));
    g.setFont(juce::Font(11.0f));
    g.drawText("valve amplifier", 78, 0, 120, 44, juce::Justification::centredLeft);

    // Footer
    auto footer = getLocalBounds().removeFromBottom(28);
    g.setColour(juce::Colour(IvanLookAndFeel::bgSecondary));
    g.fillRect(footer);
    g.setColour(juce::Colour(IvanLookAndFeel::borderSubtle));
    g.drawHorizontalLine(footer.getY(), 0, float(getWidth()));
    g.setColour(juce::Colour(IvanLookAndFeel::textTertiary));
    g.setFont(juce::Font(9.0f));
    g.drawText("ivan  |  valve amplifier  |  editsip", footer, juce::Justification::centred);

    // Section labels for current view
    auto content = getLocalBounds().reduced(0, 44).withTrimmedBottom(28);

    if (currentView == 1) {
        g.setColour(juce::Colour(IvanLookAndFeel::textTertiary));
        g.setFont(juce::Font(10.0f));
        g.drawText("TONE STACK", content.getX() + 20, content.getY() + 4, 200, 14, juce::Justification::centredLeft);
        g.drawText("CABINET & MIC", content.getX() + 20, content.getY() + 38, 200, 14, juce::Justification::centredLeft);
        g.drawText("DYNAMICS", content.getX() + 20, content.getY() + 160, 200, 14, juce::Justification::centredLeft);
    }

    if (currentView == 2) {
        g.setColour(juce::Colour(IvanLookAndFeel::textTertiary));
        g.setFont(juce::Font(10.0f));
        g.drawText("TUBE SELECTION", content.getX() + 20, content.getY() + 4, 200, 14, juce::Justification::centredLeft);
        g.drawText("POWER AMP INTERNALS", content.getX() + 20, content.getY() + 56, 200, 14, juce::Justification::centredLeft);
        g.drawText("QUALITY", content.getX() + 20, content.getY() + 220, 200, 14, juce::Justification::centredLeft);
    }
}

void IvanAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();
    auto header = bounds.removeFromTop(44);
    auto footer = bounds.removeFromBottom(28);
    auto content = bounds;

    // Channel buttons in header center
    int channelW = 70;
    int channelX = header.getCentreX() - (channelW * 3 + 8) / 2;
    cleanBtn.setBounds(channelX, 8, channelW, 28);
    crunchBtn.setBounds(channelX + channelW + 4, 8, channelW, 28);
    leadBtn.setBounds(channelX + (channelW + 4) * 2, 8, channelW, 28);

    // View tabs in header right
    int viewW = 82;
    int viewX = header.getRight() - (viewW * 3 + 8) - 12;
    perfViewBtn.setBounds(viewX, 10, viewW, 24);
    fullViewBtn.setBounds(viewX + viewW + 4, 10, viewW, 24);
    hoodViewBtn.setBounds(viewX + (viewW + 4) * 2, 10, viewW, 24);

    // Layout current view
    layoutPerformanceView(content);
    layoutFullView(content);
    layoutHoodView(content);

    // Metering in footer area
    gateLabel.setBounds(footer.getX() + 16, footer.getY() + 4, 80, 20);
    grLabel.setBounds(footer.getX() + 100, footer.getY() + 4, 80, 20);
}

void IvanAudioProcessorEditor::layoutPerformanceView(juce::Rectangle<int> area) {
    bool visible = (currentView == 0);
    int knobW = 80, knobH = 90;
    int padX = 12;

    // Row 1: gain controls
    int row1Y = area.getY() + 24;
    int totalW1 = knobW * 4 + padX * 3;
    int startX1 = area.getCentreX() - totalW1 / 2;

    inputKnob->setBounds(startX1, row1Y, knobW, knobH);
    gainKnob->setBounds(startX1 + (knobW + padX), row1Y, knobW, knobH);
    masterKnob->setBounds(startX1 + (knobW + padX) * 2, row1Y, knobW, knobH);
    outputKnob->setBounds(startX1 + (knobW + padX) * 3, row1Y, knobW, knobH);

    // Row 2: tone controls
    int row2Y = row1Y + knobH + 20;
    int totalW2 = knobW * 5 + padX * 4;
    int startX2 = area.getCentreX() - totalW2 / 2;

    bassKnob->setBounds(startX2, row2Y, knobW, knobH);
    midKnob->setBounds(startX2 + (knobW + padX), row2Y, knobW, knobH);
    trebleKnob->setBounds(startX2 + (knobW + padX) * 2, row2Y, knobW, knobH);
    presenceKnob->setBounds(startX2 + (knobW + padX) * 3, row2Y, knobW, knobH);
    resonanceKnob->setBounds(startX2 + (knobW + padX) * 4, row2Y, knobW, knobH);

    for (auto* k : {inputKnob.get(), gainKnob.get(), masterKnob.get(), outputKnob.get(),
                    bassKnob.get(), midKnob.get(), trebleKnob.get(), presenceKnob.get(), resonanceKnob.get()})
        k->setVisible(visible);
}

void IvanAudioProcessorEditor::layoutFullView(juce::Rectangle<int> area) {
    bool visible = (currentView == 1);
    int knobW = 68, knobH = 80;
    int comboH = 28;

    // Tone stack combo
    toneStackModelBox.setBounds(area.getX() + 20, area.getY() + 20, area.getWidth() - 40, comboH);
    toneStackModelBox.setVisible(visible);

    // Cabinet combos row
    int comboY = area.getY() + 54;
    int comboW = (area.getWidth() - 60) / 3;
    cabTypeBox.setBounds(area.getX() + 20, comboY, comboW, comboH);
    speakerTypeBox.setBounds(area.getX() + 30 + comboW, comboY, comboW, comboH);
    micTypeBox.setBounds(area.getX() + 40 + comboW * 2, comboY, comboW, comboH);
    cabTypeBox.setVisible(visible);
    speakerTypeBox.setVisible(visible);
    micTypeBox.setVisible(visible);

    // Mic knobs
    int micY = comboY + comboH + 10;
    int totalW = knobW * 4 + 12 * 3;
    int startX = area.getCentreX() - totalW / 2;
    micDistKnob->setBounds(startX, micY, knobW, knobH);
    micAngleKnob->setBounds(startX + knobW + 12, micY, knobW, knobH);
    micPosKnob->setBounds(startX + (knobW + 12) * 2, micY, knobW, knobH);
    roomKnob->setBounds(startX + (knobW + 12) * 3, micY, knobW, knobH);

    // Dynamics knobs
    int dynY = micY + knobH + 28;
    compThreshKnob->setBounds(startX, dynY, knobW, knobH);
    compRatioKnob->setBounds(startX + knobW + 12, dynY, knobW, knobH);
    compMixKnob->setBounds(startX + (knobW + 12) * 2, dynY, knobW, knobH);
    gateThreshKnob->setBounds(startX + (knobW + 12) * 3, dynY, knobW, knobH);

    for (auto* k : {micDistKnob.get(), micAngleKnob.get(), micPosKnob.get(), roomKnob.get(),
                    compThreshKnob.get(), compRatioKnob.get(), compMixKnob.get(), gateThreshKnob.get()})
        k->setVisible(visible);
}

void IvanAudioProcessorEditor::layoutHoodView(juce::Rectangle<int> area) {
    bool visible = (currentView == 2);
    int knobW = 68, knobH = 80;
    int comboH = 28;

    // Tube selector combos
    int comboW = (area.getWidth() - 80) / 3;
    int comboY = area.getY() + 20;
    preampTube1Box.setBounds(area.getX() + 20, comboY, comboW, comboH);
    preampTube2Box.setBounds(area.getX() + 40 + comboW, comboY, comboW, comboH);
    powerTubeBox.setBounds(area.getX() + 60 + comboW * 2, comboY, comboW, comboH);
    preampTube1Box.setVisible(visible);
    preampTube2Box.setVisible(visible);
    powerTubeBox.setVisible(visible);

    // Power amp knobs
    int knobY1 = comboY + comboH + 22;
    int totalW = knobW * 5 + 10 * 4;
    int startX = area.getCentreX() - totalW / 2;
    biasKnob->setBounds(startX, knobY1, knobW, knobH);
    sagKnob->setBounds(startX + knobW + 10, knobY1, knobW, knobH);
    varicKnob->setBounds(startX + (knobW + 10) * 2, knobY1, knobW, knobH);
    mismatchKnob->setBounds(startX + (knobW + 10) * 3, knobY1, knobW, knobH);
    classABKnob->setBounds(startX + (knobW + 10) * 4, knobY1, knobW, knobH);

    for (auto* k : {biasKnob.get(), sagKnob.get(), varicKnob.get(), mismatchKnob.get(), classABKnob.get()})
        k->setVisible(visible);

    // Quality selector
    oversamplingBox.setBounds(area.getX() + 20, knobY1 + knobH + 28, 200, comboH);
    oversamplingBox.setVisible(visible);
}

void IvanAudioProcessorEditor::switchView(int view) {
    currentView = view;

    perfViewBtn.setColour(juce::TextButton::textColourOffId,
        juce::Colour(view == 0 ? IvanLookAndFeel::textPrimary : IvanLookAndFeel::textTertiary));
    fullViewBtn.setColour(juce::TextButton::textColourOffId,
        juce::Colour(view == 1 ? IvanLookAndFeel::textPrimary : IvanLookAndFeel::textTertiary));
    hoodViewBtn.setColour(juce::TextButton::textColourOffId,
        juce::Colour(view == 2 ? IvanLookAndFeel::textPrimary : IvanLookAndFeel::textTertiary));

    resized();
    repaint();
}

void IvanAudioProcessorEditor::updateChannelButtons(int ch) {
    cleanBtn.setColour(juce::TextButton::textColourOffId,
        juce::Colour(ch == 0 ? IvanLookAndFeel::green : IvanLookAndFeel::textSecondary));
    crunchBtn.setColour(juce::TextButton::textColourOffId,
        juce::Colour(ch == 1 ? IvanLookAndFeel::amber : IvanLookAndFeel::textSecondary));
    leadBtn.setColour(juce::TextButton::textColourOffId,
        juce::Colour(ch == 2 ? IvanLookAndFeel::red : IvanLookAndFeel::textSecondary));
}

void IvanAudioProcessorEditor::timerCallback() {
    auto& chain = processorRef.getSignalChain();
    gainReduction = chain.getCompressor().getGainReduction();
    gateOpen = chain.getNoiseGate().isOpen();

    gateLabel.setText(juce::String("gate: ") + (gateOpen ? "open" : "closed"),
                      juce::dontSendNotification);
    gateLabel.setColour(juce::Label::textColourId,
        juce::Colour(gateOpen ? IvanLookAndFeel::green : IvanLookAndFeel::textTertiary));

    grLabel.setText("GR: " + juce::String(gainReduction, 1) + " dB", juce::dontSendNotification);
}

