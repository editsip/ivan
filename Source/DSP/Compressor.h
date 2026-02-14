#pragma once
#include <juce_dsp/juce_dsp.h>

namespace ivan {

// ============================================================================
// Compressor — per-channel styles with different characteristics
//
// Clean channel:  Studio-style optical comp (LA-2A inspired) — smooth, musical
// Crunch channel: FET compressor (1176 inspired) — fast, punchy
// Lead channel:   VCA compressor — tight, transparent, sustain-focused
//
// Each style has different attack/release curves, knee shapes, and
// detector characteristics.
// ============================================================================

enum class CompressorStyle {
    Optical,    // LA-2A style: program-dependent attack/release, smooth
    FET,        // 1176 style: fast attack, ratio options, punchy
    VCA         // DBX style: tight, transparent, predictable
};

class Compressor {
public:
    Compressor();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    void processBlock(juce::dsp::AudioBlock<float>& block);

    void setStyle(CompressorStyle style);
    void setThreshold(float thresholdDb);     // -60..0 dB
    void setRatio(float ratio);               // 1..20 (1=no compression)
    void setAttack(float attackMs);           // 0.1..100 ms
    void setRelease(float releaseMs);         // 10..2000 ms
    void setMakeupGain(float gainDb);         // 0..30 dB
    void setMix(float mixNormalized);         // 0..1 (parallel compression)
    void setEnabled(bool enabled) { compEnabled = enabled; }

    // Metering
    float getGainReduction() const { return currentGainReduction; }

private:
    float processDetector(float inputLevel);
    float computeGain(float detectorLevel);

    CompressorStyle style = CompressorStyle::Optical;

    float threshold = -20.0f;
    float ratio = 4.0f;
    float attackMs = 10.0f;
    float releaseMs = 100.0f;
    float makeupGain = 0.0f;
    float mix = 1.0f;
    bool compEnabled = true;

    double sampleRate = 44100.0;

    // Envelope detector state
    float envelope = 0.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    // Optical comp: second slower envelope for program-dependent behavior
    float opticalEnvelope = 0.0f;

    // Gain reduction metering
    float currentGainReduction = 0.0f;

    // Soft knee width in dB
    float kneeWidth = 6.0f;
};

} // namespace ivan
