#include "CabinetSim.h"

namespace ivan {

CabinetSim::CabinetSim() = default;

void CabinetSim::prepare(double sr, int /*samplesPerBlock*/) {
    sampleRate = sr;

    speakerResonance.reset();
    cabResonance.reset();
    speakerRolloff.reset();
    micCharacter.reset();
    micDistanceFilter.reset();

    roomBuffer.fill(0.0f);
    roomWritePos = 0;
    roomLPstate = 0.0f;

    updateFilters();
}

void CabinetSim::reset() {
    speakerResonance.reset();
    cabResonance.reset();
    speakerRolloff.reset();
    micCharacter.reset();
    micDistanceFilter.reset();
    roomBuffer.fill(0.0f);
    roomWritePos = 0;
    roomLPstate = 0.0f;
}

void CabinetSim::processBlock(juce::dsp::AudioBlock<float>& block) {
    if (!cabEnabled) return;

    for (size_t ch = 0; ch < block.getNumChannels(); ++ch) {
        auto* data = block.getChannelPointer(ch);

        for (size_t i = 0; i < block.getNumSamples(); ++i) {
            float x = data[i];

            // Speaker resonance
            x = speakerResonance.processSample(x);

            // Cabinet body
            x = cabResonance.processSample(x);

            // Speaker rolloff
            x = speakerRolloff.processSample(x);

            // Mic character
            x = micCharacter.processSample(x);

            // Mic distance/angle filtering
            x = micDistanceFilter.processSample(x);

            // Simple room simulation
            if (roomAmount > 0.001f) {
                // Write to delay buffer
                roomBuffer[roomWritePos] = x;
                roomWritePos = (roomWritePos + 1) % kMaxRoomDelay;

                // Read from delay buffer (early reflection)
                int readPos = (roomWritePos - roomDelaySamples + kMaxRoomDelay) % kMaxRoomDelay;
                float roomSig = roomBuffer[readPos];

                // Low-pass the room signal (walls absorb HF)
                float roomLPcoeff = 0.3f;
                roomLPstate += roomLPcoeff * (roomSig - roomLPstate);

                x += roomLPstate * roomAmount * 0.4f;
            }

            data[i] = x;
        }
    }
}

void CabinetSim::setCabinetType(CabinetType type) { cabType = type; updateFilters(); }
void CabinetSim::setSpeakerType(SpeakerType type) { speakerType = type; updateFilters(); }
void CabinetSim::setMicType(MicType type)         { micType = type; updateFilters(); }
void CabinetSim::setMicDistance(float d)           { micDistance = juce::jlimit(0.0f, 100.0f, d); updateFilters(); }
void CabinetSim::setMicAngle(float a)             { micAngle = juce::jlimit(0.0f, 90.0f, a); updateFilters(); }
void CabinetSim::setMicPosition(float p)          { micPosition = juce::jlimit(0.0f, 1.0f, p); updateFilters(); }
void CabinetSim::setRoomAmount(float r)           { roomAmount = juce::jlimit(0.0f, 1.0f, r); }

CabinetSim::SpeakerModel CabinetSim::getSpeakerModel(SpeakerType type) {
    // Values from manufacturer Thiele-Small data and published measurements
    switch (type) {
        case SpeakerType::Celestion_G12M_Greenback:
            return { 75.0f, 0.85f, 5000.0f, 3000.0f };
        case SpeakerType::Celestion_V30:
            return { 80.0f, 0.78f, 5500.0f, 3500.0f };
        case SpeakerType::Celestion_Blue:
            return { 100.0f, 1.1f, 6000.0f, 4000.0f };
        case SpeakerType::Celestion_G12H_Anniversary:
            return { 70.0f, 0.82f, 5200.0f, 3200.0f };
        case SpeakerType::Jensen_P12R:
            return { 95.0f, 1.05f, 5000.0f, 3800.0f };
        case SpeakerType::JBL_D120:
            return { 55.0f, 0.35f, 8000.0f, 5000.0f };
        case SpeakerType::Eminence_Governor:
            return { 70.0f, 0.65f, 6000.0f, 4200.0f };
        case SpeakerType::WGS_Retro30:
            return { 78.0f, 0.80f, 5800.0f, 3600.0f };
        default:
            return { 75.0f, 0.85f, 5000.0f, 3000.0f };
    }
}

void CabinetSim::updateFilters() {
    float sr = (float)sampleRate;
    auto spk = getSpeakerModel(speakerType);

    // 1. Speaker resonance — peak at Fs
    speakerResonanceCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr, spk.resonanceFreq, spk.qFactor, juce::Decibels::decibelsToGain(3.0f));
    speakerResonance.coefficients = speakerResonanceCoeffs;

