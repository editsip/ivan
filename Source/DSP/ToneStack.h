#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>

namespace ivan {

// ============================================================================
// Tone Stack models — derived from circuit analysis of classic amps.
//
// Based on the FMV (Fender/Marshall/Vox) tone stack topology analyzed by
// Duncan (1999, "Tone Stack Calculator") and Yeh et al. (2006, DAFx paper
// "Automated Physical Modeling of Nonlinear Audio Circuits For Real-Time
// Audio Effects").
//
// The FMV stack is a passive R/C network with three pots (Bass, Mid, Treble).
// We model it as a state-variable filter with coefficients derived from the
// component values of each amp type.
// ============================================================================

enum class ToneStackModel {
    Marshall,    // Classic Marshall: mid-scooped, bright, aggressive
    Fender,      // Fender: mid-forward, warm bass, sparkly treble
    Vox,         // Vox: chimey, upper-mid presence
    Mesa,        // Mesa: tight bass, pronounced mids, smooth treble
    Hiwatt       // Hiwatt: flat response, transparent, articulate
};

// Component values for each tone stack (resistors in ohms, caps in farads)
struct ToneStackComponents {
    float R1, R2, R3, R4;  // Fixed resistors
    float C1, C2, C3;       // Capacitors
    float Rt, Rm, Rb;       // Treble, mid, bass pot values
};

constexpr ToneStackComponents getToneStackComponents(ToneStackModel model) {
    switch (model) {
        case ToneStackModel::Marshall:
            // JCM800 tone stack
            return { 33e3f, 22e3f, 25e3f, 56e3f,
                     470e-12f, 22e-9f, 22e-9f,
                     250e3f, 25e3f, 1e6f };
        case ToneStackModel::Fender:
            // Bassman/Twin tone stack
            return { 250e3f, 1e6f, 25e3f, 56e3f,
                     250e-12f, 100e-9f, 47e-9f,
                     250e3f, 10e3f, 1e6f };
        case ToneStackModel::Vox:
            // AC30 "cut" tone control (simplified to 3-band)
            return { 100e3f, 68e3f, 22e3f, 100e3f,
                     47e-12f, 22e-9f, 22e-9f,
                     1e6f, 50e3f, 1e6f };
        case ToneStackModel::Mesa:
            // Mesa/Boogie tone stack
            return { 250e3f, 250e3f, 25e3f, 100e3f,
                     250e-12f, 100e-9f, 47e-9f,
                     250e3f, 25e3f, 1e6f };
        case ToneStackModel::Hiwatt:
            // Hiwatt DR103
            return { 220e3f, 100e3f, 22e3f, 47e3f,
                     470e-12f, 22e-9f, 22e-9f,
                     250e3f, 25e3f, 1e6f };
        default:
            return { 33e3f, 22e3f, 25e3f, 56e3f,
                     470e-12f, 22e-9f, 22e-9f,
                     250e3f, 25e3f, 1e6f };
    }
}

class ToneStack {
public:
    ToneStack();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    void processBlock(juce::dsp::AudioBlock<float>& block);

    void setModel(ToneStackModel model);
    void setBass(float value);      // 0..1
    void setMid(float value);       // 0..1
    void setTreble(float value);    // 0..1

private:
    void updateCoefficients();

    ToneStackModel currentModel = ToneStackModel::Marshall;
    ToneStackComponents components = getToneStackComponents(ToneStackModel::Marshall);

    float bass = 0.5f;
    float mid = 0.5f;
    float treble = 0.5f;

    double sampleRate = 44100.0;

    // State-variable filter implementation (biquad cascade)
    // We use 3 biquad sections to model the tone stack transfer function
    struct BiquadState {
        float x1 = 0, x2 = 0;
        float y1 = 0, y2 = 0;
    };

    static constexpr int kNumSections = 3;
    std::array<BiquadState, kNumSections> state;

    // Biquad coefficients: H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
    struct BiquadCoeffs {
        float b0 = 1, b1 = 0, b2 = 0;
        float a1 = 0, a2 = 0;
    };

    std::array<BiquadCoeffs, kNumSections> coeffs;
};

} // namespace ivan
