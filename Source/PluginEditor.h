#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "UI/IvanLookAndFeel.h"
#include "UI/KnobComponent.h"

class IvanAudioProcessorEditor : public juce::AudioProcessorEditor,
                                  private juce::Timer {
public:
    IvanAudioProcessorEditor(IvanAudioProcessor&);
    ~IvanAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    IvanAudioProcessor& processorRef;
    IvanLookAndFeel lookAndFeel;

    // Channel tabs
    juce::TextButton cleanBtn{"clean"}, crunchBtn{"crunch"}, leadBtn{"lead"};

    // View tabs
    juce::TextButton perfViewBtn{"performance"}, fullViewBtn{"full panel"}, hoodViewBtn{"under the hood"};
    int currentView = 0;

    // Performance view knobs
    std::unique_ptr<KnobComponent> inputKnob, gainKnob, masterKnob, outputKnob;
    std::unique_ptr<KnobComponent> bassKnob, midKnob, trebleKnob, presenceKnob, resonanceKnob;

    // Full panel
    juce::ComboBox toneStackModelBox, cabTypeBox, speakerTypeBox, micTypeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        toneStackAtt, cabTypeAtt, speakerTypeAtt, micTypeAtt;
    std::unique_ptr<KnobComponent> micDistKnob, micAngleKnob, micPosKnob, roomKnob;
    std::unique_ptr<KnobComponent> compThreshKnob, compRatioKnob, compMixKnob, gateThreshKnob;

    // Under the hood
    juce::ComboBox preampTube1Box, preampTube2Box, powerTubeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        tube1Att, tube2Att, powerTubeAtt;
    std::unique_ptr<KnobComponent> biasKnob, sagKnob, varicKnob, mismatchKnob, classABKnob;
    juce::ComboBox oversamplingBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osAtt;

    // Metering
    juce::Label gateLabel, grLabel;
    float gainReduction = 0.0f;
    bool gateOpen = false;

    // Layout helpers
    void layoutPerformanceView(juce::Rectangle<int> area);
    void layoutFullView(juce::Rectangle<int> area);
    void layoutHoodView(juce::Rectangle<int> area);
    void switchView(int view);
    void updateChannelButtons(int channel);

    static constexpr int kWidth = 900;
    static constexpr int kHeight = 580;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IvanAudioProcessorEditor)
};
