#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/SignalChain.h"

class IvanAudioProcessor : public juce::AudioProcessor {
public:
    IvanAudioProcessor();
    ~IvanAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Parameter tree
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Access to signal chain for metering
    ivan::SignalChain& getSignalChain() { return signalChain; }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateParametersFromAPVTS();

    juce::AudioProcessorValueTreeState apvts;
    ivan::SignalChain signalChain;

    // Cached parameter pointers for efficient access
    std::atomic<float>* channelParam = nullptr;
    std::atomic<float>* inputGainParam = nullptr;
    std::atomic<float>* outputLevelParam = nullptr;
    std::atomic<float>* preampGainParam = nullptr;
    std::atomic<float>* bassParam = nullptr;
    std::atomic<float>* midParam = nullptr;
    std::atomic<float>* trebleParam = nullptr;
    std::atomic<float>* presenceParam = nullptr;
    std::atomic<float>* resonanceParam = nullptr;
    std::atomic<float>* masterParam = nullptr;

    // Preamp tube selections
    std::atomic<float>* preampTube1Param = nullptr;
    std::atomic<float>* preampTube2Param = nullptr;
    std::atomic<float>* powerTubeParam = nullptr;

    // Advanced controls
    std::atomic<float>* biasParam = nullptr;
    std::atomic<float>* sagParam = nullptr;
    std::atomic<float>* varicParam = nullptr;
    std::atomic<float>* mismatchParam = nullptr;
    std::atomic<float>* classABParam = nullptr;

    // Tone stack model
    std::atomic<float>* toneStackModelParam = nullptr;

    // Compressor
    std::atomic<float>* compEnabledParam = nullptr;
    std::atomic<float>* compThreshParam = nullptr;
    std::atomic<float>* compRatioParam = nullptr;
    std::atomic<float>* compAttackParam = nullptr;
    std::atomic<float>* compReleaseParam = nullptr;
    std::atomic<float>* compMixParam = nullptr;

    // Noise gate
    std::atomic<float>* gateEnabledParam = nullptr;
    std::atomic<float>* gateThreshParam = nullptr;
    std::atomic<float>* gateAttackParam = nullptr;
    std::atomic<float>* gateReleaseParam = nullptr;

    // Cabinet
    std::atomic<float>* cabEnabledParam = nullptr;
    std::atomic<float>* cabTypeParam = nullptr;
    std::atomic<float>* speakerTypeParam = nullptr;
    std::atomic<float>* micTypeParam = nullptr;
    std::atomic<float>* micDistanceParam = nullptr;
    std::atomic<float>* micAngleParam = nullptr;
    std::atomic<float>* micPositionParam = nullptr;
    std::atomic<float>* roomAmountParam = nullptr;

    // Oversampling
    std::atomic<float>* oversamplingParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IvanAudioProcessor)
};
