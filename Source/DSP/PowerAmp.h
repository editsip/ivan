#pragma once
#include <juce_dsp/juce_dsp.h>
#include "TubeStage.h"

namespace ivan {

// ============================================================================
// Power Amp simulation — push-pull output stage with transformer modeling.
//
// Models key power amp behaviors:
// - Push-pull tube pair (configurable tube type)
// - Output transformer saturation (LF resonance, HF rolloff)
// - Power supply sag (rectifier droop under load)
// - Negative feedback loop (presence/resonance controls)
// - Variac simulation (reduced B+ voltage = earlier breakup)
//
// References:
// - Aiken Amplification: "What Happens When a Tube Amp Clips"
// - Barbour (2012): "Designing Valve Preamps for Guitar and Bass"
// - Blencowe (2009): "Designing Tube Preamps for Guitar and Bass"
// ============================================================================

class PowerAmp {
public:
    PowerAmp();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    void processBlock(juce::dsp::AudioBlock<float>& block);

    // Tube selection
    void setPowerTubeType(TubeType type);

    // Controls
    void setDrive(float driveNormalized);       // 0..1 master volume
    void setPresence(float presenceNormalized);  // 0..1 (negative feedback HF)
    void setResonance(float resonanceNormalized);// 0..1 (negative feedback LF)
    void setSag(float sagAmount);                // 0..1 (power supply droop)
    void setVariac(float varicNormalized);       // 0..1 (B+ voltage scaling, 1=full)
    void setBias(float biasNormalized);          // 0..1 (power tube bias)
    void setClassAB_A(float blend);              // 0=classAB, 1=classA

private:
    // Push-pull processing: split signal into two halves, process, recombine
    float processPushPull(float input);

    // Output transformer model
    float processOutputTransformer(float input);

    // Power supply sag model (envelope follower modulating headroom)
    float processSag(float input);

    // Negative feedback loop
    float processNegativeFeedback(float input);

    TubeStage tubePositive;  // "Top" tube of push-pull pair
    TubeStage tubeNegative;  // "Bottom" tube of push-pull pair

    float drive = 0.5f;
    float presence = 0.5f;
    float resonance = 0.5f;
    float sagAmount = 0.3f;
    float variac = 1.0f;
    float classABlend = 0.0f;  // 0 = AB, 1 = A

    double sampleRate = 44100.0;

    // Sag state (envelope follower)
    float sagEnvelope = 1.0f;
    float sagAttack = 0.0f;
    float sagRelease = 0.0f;

    // Output transformer state
    float transformerLFstate = 0.0f;
    float transformerHFstate = 0.0f;
    float transformerLFcoeff = 0.0f;
    float transformerHFcoeff = 0.0f;

    // Negative feedback state
    float nfbState = 0.0f;

    // Presence/resonance filter states
    float presenceState = 0.0f;
    float resonanceState = 0.0f;
    float presenceCoeff = 0.0f;
    float resonanceCoeff = 0.0f;
};

} // namespace ivan
