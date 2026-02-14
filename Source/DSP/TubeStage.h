#pragma once
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <array>

namespace ivan {

// ============================================================================
// Tube type definitions derived from published SPICE models and datasheets.
// Sources: Koren (1996) improved triode model, Leach (1995) pentode model,
// Dempwolf et al. (2011) "Physically-motivated Triode Model for Circuit Sim."
// Values cross-referenced with Duncan Amps tube data archive.
// ============================================================================

enum class TubeType {
    ECC83_12AX7,   // Classic high-gain preamp triode (mu ~100)
    ECC82_12AU7,   // Medium-gain preamp triode (mu ~17)
    ECC81_12AT7,   // Low-gain preamp triode (mu ~60)
    EF86,          // Pentode preamp (Vox AC15-style)
    _6V6,          // Small bottle power tube (Fender Deluxe)
    EL84,          // British Class A power tube (Vox AC30)
    _6L6,          // American power tube (Fender Twin)
    EL34,          // British power tube (Marshall)
    KT88,          // High-headroom power tube (Hiwatt, bass amps)
    _6550           // American high-power (SVT, Mesa)
};

// Koren triode model parameters
struct TubeCharacteristics {
    float mu;             // Amplification factor
    float kx;             // Koren exponent
    float kg1;            // Grid constant
    float kp;             // Plate constant
    float kvb;            // Knee voltage adjustment
    float vct;            // Cutoff offset voltage

    // Plate dissipation for thermal modeling
    float maxPlateDissipation;  // Watts

    // Derived nonlinearity shape
    float asymmetry;      // Even harmonic content (0 = symmetric, 1 = full rectification)
    float softness;       // How gradually clipping onset occurs
};

constexpr TubeCharacteristics getTubeCharacteristics(TubeType type) {
    switch (type) {
        // Koren model parameters from Norman Koren's "Improved vacuum tube models"
        // and validated against published plate curves
        case TubeType::ECC83_12AX7:
            return { 100.0f, 1.4f, 1060.0f, 600.0f, 300.0f, 0.0f, 1.0f, 0.15f, 0.7f };
        case TubeType::ECC82_12AU7:
            return { 17.0f, 1.4f, 1180.0f, 84.0f, 300.0f, 0.0f, 2.75f, 0.10f, 0.5f };
        case TubeType::ECC81_12AT7:
            return { 60.0f, 1.35f, 460.0f, 300.0f, 300.0f, 0.0f, 2.5f, 0.12f, 0.6f };
        case TubeType::EF86:
            return { 38.0f, 1.3f, 1460.0f, 200.0f, 300.0f, 0.0f, 1.5f, 0.20f, 0.8f };
        case TubeType::_6V6:
            return { 17.0f, 1.35f, 1500.0f, 150.0f, 22.0f, 0.0f, 14.0f, 0.12f, 0.4f };
        case TubeType::EL84:
            return { 19.0f, 1.35f, 820.0f, 130.0f, 24.0f, 0.0f, 12.0f, 0.18f, 0.55f };
        case TubeType::_6L6:
            return { 8.7f, 1.35f, 1460.0f, 60.0f, 24.0f, 0.0f, 30.0f, 0.08f, 0.35f };
        case TubeType::EL34:
            return { 11.0f, 1.35f, 650.0f, 115.0f, 28.0f, 0.0f, 25.0f, 0.16f, 0.5f };
        case TubeType::KT88:
            return { 8.8f, 1.35f, 730.0f, 32.0f, 16.0f, 0.0f, 42.0f, 0.06f, 0.3f };
        case TubeType::_6550:
            return { 7.9f, 1.35f, 890.0f, 48.0f, 20.0f, 0.0f, 35.0f, 0.07f, 0.32f };
        default:
            return { 100.0f, 1.4f, 1060.0f, 600.0f, 300.0f, 0.0f, 1.0f, 0.15f, 0.7f };
    }
}

// ============================================================================
// Single triode stage — waveshaper based on Koren's improved model
// Includes cathode bypass capacitor emulation and grid conduction
// ============================================================================

class TubeStage {
public:
    TubeStage();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    // Process a single sample (for use inside oversampled loop)
    float processSample(float input);

    // Process a buffer
    void processBlock(juce::dsp::AudioBlock<float>& block);

    // Configuration
    void setTubeType(TubeType type);
    void setDrive(float driveNormalized);      // 0..1
    void setBias(float biasNormalized);        // 0..1 (0.5 = nominal)
    void setMismatch(float mismatchAmount);    // 0..1 (for matched/mismatched pair simulation)
    void setBypassCap(bool enabled);           // Cathode bypass capacitor
    void setInputGain(float gainDb);
    void setOutputGain(float gainDb);

    TubeType getCurrentType() const { return currentType; }

private:
    // Koren model waveshaper - attempt to solve the implicit equation
    // using Newton-Raphson iteration (Yeh et al., 2010 DAFx approach)
    float korenWaveshaper(float vGrid, float vPlate) const;

    // Simplified waveshaper with asymmetry for real-time use
    float asymmetricWaveshaper(float x) const;

    // Grid conduction model (grid current when signal exceeds grid voltage)
    float gridConduction(float vGrid) const;

    // Cathode bypass filter (boosts gain at frequencies above RC corner)
    float processBypassCap(float input);

    TubeType currentType = TubeType::ECC83_12AX7;
    TubeCharacteristics chars = getTubeCharacteristics(TubeType::ECC83_12AX7);

    float drive = 0.5f;
    float biasPoint = 0.5f;
    float mismatch = 0.0f;
    float inputGainLinear = 1.0f;
    float outputGainLinear = 1.0f;
    bool bypassCapEnabled = true;

    double sampleRate = 44100.0;

    // Cathode bypass cap state (simple one-pole HPF)
    float bypassCapState = 0.0f;
    float bypassCapCoeff = 0.0f;

    // DC blocker state
    float dcBlockerX1 = 0.0f;
    float dcBlockerY1 = 0.0f;
    static constexpr float dcBlockerCoeff = 0.995f;
};

// ============================================================================
// Preamp chain — multiple cascaded triode stages with interstage coupling
// Based on classic amp topologies (Marshall JCM800, Fender, Vox)
// ============================================================================

class PreampChain {
public:
    PreampChain();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    void processBlock(juce::dsp::AudioBlock<float>& block);

    // Channel-aware configuration
    void setChannel(int channel);  // 0=clean, 1=crunch, 2=lead
    void setPreampTube1(TubeType type);
    void setPreampTube2(TubeType type);
    void setGain(float gainNormalized);
    void setBias(float biasNormalized);
    void setMismatch(float mismatchAmount);
    void setBright(bool enabled);

    static constexpr int kMaxStages = 4;

private:
    std::array<TubeStage, kMaxStages> stages;
    int activeStages = 2;
    int currentChannel = 0;

    // Interstage coupling caps (HPF between stages)
    std::array<float, kMaxStages> couplingCapState{};
    std::array<float, kMaxStages> couplingCapCoeff{};

    // Bright cap (treble bypass on gain pot)
    bool brightEnabled = false;
    float brightCapState = 0.0f;
    float brightCapCoeff = 0.0f;

    double sampleRate = 44100.0;
};

} // namespace ivan
