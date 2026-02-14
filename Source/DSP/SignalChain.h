#pragma once
#include <juce_dsp/juce_dsp.h>
#include "TubeStage.h"
#include "ToneStack.h"
#include "PowerAmp.h"
#include "CabinetSim.h"
#include "Compressor.h"
#include "NoiseGate.h"
#include "Oversampling.h"

namespace ivan {

// ============================================================================
// SignalChain — orchestrates the full amp signal path.
//
// Signal flow:
//   Input → Noise Gate → Input Gain → Compressor →
//   Preamp (N cascaded tube stages) → Tone Stack →
//   Power Amp (push-pull + sag + NFB) →
//   Cabinet Sim (speaker + mic + room) → Output Level
//
// The chain processes at oversampled rate through the nonlinear sections
// (preamp + power amp) and returns to native rate for the linear sections.
// ============================================================================

class SignalChain {
public:
    SignalChain();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    void processBlock(juce::AudioBuffer<float>& buffer);

    // Access sub-modules for parameter control
    NoiseGate& getNoiseGate()         { return noiseGate; }
    Compressor& getCompressor()       { return compressor; }
    PreampChain& getPreamp()          { return preamp; }
    ToneStack& getToneStack()         { return toneStack; }
    PowerAmp& getPowerAmp()           { return powerAmp; }
    CabinetSim& getCabinetSim()       { return cabinetSim; }

    // Global controls
    void setInputGain(float gainDb);
    void setOutputLevel(float levelDb);
    void setChannel(int channel);       // 0=clean, 1=crunch, 2=lead
    void setOversamplingQuality(OversamplingQuality quality);

    int getCurrentChannel() const { return currentChannel; }

private:
    void createOversampler();

    NoiseGate noiseGate;
    Compressor compressor;
    PreampChain preamp;
    ToneStack toneStack;
    PowerAmp powerAmp;
    CabinetSim cabinetSim;

    float inputGainLinear = 1.0f;
    float outputLevelLinear = 1.0f;
    int currentChannel = 0;

    double sampleRate = 44100.0;
    int maxBlockSize = 512;

    // Oversampling
    OversamplingQuality osQuality = OversamplingQuality::Normal;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
};

} // namespace ivan
