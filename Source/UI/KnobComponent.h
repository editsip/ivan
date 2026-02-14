#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

// A labeled knob with value display, matching the Ivan aesthetic
class KnobComponent : public juce::Component {
public:
    KnobComponent(juce::AudioProcessorValueTreeState& apvts,
                  const juce::String& paramID,
                  const juce::String& label,
                  const juce::String& suffix = "");

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::Slider slider;
    juce::Label nameLabel;
    juce::Label valueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    juce::String suffix;
    juce::String paramID;

    void updateValueLabel();
};
