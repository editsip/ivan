#include "NoiseGate.h"

namespace ivan {

NoiseGate::NoiseGate() {
    rangeLinear = juce::Decibels::decibelsToGain(rangeDb);
}

void NoiseGate::prepare(double sr, int /*samplesPerBlock*/) {
    sampleRate = sr;

    attackCoeff = 1.0f - std::exp(-1.0f / (float(sr) * attackMs * 0.001f));
    releaseCoeff = 1.0f - std::exp(-1.0f / (float(sr) * releaseMs * 0.001f));
    holdSamples = int(holdMs * 0.001f * float(sr));

    // Default sidechain: HPF at 80Hz, LPF at 3kHz
    setSidechainHPF(80.0f);
    setSidechainLPF(3000.0f);

    reset();
}

void NoiseGate::reset() {
    envelope = 0.0f;
    gateState = 0.0f;
    holdCounter = 0;
    scHPFstate = 0.0f;
    scLPFstate = 0.0f;
}

void NoiseGate::processBlock(juce::dsp::AudioBlock<float>& block) {
    if (!gateEnabled) return;

    for (size_t i = 0; i < block.getNumSamples(); ++i) {
        // Compute sidechain level (peak across channels, filtered)
        float scLevel = 0.0f;
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch) {
            float x = block.getChannelPointer(ch)[i];

            // Sidechain HPF (remove hum)
            scHPFstate += scHPFcoeff * (x - scHPFstate);
            float hpOut = x - scHPFstate;

            // Sidechain LPF (remove hiss)
            scLPFstate += scLPFcoeff * (hpOut - scLPFstate);

            scLevel = std::max(scLevel, std::abs(scLPFstate));
        }

        // Convert to dB
        float scDb = juce::Decibels::gainToDecibels(scLevel, -96.0f);

        // Gate logic with hysteresis
        float openThreshold = threshold;
        float closeThreshold = threshold - hysteresis;

        if (scDb > openThreshold) {
            // Signal above threshold — open gate
            holdCounter = holdSamples;
            gateState += attackCoeff * (1.0f - gateState);
        } else if (scDb < closeThreshold && holdCounter <= 0) {
            // Signal below threshold - hysteresis, hold expired — close gate
            gateState += releaseCoeff * (0.0f - gateState);
        } else {
            // In hold period or in hysteresis zone
            if (holdCounter > 0) holdCounter--;
        }

        gateState = juce::jlimit(0.0f, 1.0f, gateState);

        // Apply gate: interpolate between full signal and attenuated signal
        float gain = rangeLinear + gateState * (1.0f - rangeLinear);

        for (size_t ch = 0; ch < block.getNumChannels(); ++ch) {
            block.getChannelPointer(ch)[i] *= gain;
        }
    }
}

void NoiseGate::setThreshold(float t)   { threshold = juce::jlimit(-96.0f, 0.0f, t); }
void NoiseGate::setHysteresis(float h)  { hysteresis = juce::jlimit(0.0f, 12.0f, h); }
void NoiseGate::setRange(float r)       {
    rangeDb = juce::jlimit(-96.0f, 0.0f, r);
    rangeLinear = juce::Decibels::decibelsToGain(rangeDb);
}

void NoiseGate::setAttack(float a) {
    attackMs = juce::jlimit(0.01f, 50.0f, a);
    attackCoeff = 1.0f - std::exp(-1.0f / (float(sampleRate) * attackMs * 0.001f));
}

void NoiseGate::setHold(float h) {
    holdMs = juce::jlimit(0.0f, 500.0f, h);
    holdSamples = int(holdMs * 0.001f * float(sampleRate));
}

void NoiseGate::setRelease(float r) {
    releaseMs = juce::jlimit(5.0f, 2000.0f, r);
    releaseCoeff = 1.0f - std::exp(-1.0f / (float(sampleRate) * releaseMs * 0.001f));
}

void NoiseGate::setSidechainHPF(float freq) {
    freq = juce::jlimit(20.0f, 500.0f, freq);
    scHPFcoeff = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * freq / float(sampleRate));
}

void NoiseGate::setSidechainLPF(float freq) {
    freq = juce::jlimit(1000.0f, 20000.0f, freq);
    scLPFcoeff = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * freq / float(sampleRate));
}

} // namespace ivan
