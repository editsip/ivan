#include "PowerAmp.h"

namespace ivan {

PowerAmp::PowerAmp() {
    tubePositive.setTubeType(TubeType::EL34);
    tubeNegative.setTubeType(TubeType::EL34);
}

void PowerAmp::prepare(double sr, int samplesPerBlock) {
    sampleRate = sr;

    tubePositive.prepare(sr, samplesPerBlock);
    tubeNegative.prepare(sr, samplesPerBlock);

    // Sag envelope: attack ~10ms (rectifier charging), release ~100ms (filter cap discharge)
    sagAttack = 1.0f - std::exp(-1.0f / (float(sr) * 0.010f));
    sagRelease = 1.0f - std::exp(-1.0f / (float(sr) * 0.100f));

    // Output transformer: LF resonance ~60Hz, HF rolloff ~8kHz
    transformerLFcoeff = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * 60.0f / float(sr));
    transformerHFcoeff = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * 8000.0f / float(sr));

    // Presence filter: ~3-5kHz shelf
    presenceCoeff = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * 4000.0f / float(sr));

    // Resonance filter: ~80-120Hz
    resonanceCoeff = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * 90.0f / float(sr));

    reset();
}

void PowerAmp::reset() {
    tubePositive.reset();
    tubeNegative.reset();
    sagEnvelope = 1.0f;
    transformerLFstate = 0.0f;
    transformerHFstate = 0.0f;
    nfbState = 0.0f;
    presenceState = 0.0f;
    resonanceState = 0.0f;
}

void PowerAmp::processBlock(juce::dsp::AudioBlock<float>& block) {
    for (size_t ch = 0; ch < block.getNumChannels(); ++ch) {
        auto* data = block.getChannelPointer(ch);
        for (size_t i = 0; i < block.getNumSamples(); ++i) {
            float x = data[i] * drive * 2.0f;

            // Apply variac — scales the effective headroom
            x *= variac;

            // Negative feedback subtraction
            x = processNegativeFeedback(x);

            // Power supply sag
            x = processSag(x);

            // Push-pull tube pair
            x = processPushPull(x);

            // Output transformer coloring
            x = processOutputTransformer(x);

            data[i] = x;
        }
    }
}

void PowerAmp::setPowerTubeType(TubeType type) {
    tubePositive.setTubeType(type);
    tubeNegative.setTubeType(type);
}

void PowerAmp::setDrive(float d)        { drive = juce::jlimit(0.0f, 1.0f, d); }
void PowerAmp::setPresence(float p)     { presence = juce::jlimit(0.0f, 1.0f, p); }
void PowerAmp::setResonance(float r)    { resonance = juce::jlimit(0.0f, 1.0f, r); }
void PowerAmp::setSag(float s)          { sagAmount = juce::jlimit(0.0f, 1.0f, s); }
void PowerAmp::setVariac(float v)       { variac = juce::jlimit(0.3f, 1.0f, v); }
void PowerAmp::setBias(float b)         {
    tubePositive.setBias(b);
    tubeNegative.setBias(b);
}
void PowerAmp::setClassAB_A(float blend) { classABlend = juce::jlimit(0.0f, 1.0f, blend); }

// ============================================================================
// Push-pull output stage
//
// In a real push-pull amp, the output transformer receives current from
// two tubes operating 180° out of phase. In Class AB, each tube conducts
// for slightly more than half the cycle. At the crossover point, both
// tubes are near cutoff, creating the characteristic "crossover notch."
//
// In Class A (e.g., Vox AC30), both tubes conduct for the full cycle,
// eliminating crossover distortion but reducing efficiency and increasing
// even harmonics.
// ============================================================================

float PowerAmp::processPushPull(float input) {
    // Split signal for push-pull operation
    float positive = std::max(0.0f, input);
    float negative = std::min(0.0f, input);

    // Class A component: both tubes see full signal
    float classAout = tubePositive.processSample(input * 0.5f);

    // Class AB component: each tube handles one half
    float posOut = tubePositive.processSample(positive);
    float negOut = tubeNegative.processSample(negative);
    float classABout = posOut + negOut;

    // Blend between Class A and Class AB
    return classABout * (1.0f - classABlend) + classAout * classABlend;
}

// ============================================================================
// Power supply sag
//
// When the amp is driven hard, the rectifier can't supply current fast enough.
// The B+ voltage drops ("sags"), reducing headroom. This creates a natural
// compression — attack is softened, sustain is enhanced. It's a defining
// characteristic of vintage amps (especially with tube rectifiers like GZ34).
//
// We model this as an envelope follower that modulates the signal amplitude.
// ============================================================================

float PowerAmp::processSag(float input) {
    float absInput = std::abs(input);

    // Envelope follower with asymmetric attack/release
    float coeff = (absInput > sagEnvelope) ? sagAttack : sagRelease;
    sagEnvelope += coeff * (absInput - sagEnvelope);

    // Sag reduces effective headroom as envelope increases
    float sagReduction = 1.0f - sagAmount * sagEnvelope * 0.5f;
    sagReduction = std::max(0.3f, sagReduction);  // Never fully collapse

    return input * sagReduction;
}

// ============================================================================
// Output transformer
//
// The output transformer is not a transparent element. It has:
// - Low frequency resonance (~50-80Hz depending on iron)
// - High frequency rolloff (~8-12kHz)
// - Saturation at LF (iron core saturation adds warmth)
//
// We model this as a bandpass characteristic with mild saturation.
// ============================================================================

float PowerAmp::processOutputTransformer(float input) {
    // LF: high-pass to model transformer's low-frequency rolloff
    transformerLFstate += transformerLFcoeff * (input - transformerLFstate);
    float hpOut = input - transformerLFstate;

    // Mild LF saturation (transformer iron core)
    float lfSat = transformerLFstate;
    lfSat = std::tanh(lfSat * 1.5f) / 1.5f;

    // HF: low-pass to model winding capacitance rolloff
    float hfInput = hpOut + lfSat;
    transformerHFstate += transformerHFcoeff * (hfInput - transformerHFstate);

    return transformerHFstate;
}

// ============================================================================
// Negative feedback
//
// Most guitar amps have a negative feedback loop from the output transformer
// secondary back to the phase inverter input. This:
// - Reduces distortion (tightens the sound)
// - Extends bandwidth
// - Lowers output impedance (affects speaker damping)
//
// Presence and Resonance controls modify this feedback at HF and LF:
// - Presence: reduces NFB at high frequencies → more treble/bite
// - Resonance: reduces NFB at low frequencies → more bass thump
// ============================================================================

float PowerAmp::processNegativeFeedback(float input) {
    // Presence: extract HF from feedback to reduce HF NFB
    presenceState += presenceCoeff * (nfbState - presenceState);
    float hfFeedback = nfbState - presenceState;

    // Resonance: extract LF from feedback to reduce LF NFB
    resonanceState += resonanceCoeff * (nfbState - resonanceState);
    float lfFeedback = resonanceState;

    // Modified feedback signal: reduce NFB in frequency ranges controlled by presence/resonance
    float feedback = nfbState
                   - hfFeedback * presence * 0.8f
                   - lfFeedback * resonance * 0.6f;

    // Apply feedback (typical NFB amount ~6-12dB)
    float nfbAmount = 0.3f;  // ~8dB of feedback
    float output = input - feedback * nfbAmount;

    // Store output for next cycle's feedback
    nfbState = output;

    return output;
}

} // namespace ivan
