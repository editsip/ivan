#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

// ============================================================================
// Plugin Editor — uses an embedded WebBrowserComponent for the UI.
//
// The JUCE WebBrowserComponent renders our HTML/CSS/JS interface.
// Communication between the C++ processor and JS UI happens via:
// - C++ → JS: evaluateJavascript() to push parameter state
// - JS → C++: page navigation interception for parameter changes
//
// This approach gives us full creative control over the UI while keeping
// all DSP in native C++.
// ============================================================================

class IvanAudioProcessorEditor : public juce::AudioProcessorEditor,
                                  private juce::Timer {
public:
    IvanAudioProcessorEditor(IvanAudioProcessor&);
    ~IvanAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void sendParameterUpdate(const juce::String& paramId, float value);
    void sendAllParameters();

    // Build the HTML content from binary resources
    juce::String buildHTMLContent();

    IvanAudioProcessor& processorRef;

    std::unique_ptr<juce::WebBrowserComponent> webView;

    // Parameter listener for bidirectional sync
    struct ParameterListener : public juce::AudioProcessorValueTreeState::Listener {
        IvanAudioProcessorEditor& editor;
        ParameterListener(IvanAudioProcessorEditor& e) : editor(e) {}
        void parameterChanged(const juce::String& parameterID, float newValue) override;
    };
    std::unique_ptr<ParameterListener> paramListener;

    static constexpr int kDefaultWidth = 900;
    static constexpr int kDefaultHeight = 640;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IvanAudioProcessorEditor)
};
