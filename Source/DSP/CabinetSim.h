#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>

namespace ivan {

// ============================================================================
// Cabinet & Microphone Simulation
//
// Models the frequency response and resonance characteristics of guitar
// speaker cabinets and microphone placement.
//
// Cabinet types modeled from published impulse response measurements
// and frequency response data (Ownhammer, 3 Sigma Audio research papers).
//
// Speaker models based on Thiele-Small parameters from manufacturer datasheets.
// Microphone models approximate the polar pattern and frequency response
// of classic studio microphones at varying distances and angles.
// ============================================================================

enum class CabinetType {
    Open1x12,     // Open-back 1x12 (Fender Deluxe style)
    Closed2x12,   // Closed-back 2x12 (Mesa, Matchless)
    Closed4x12,   // Closed-back 4x12 (Marshall 1960)
    Open2x12,     // Open-back 2x12 (Fender Twin)
    Open4x10,     // Open-back 4x10 (Fender Bassman)
    Closed1x15,   // Closed-back 1x15 (bass cab)
    IsoBox         // Isolation box (very dry, tight)
};

enum class SpeakerType {
    Celestion_G12M_Greenback,    // Classic Marshall speaker
    Celestion_V30,               // Modern high-gain staple
    Celestion_Blue,              // Vox AC30 speaker
    Celestion_G12H_Anniversary,  // High-power Greenback variant
    Jensen_P12R,                 // Vintage Fender Jensen
    JBL_D120,                    // Clean headroom king
    Eminence_Governor,           // Modern all-rounder
    WGS_Retro30                  // Warehouse Guitar Speakers
};

enum class MicType {
    SM57,          // Shure SM57 — industry standard, presence peak ~5kHz
    MD421,         // Sennheiser MD421 — fuller, less peaky
    R121,          // Royer R121 — ribbon, smooth, dark
    U87,           // Neumann U87 — condenser, detailed, room
    E609,          // Sennheiser e609 — flat on cab, balanced
    C414           // AKG C414 — versatile condenser
};

class CabinetSim {
public:
    CabinetSim();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    void processBlock(juce::dsp::AudioBlock<float>& block);

    // Cabinet selection
    void setCabinetType(CabinetType type);
    void setSpeakerType(SpeakerType type);
    void setMicType(MicType type);

    // Mic positioning (3D)
    void setMicDistance(float distanceCm);     // 0-100cm (0=on cone, 100=room)
    void setMicAngle(float angleDegrees);      // 0-90° (0=on axis, 90=off axis)
    void setMicPosition(float coneToEdge);     // 0-1 (0=center cone, 1=edge/cap)

    // Room
    void setRoomAmount(float amount);          // 0..1 (dry to roomy)

    void setEnabled(bool enabled) { cabEnabled = enabled; }

private:
    void updateFilters();

    // Speaker resonance model (2nd order resonance from Thiele-Small)
    struct SpeakerModel {
        float resonanceFreq;   // Fs (free air resonance)
        float qFactor;         // Qts (total Q at resonance)
        float rolloffFreq;     // Upper frequency rolloff
        float coneBreakup;     // Frequency where cone breakup modes begin
    };

    static SpeakerModel getSpeakerModel(SpeakerType type);

    CabinetType cabType = CabinetType::Closed4x12;
    SpeakerType speakerType = SpeakerType::Celestion_G12M_Greenback;
    MicType micType = MicType::SM57;

    float micDistance = 5.0f;
    float micAngle = 0.0f;
    float micPosition = 0.3f;
    float roomAmount = 0.1f;
    bool cabEnabled = true;

    double sampleRate = 44100.0;

    // Filter chain for cabinet simulation
    // We use a series of biquad filters to approximate the cab+mic response

    // Speaker resonance (bandpass/peak)
    juce::dsp::IIR::Filter<float> speakerResonance;
    juce::dsp::IIR::Coefficients<float>::Ptr speakerResonanceCoeffs;

    // Cabinet body resonance (low shelf / peak)
    juce::dsp::IIR::Filter<float> cabResonance;
    juce::dsp::IIR::Coefficients<float>::Ptr cabResonanceCoeffs;

    // Speaker rolloff (low pass)
    juce::dsp::IIR::Filter<float> speakerRolloff;
    juce::dsp::IIR::Coefficients<float>::Ptr speakerRolloffCoeffs;

    // Mic presence peak / characteristic
    juce::dsp::IIR::Filter<float> micCharacter;
    juce::dsp::IIR::Coefficients<float>::Ptr micCharacterCoeffs;

    // High cut for off-axis / distance
    juce::dsp::IIR::Filter<float> micDistanceFilter;
    juce::dsp::IIR::Coefficients<float>::Ptr micDistanceCoeffs;

    // Simple room (very short delay + low-pass)
    static constexpr int kMaxRoomDelay = 2048;
    std::array<float, kMaxRoomDelay> roomBuffer{};
    int roomWritePos = 0;
    int roomDelaySamples = 0;
    float roomLPstate = 0.0f;
};

} // namespace ivan
