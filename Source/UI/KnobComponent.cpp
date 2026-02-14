#include "KnobComponent.h"
#include "IvanLookAndFeel.h"

KnobComponent::KnobComponent(juce::AudioProcessorValueTreeState& apvts,
                               const juce::String& paramID_,
                               const juce::String& label,
                               const juce::String& suffix_)
    : suffix(suffix_), paramID(paramID_)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    slider.setDoubleClickReturnValue(true, apvts.getParameter(paramID)->getDefaultValue());
    addAndMakeVisible(slider);

    nameLabel.setText(label, juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setFont(juce::Font(10.0f));
    nameLabel.setColour(juce::Label::textColourId, juce::Colour(IvanLookAndFeel::textSecondary));
    addAndMakeVisible(nameLabel);

    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setFont(juce::Font(9.0f));
    valueLabel.setColour(juce::Label::textColourId, juce::Colour(IvanLookAndFeel::textTertiary));
    addAndMakeVisible(valueLabel);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, paramID, slider);

    slider.onValueChange = [this] { updateValueLabel(); };
    updateValueLabel();
}

void KnobComponent::resized() {
    auto bounds = getLocalBounds();
    auto knobSize = juce::jmin(bounds.getWidth(), bounds.getHeight() - 28);
    slider.setBounds(bounds.removeFromTop(knobSize).withSizeKeepingCentre(knobSize, knobSize));
    nameLabel.setBounds(bounds.removeFromTop(14));
    valueLabel.setBounds(bounds.removeFromTop(14));
}

void KnobComponent::paint(juce::Graphics&) {
    // All drawing handled by children and LookAndFeel
}

void KnobComponent::updateValueLabel() {
    auto val = slider.getValue();

    juce::String text;
    if (suffix == "dB") {
        text = juce::String(val, 1) + " dB";
    } else if (suffix == "%") {
        text = juce::String(int(val * 100)) + "%";
    } else if (suffix == ":1") {
        text = juce::String(val, 1) + ":1";
    } else if (suffix == "cm") {
        text = juce::String(int(val)) + "cm";
    } else if (suffix == "deg") {
        text = juce::String(int(val)) + juce::String::charToString(0x00B0);
    } else if (suffix == "ms") {
        text = juce::String(val, 1) + "ms";
    } else if (paramID == "classAB") {
        text = val < 0.3 ? "AB" : val > 0.7 ? "A" : "A/AB";
    } else {
        text = juce::String(int(val * 100)) + "%";
    }

    valueLabel.setText(text, juce::dontSendNotification);
}
