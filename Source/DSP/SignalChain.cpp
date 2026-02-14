#include "SignalChain.h"

namespace ivan {

SignalChain::SignalChain() {
    createOversampler();
}

void SignalChain::prepare(double sr, int maxBlock) {
    sampleRate = sr;
    maxBlockSize = maxBlock;

    // Prepare oversampler
    createOversampler();
    oversampler->initProcessing(size_t(maxBlock));

    float osRate = float(sr) * float(oversampler->getOversamplingFactor());
    int osBlock = maxBlock * int(oversampler->getOversamplingFactor());

    // Prepare modules at their operating rates
    noiseGate.prepare(sr, maxBlock);         // Pre-distortion, native rate
    compressor.prepare(sr, maxBlock);        // Pre-distortion, native rate
    preamp.prepare(double(osRate), osBlock); // Oversampled
    toneStack.prepare(sr, maxBlock);         // Post-preamp, native rate
    powerAmp.prepare(double(osRate), osBlock); // Oversampled
    cabinetSim.prepare(sr, maxBlock);        // Post-power, native rate
}

void SignalChain::reset() {
    noiseGate.reset();
    compressor.reset();
    preamp.reset();
    toneStack.reset();
    powerAmp.reset();
    cabinetSim.reset();
    oversampler->reset();
}

void SignalChain::processBlock(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);

    // === Native rate: input processing ===

    // Input gain
    block.multiplyBy(inputGainLinear);

    // Noise gate (frequency-conscious, before distortion)
    noiseGate.processBlock(block);

    // Compressor (per-channel style)
    compressor.processBlock(block);

    // === Oversampled: nonlinear sections ===
    auto osBlock = oversampler->processSamplesUp(block);

    // Preamp tube stages
    preamp.processBlock(osBlock);

    // Power amp (push-pull, sag, NFB)
    powerAmp.processBlock(osBlock);

    // Downsample back to native rate
    oversampler->processSamplesDown(block);

    // === Native rate: post-distortion processing ===

    // Tone stack
    toneStack.processBlock(block);

    // Cabinet & mic simulation
    cabinetSim.processBlock(block);

    // Output level
    block.multiplyBy(outputLevelLinear);
}

void SignalChain::setInputGain(float dB) {
    inputGainLinear = juce::Decibels::decibelsToGain(dB);
}

void SignalChain::setOutputLevel(float dB) {
    outputLevelLinear = juce::Decibels::decibelsToGain(dB);
}

void SignalChain::setChannel(int ch) {
    currentChannel = juce::jlimit(0, 2, ch);
    preamp.setChannel(currentChannel);

    // Set channel-appropriate compressor style
    switch (currentChannel) {
        case 0: compressor.setStyle(CompressorStyle::Optical); break;
        case 1: compressor.setStyle(CompressorStyle::FET); break;
        case 2: compressor.setStyle(CompressorStyle::VCA); break;
    }
}

void SignalChain::setOversamplingQuality(OversamplingQuality quality) {
    if (quality != osQuality) {
        osQuality = quality;
        // Will be applied on next prepare() call
    }
}

void SignalChain::createOversampler() {
    int factor = getOversamplingFactor(osQuality);
    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        2,       // num channels
        factor,  // 2^factor oversampling
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true     // max quality
    );
}

} // namespace ivan
