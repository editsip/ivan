#include "TubeStage.h"

namespace ivan {

// ============================================================================
// TubeStage
// ============================================================================

TubeStage::TubeStage() = default;

void TubeStage::prepare(double sr, int /*samplesPerBlock*/) {
    sampleRate = sr;
    reset();

    // Cathode bypass cap: RC corner ~ 80Hz for typical 12AX7 stage
    // (1.5k cathode resistor, 25uF cap → f = 1/(2π*R*C) ≈ 4.2Hz)
    // We model it as a HPF that lets AC through
    float bypassFreq = 4.2f;
    bypassCapCoeff = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * bypassFreq / (float)sampleRate);
}

void TubeStage::reset() {
    bypassCapState = 0.0f;
    dcBlockerX1 = 0.0f;
    dcBlockerY1 = 0.0f;
}

float TubeStage::processSample(float input) {
    // Apply input gain
    float x = input * inputGainLinear;

    // Cathode bypass cap processing
    if (bypassCapEnabled)
        x = processBypassCap(x);

    // Apply drive/bias to shift operating point
    // Bias shifts the quiescent point: 0.5 = class AB nominal
    // Below 0.5 = colder (more crossover distortion)
    // Above 0.5 = hotter (richer harmonics, earlier clipping)
    float biasShift = (biasPoint - 0.5f) * 0.4f;
    x = x * (0.5f + drive * 4.0f) + biasShift;

    // Apply mismatch: slight gain/bias offset for paired tubes
    if (mismatch > 0.0f) {
        x *= (1.0f + mismatch * 0.08f);
        x += mismatch * 0.02f;
    }

    // Main waveshaping
    float y = asymmetricWaveshaper(x);

    // Grid conduction — clips positive excursions harder (blocking distortion)
    y -= gridConduction(x) * 0.3f;

    // Apply output gain
    y *= outputGainLinear;

    // DC blocker (leaky integrator)
    float dcOut = y - dcBlockerX1 + dcBlockerCoeff * dcBlockerY1;
    dcBlockerX1 = y;
    dcBlockerY1 = dcOut;

    return dcOut;
}

void TubeStage::processBlock(juce::dsp::AudioBlock<float>& block) {
    for (size_t ch = 0; ch < block.getNumChannels(); ++ch) {
        auto* data = block.getChannelPointer(ch);
        for (size_t i = 0; i < block.getNumSamples(); ++i) {
            data[i] = processSample(data[i]);
        }
    }
}

void TubeStage::setTubeType(TubeType type) {
    currentType = type;
    chars = getTubeCharacteristics(type);
}

void TubeStage::setDrive(float d)        { drive = juce::jlimit(0.0f, 1.0f, d); }
void TubeStage::setBias(float b)         { biasPoint = juce::jlimit(0.0f, 1.0f, b); }
void TubeStage::setMismatch(float m)     { mismatch = juce::jlimit(0.0f, 1.0f, m); }
void TubeStage::setBypassCap(bool e)     { bypassCapEnabled = e; }

void TubeStage::setInputGain(float dB)   {
    inputGainLinear = juce::Decibels::decibelsToGain(dB);
}

void TubeStage::setOutputGain(float dB)  {
    outputGainLinear = juce::Decibels::decibelsToGain(dB);
}

// ============================================================================
// Koren-inspired waveshaper
// The full Koren model solves: Ip = (E1/Kp)^(1/Kvb) * arctan(E1/Kvb)
// where E1 = Vp/Kp * log(1 + exp(Kp * (1/mu + Vg/sqrt(Kvb + Vp^2))))
// For real-time we use a polynomial approximation capturing the key
// characteristics: asymmetric clipping, soft knee, and tube-specific harmonics.
// ============================================================================

float TubeStage::asymmetricWaveshaper(float x) const {
    // Softness controls transition sharpness
    float s = chars.softness;
    float a = chars.asymmetry;

    // Soft clipping with asymmetry
    // Negative half: harder clip (plate current cutoff)
    // Positive half: softer clip (grid conduction region)
    float shaped;

    if (x >= 0.0f) {
        // Positive: tanh-like with asymmetry pushing even harmonics
        float xp = x * (1.0f + a * 0.5f);
        shaped = std::tanh(xp * s + xp * xp * a * 0.5f);
    } else {
        // Negative: sharper cutoff — emulates plate current cutoff
        float xn = x * (1.0f - a * 0.3f);
        shaped = std::tanh(xn * s * 1.2f) * (1.0f - a * 0.15f);
    }

    // Blend in tube mu influence — higher mu tubes produce more gain and compression
    float muNorm = chars.mu / 100.0f;
    shaped *= (0.7f + 0.3f * muNorm);

    return shaped;
}