    // 2. Cabinet body resonance
    // Open-back cabs have a broader, looser LF response
    // Closed-back cabs have tighter, more focused LF
    float cabQ = 0.7f;
    float cabFreq = 120.0f;
    float cabGain = 2.0f;

    switch (cabType) {
        case CabinetType::Open1x12:   cabQ = 0.5f; cabFreq = 110.0f; cabGain = 3.0f; break;
        case CabinetType::Open2x12:   cabQ = 0.5f; cabFreq = 100.0f; cabGain = 3.5f; break;
        case CabinetType::Open4x10:   cabQ = 0.6f; cabFreq = 130.0f; cabGain = 2.5f; break;
        case CabinetType::Closed2x12: cabQ = 0.8f; cabFreq = 120.0f; cabGain = 2.0f; break;
        case CabinetType::Closed4x12: cabQ = 0.9f; cabFreq = 100.0f; cabGain = 4.0f; break;
        case CabinetType::Closed1x15: cabQ = 0.7f; cabFreq = 60.0f;  cabGain = 5.0f; break;
        case CabinetType::IsoBox:     cabQ = 1.2f; cabFreq = 150.0f; cabGain = 1.0f; break;
    }

    cabResonanceCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr, cabFreq, cabQ, juce::Decibels::decibelsToGain(cabGain));
    cabResonance.coefficients = cabResonanceCoeffs;

    // 3. Speaker high-frequency rolloff
    // Adjusted by mic position: center cone = brighter, edge = darker
    float rolloff = spk.rolloffFreq * (1.0f - micPosition * 0.4f);
    rolloff = juce::jlimit(2000.0f, 10000.0f, rolloff);
    speakerRolloffCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, rolloff, 0.707f);
    speakerRolloff.coefficients = speakerRolloffCoeffs;

    // 4. Microphone character
    float micPeakFreq = 5000.0f;
    float micPeakQ = 1.5f;
    float micPeakGain = 4.0f;

    switch (micType) {
        case MicType::SM57:  micPeakFreq = 5000.0f; micPeakQ = 1.5f; micPeakGain = 5.0f; break;
        case MicType::MD421: micPeakFreq = 3000.0f; micPeakQ = 0.8f; micPeakGain = 2.0f; break;
        case MicType::R121:  micPeakFreq = 2000.0f; micPeakQ = 0.6f; micPeakGain = -2.0f; break;
        case MicType::U87:   micPeakFreq = 8000.0f; micPeakQ = 1.0f; micPeakGain = 3.0f; break;
        case MicType::E609:  micPeakFreq = 4000.0f; micPeakQ = 1.0f; micPeakGain = 2.0f; break;
        case MicType::C414:  micPeakFreq = 6000.0f; micPeakQ = 0.8f; micPeakGain = 3.0f; break;
    }

    micCharacterCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr, micPeakFreq, micPeakQ, juce::Decibels::decibelsToGain(micPeakGain));
    micCharacter.coefficients = micCharacterCoeffs;

    // 5. Distance and angle filtering
    // Off-axis: HF drops significantly. Distance: proximity effect (bass boost) and HF loss
    float distanceCutoff = 12000.0f - micDistance * 80.0f - micAngle * 50.0f;
    distanceCutoff = juce::jlimit(2000.0f, 12000.0f, distanceCutoff);
    micDistanceCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, distanceCutoff, 0.707f);
    micDistanceFilter.coefficients = micDistanceCoeffs;

    // Room delay from mic distance (speed of sound ~343 m/s)
    float delaySec = micDistance * 0.01f / 343.0f;  // cm to meters, then time
    roomDelaySamples = juce::jlimit(1, kMaxRoomDelay - 1, (int)(delaySec * sr));
}

} // namespace ivan
