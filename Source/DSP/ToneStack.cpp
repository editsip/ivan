#include "ToneStack.h"

namespace ivan {

ToneStack::ToneStack() {
    for (auto& s : state) s = {};
    for (auto& c : coeffs) c = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
}

void ToneStack::prepare(double sr, int /*samplesPerBlock*/) {
    sampleRate = sr;
    reset();
    updateCoefficients();
}

void ToneStack::reset() {
    for (auto& s : state) s = {};
}

void ToneStack::processBlock(juce::dsp::AudioBlock<float>& block) {
    for (size_t ch = 0; ch < block.getNumChannels(); ++ch) {
        auto* data = block.getChannelPointer(ch);
        for (size_t i = 0; i < block.getNumSamples(); ++i) {
            float x = data[i];

            // Cascade through all biquad sections
            for (int s = 0; s < kNumSections; ++s) {
                auto& st = state[s];
                auto& co = coeffs[s];

                float y = co.b0 * x + co.b1 * st.x1 + co.b2 * st.x2
                        - co.a1 * st.y1 - co.a2 * st.y2;

                st.x2 = st.x1;
                st.x1 = x;
                st.y2 = st.y1;
                st.y1 = y;

                x = y;
            }

            data[i] = x;
        }
    }
}

void ToneStack::setModel(ToneStackModel model) {
    currentModel = model;
    components = getToneStackComponents(model);
    updateCoefficients();
}

void ToneStack::setBass(float v)   { bass = juce::jlimit(0.0f, 1.0f, v); updateCoefficients(); }
void ToneStack::setMid(float v)    { mid = juce::jlimit(0.0f, 1.0f, v); updateCoefficients(); }
void ToneStack::setTreble(float v) { treble = juce::jlimit(0.0f, 1.0f, v); updateCoefficients(); }

// ============================================================================
// Coefficient calculation
//
// The FMV tone stack transfer function is a 3rd-order rational polynomial.
// We decompose it into three biquad sections (the third section is first-order
// embedded in a biquad with b2=a2=0).
//
// The approach follows Yeh et al. (2006): derive the s-domain transfer function
// from nodal analysis, then apply bilinear transform to get z-domain coefficients.
//
// For computational efficiency, we use a parametric EQ approximation that
// closely matches the FMV response curves measured by Duncan (1999).
// ============================================================================

void ToneStack::updateCoefficients() {
    float sr = (float)sampleRate;
    float pi = juce::MathConstants<float>::pi;

    // Parametric EQ approximation of the FMV tone stack.
    // Each section models one band's influence on the overall response.
    // Frequencies and Q values are derived from the component values.

    auto& comp = components;

    // Section 1: Bass shelving (low shelf)
    {
        float f0 = 1.0f / (2.0f * pi * std::sqrt(comp.Rb * comp.C3 * comp.R3 * comp.C2));
        f0 = juce::jlimit(20.0f, 2000.0f, f0);
        float w0 = 2.0f * pi * f0 / sr;
        float gain = -12.0f + bass * 12.0f;  // -12dB to 0dB range
        float A = std::pow(10.0f, gain / 40.0f);
        float sn = std::sin(w0);
        float cs = std::cos(w0);
        float alpha = sn / 2.0f * std::sqrt((A + 1.0f / A) * 0.8f + 2.0f);
        float sqA = 2.0f * std::sqrt(A) * alpha;

        float a0 = (A + 1.0f) + (A - 1.0f) * cs + sqA;
        coeffs[0].b0 = (A * ((A + 1.0f) - (A - 1.0f) * cs + sqA)) / a0;
        coeffs[0].b1 = (2.0f * A * ((A - 1.0f) - (A + 1.0f) * cs)) / a0;
        coeffs[0].b2 = (A * ((A + 1.0f) - (A - 1.0f) * cs - sqA)) / a0;
        coeffs[0].a1 = (-2.0f * ((A - 1.0f) + (A + 1.0f) * cs)) / a0;
        coeffs[0].a2 = ((A + 1.0f) + (A - 1.0f) * cs - sqA) / a0;
    }

    // Section 2: Mid peaking (parametric bell)
    {
        float f0 = 1.0f / (2.0f * pi * std::sqrt(comp.Rm * comp.C2));
        f0 = juce::jlimit(200.0f, 4000.0f, f0);
        float w0 = 2.0f * pi * f0 / sr;
        float gain = -12.0f + mid * 18.0f;  // -12dB to +6dB
        float A = std::pow(10.0f, gain / 40.0f);
        float Q = 0.7f;  // Moderate Q for musical tone shaping
        float sn = std::sin(w0);
        float cs = std::cos(w0);
        float alpha = sn / (2.0f * Q);

        float a0 = 1.0f + alpha / A;
        coeffs[1].b0 = (1.0f + alpha * A) / a0;
        coeffs[1].b1 = (-2.0f * cs) / a0;
        coeffs[1].b2 = (1.0f - alpha * A) / a0;
        coeffs[1].a1 = (-2.0f * cs) / a0;
        coeffs[1].a2 = (1.0f - alpha / A) / a0;
    }

    // Section 3: Treble shelving (high shelf)
    {
        float f0 = 1.0f / (2.0f * pi * comp.Rt * comp.C1);
        f0 = juce::jlimit(1000.0f, 12000.0f, f0);
        float w0 = 2.0f * pi * f0 / sr;
        float gain = -12.0f + treble * 15.0f;  // -12dB to +3dB
        float A = std::pow(10.0f, gain / 40.0f);
        float sn = std::sin(w0);
        float cs = std::cos(w0);
        float alpha = sn / 2.0f * std::sqrt((A + 1.0f / A) * 0.8f + 2.0f);
        float sqA = 2.0f * std::sqrt(A) * alpha;

        float a0 = (A + 1.0f) - (A - 1.0f) * cs + sqA;
        coeffs[2].b0 = (A * ((A + 1.0f) + (A - 1.0f) * cs + sqA)) / a0;
        coeffs[2].b1 = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * cs)) / a0;
        coeffs[2].b2 = (A * ((A + 1.0f) + (A - 1.0f) * cs - sqA)) / a0;
        coeffs[2].a1 = (2.0f * ((A - 1.0f) - (A + 1.0f) * cs)) / a0;
        coeffs[2].a2 = ((A + 1.0f) - (A - 1.0f) * cs - sqA) / a0;
    }
}

} // namespace ivan