float TubeStage::gridConduction(float vGrid) const {
    // Grid conduction occurs when the grid goes positive relative to the cathode.
    // This causes grid current to flow, loading the previous stage and causing
    // "blocking distortion" — a distinctive compressed, choked sound.
    if (vGrid > 0.0f) {
        return vGrid * vGrid * 0.5f;  // Quadratic soft onset
    }
    return 0.0f;
}

float TubeStage::processBypassCap(float input) {
    // Simple model: the bypass cap AC-couples the cathode, increasing gain
    // at frequencies above the RC corner. We model the *difference* from
    // bypassed behavior: when enabled, we subtract the slowly-varying DC
    // component, effectively boosting the AC signal.
    bypassCapState += bypassCapCoeff * (input - bypassCapState);
    return input + (input - bypassCapState) * 0.3f;  // 30% gain boost from bypass
}

// ============================================================================
// PreampChain
// ============================================================================

PreampChain::PreampChain() {
    // Default: 12AX7 for both preamp stages
    for (auto& stage : stages) {
        stage.setTubeType(TubeType::ECC83_12AX7);
    }
}

void PreampChain::prepare(double sr, int samplesPerBlock) {
    sampleRate = sr;

    for (auto& stage : stages) {
        stage.prepare(sr, samplesPerBlock);
    }

    // Interstage coupling caps — model the high-pass filtering between stages
    // Typical values: 22nF coupling cap + 1M grid leak = ~7Hz corner
    float couplingFreq = 7.0f;
    float coeff = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * couplingFreq / (float)sr);
    for (auto& c : couplingCapCoeff) c = coeff;
    for (auto& s : couplingCapState) s = 0.0f;

    // Bright cap: ~1kHz shelf boost on first stage input
    float brightFreq = 1200.0f;
    brightCapCoeff = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * brightFreq / (float)sr);
    brightCapState = 0.0f;
}

void PreampChain::reset() {
    for (auto& stage : stages) stage.reset();
    for (auto& s : couplingCapState) s = 0.0f;
    brightCapState = 0.0f;
}

void PreampChain::processBlock(juce::dsp::AudioBlock<float>& block) {
    for (int s = 0; s < activeStages; ++s) {
        stages[s].processBlock(block);

        // Interstage coupling cap (DC blocking between stages)
        if (s < activeStages - 1) {
            for (size_t ch = 0; ch < block.getNumChannels(); ++ch) {
                auto* data = block.getChannelPointer(ch);
                for (size_t i = 0; i < block.getNumSamples(); ++i) {
                    couplingCapState[s] += couplingCapCoeff[s] * (data[i] - couplingCapState[s]);
                    data[i] -= couplingCapState[s];
                }
            }
        }
    }
}

void PreampChain::setChannel(int channel) {
    currentChannel = juce::jlimit(0, 2, channel);

    switch (currentChannel) {
        case 0:  // Clean: single stage, moderate gain
            activeStages = 1;
            stages[0].setDrive(0.3f);
            stages[0].setOutputGain(-3.0f);
            break;
        case 1:  // Crunch: two stages, moderate drive
            activeStages = 2;
            stages[0].setDrive(0.5f);
            stages[0].setOutputGain(-2.0f);
            stages[1].setDrive(0.4f);
            stages[1].setOutputGain(-4.0f);
            break;
        case 2:  // Lead: four cascaded stages, high gain
            activeStages = 4;
            stages[0].setDrive(0.6f);
            stages[0].setOutputGain(0.0f);
            stages[1].setDrive(0.7f);
            stages[1].setOutputGain(-1.0f);
            stages[2].setDrive(0.8f);
            stages[2].setOutputGain(-2.0f);
            stages[3].setDrive(0.5f);
            stages[3].setOutputGain(-6.0f);
            break;
    }
}

void PreampChain::setPreampTube1(TubeType type) {
    stages[0].setTubeType(type);
    if (activeStages > 2) stages[2].setTubeType(type);
}

void PreampChain::setPreampTube2(TubeType type) {
    stages[1].setTubeType(type);
    if (activeStages > 3) stages[3].setTubeType(type);
}

void PreampChain::setGain(float gain) {
    float g = juce::jlimit(0.0f, 1.0f, gain);
    for (int i = 0; i < activeStages; ++i) {
        stages[i].setDrive(g * (0.3f + 0.7f * (float(i) / float(kMaxStages))));
    }
}

void PreampChain::setBias(float bias) {
    for (auto& stage : stages) stage.setBias(bias);
}

void PreampChain::setMismatch(float m) {
    // Apply mismatch asymmetrically across stages
    for (int i = 0; i < kMaxStages; ++i) {
        float stageM = m * ((i % 2 == 0) ? 1.0f : -0.7f);
        stages[i].setMismatch(std::abs(stageM));
    }
}

void PreampChain::setBright(bool enabled) {
    brightEnabled = enabled;
}

} // namespace ivan
