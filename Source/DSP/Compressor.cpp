#include "Compressor.h"

namespace ivan {

Compressor::Compressor() = default;

void Compressor::prepare(double sr, int /*samplesPerBlock*/) {
    sampleRate = sr;
    reset();

    attackCoeff = 1.0f - std::exp(-1.0f / (float(sr) * attackMs * 0.001f));
    releaseCoeff = 1.0f - std::exp(-1.0f / (float(sr) * releaseMs * 0.001f));
}

void Compressor::reset() {
    envelope = 0.0f;
    opticalEnvelope = 0.0f;
    currentGainReduction = 0.0f;
}

void Compressor::processBlock(juce::dsp::AudioBlock<float>& block) {
    if (!compEnabled) return;

    float makeupLinear = juce::Decibels::decibelsToGain(makeupGain);

    for (size_t i = 0; i < block.getNumSamples(); ++i) {
        // Compute input level (peak across channels)
        float peakLevel = 0.0f;
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch) {
            peakLevel = std::max(peakLevel, std::abs(block.getChannelPointer(ch)[i]));
        }

        // Convert to dB for processing
        float inputDb = juce::Decibels::gainToDecibels(peakLevel, -96.0f);

        // Detector (style-dependent)
        float detectedDb = processDetector(inputDb);

        // Gain computer
        float gainDb = computeGain(detectedDb);

        // Track gain reduction for metering
        currentGainReduction = gainDb;

        float gainLinear = juce::Decibels::decibelsToGain(gainDb) * makeupLinear;

        // Apply gain to all channels
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch) {
            float dry = block.getChannelPointer(ch)[i];
            float wet = dry * gainLinear;

            // Parallel compression (mix)
            block.getChannelPointer(ch)[i] = dry * (1.0f - mix) + wet * mix;
        }
    }
}

void Compressor::setStyle(CompressorStyle s) {
    style = s;

    // Set style-specific defaults
    switch (style) {
        case CompressorStyle::Optical:
            kneeWidth = 10.0f;  // Very soft knee (optical is smooth)
            break;
        case CompressorStyle::FET:
            kneeWidth = 3.0f;   // Hard knee (FET is punchy)
            break;
        case CompressorStyle::VCA:
            kneeWidth = 6.0f;   // Medium knee
            break;
    }
}

void Compressor::setThreshold(float t)   { threshold = juce::jlimit(-60.0f, 0.0f, t); }
void Compressor::setRatio(float r)       { ratio = juce::jlimit(1.0f, 20.0f, r); }
void Compressor::setMakeupGain(float g)  { makeupGain = juce::jlimit(0.0f, 30.0f, g); }
void Compressor::setMix(float m)         { mix = juce::jlimit(0.0f, 1.0f, m); }

void Compressor::setAttack(float a) {
    attackMs = juce::jlimit(0.1f, 100.0f, a);
    attackCoeff = 1.0f - std::exp(-1.0f / (float(sampleRate) * attackMs * 0.001f));
}

void Compressor::setRelease(float r) {
    releaseMs = juce::jlimit(10.0f, 2000.0f, r);
    releaseCoeff = 1.0f - std::exp(-1.0f / (float(sampleRate) * releaseMs * 0.001f));
}

// ============================================================================
// Envelope detector — different behaviors per style
// ============================================================================

float Compressor::processDetector(float inputDb) {
    float target = inputDb;

    switch (style) {
        case CompressorStyle::Optical: {
            // Optical compressor: the photocell has inherent program-dependent
            // behavior. Attack gets slower as compression deepens (light element
            // thermal mass). Release is two-stage: fast initial, slow tail.
            float opticalAttack = attackCoeff * 0.5f;  // Slower attack
            float diff = target - envelope;

            if (diff > 0.0f) {
                // Attack: slower when already compressing hard
                float depthFactor = 1.0f / (1.0f + std::abs(envelope - threshold) * 0.02f);
                envelope += opticalAttack * depthFactor * diff;
            } else {
                // Two-stage release: fast component + slow component
                float fastRelease = releaseCoeff * 2.0f;
                float slowRelease = releaseCoeff * 0.2f;
                opticalEnvelope += slowRelease * (target - opticalEnvelope);
                envelope += fastRelease * (opticalEnvelope - envelope);
            }
            break;
        }

        case CompressorStyle::FET: {
            // FET compressor: very fast attack, feed-back topology
            // Attack time can be sub-millisecond
            float fetAttack = attackCoeff * 3.0f;  // Faster
            fetAttack = std::min(fetAttack, 0.99f);

            if (target > envelope)
                envelope += fetAttack * (target - envelope);
            else
                envelope += releaseCoeff * (target - envelope);
            break;
        }

        case CompressorStyle::VCA: {
            // VCA: predictable, linear envelope tracking
            if (target > envelope)
                envelope += attackCoeff * (target - envelope);
            else
                envelope += releaseCoeff * (target - envelope);
            break;
        }
    }

    return envelope;
}

// ============================================================================
// Gain computer with soft knee
// ============================================================================

float Compressor::computeGain(float detectorLevel) {
    float x = detectorLevel;
    float t = threshold;
    float w = kneeWidth;
    float r = ratio;

    float output;

    if (x < (t - w / 2.0f)) {
        // Below knee: no compression
        output = x;
    } else if (x > (t + w / 2.0f)) {
        // Above knee: full compression
        output = t + (x - t) / r;
    } else {
        // In knee: smooth transition (quadratic interpolation)
        float kneeInput = x - t + w / 2.0f;
        output = x + (1.0f / r - 1.0f) * kneeInput * kneeInput / (2.0f * w);
    }

    return output - x;  // Return gain reduction (negative dB)
}

} // namespace ivan
