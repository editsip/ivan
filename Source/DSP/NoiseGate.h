#pragma once
#include <juce_dsp/juce_dsp.h>

namespace ivan {

// ============================================================================
// Frequency-Conscious Noise Gate
//
// Unlike a simple level gate, this uses a sidechain filter to focus the
// detection on frequencies where guitar signal lives (roughly 80Hz-3kHz),
// ignoring hum (50/60Hz), hiss (>5kHz), and other noise that shouldn't
// trigger the gate open.
//
// Features:
// - Adjustable threshold, attack, hold, release
// - Sidechain high-pass and low-pass for frequency-conscious detection
// - Hysteresis to prevent chattering
// - Range control (how much attenuation when closed)
// ============================================================================

class NoiseGate {
public:
    NoiseGate();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    void processBlock(juce::dsp::AudioBlock<float>& block);

    void setThreshold(float thresholdDb);     // -96..0 dB
    void setAttack(float attackMs);           // 0.01..50 ms
    void setHold(float holdMs);               // 0..500 ms
    void setRelease(float releaseMs);         // 5..2000 ms
    void setRange(float rangeDb);             // -96..0 dB (how much attenuation)
    void setHysteresis(float hysteresisDb);   // 0..12 dB
    void setEnabled(bool enabled) { gateEnabled = enabled; }

    // Sidechain filter control
    void setSidechainHPF(float freqHz);       // 20..500 Hz
    void setSidechainLPF(float freqHz);       // 1000..20000 Hz

    bool isOpen() const { return gateState > 0.5f; }

private:
    float threshold = -40.0f;
    float attackMs = 0.5f;
    float holdMs = 50.0f;
    float releaseMs = 100.0f;
    float rangeDb = -80.0f;
    float hysteresis = 6.0f;
    bool gateEnabled = true;

    double sampleRate = 44100.0;

    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    int holdSamples = 0;
    int holdCounter = 0;

    float gateState = 0.0f;  // 0=closed, 1=open
    float envelope = 0.0f;

    // Sidechain filters
    float scHPFstate = 0.0f;
    float scHPFcoeff = 0.0f;
    float scLPFstate = 0.0f;
    float scLPFcoeff = 0.0f;

    float rangeLinear = 0.0f;
};

} // namespace ivan
