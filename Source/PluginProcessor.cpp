#include "PluginProcessor.h"
#include "PluginEditor.h"

IvanAudioProcessor::IvanAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "IvanParameters", createParameterLayout())
{
    // Cache parameter pointers
    channelParam       = apvts.getRawParameterValue("channel");
    inputGainParam     = apvts.getRawParameterValue("inputGain");
    outputLevelParam   = apvts.getRawParameterValue("outputLevel");
    preampGainParam    = apvts.getRawParameterValue("preampGain");
    bassParam          = apvts.getRawParameterValue("bass");
    midParam           = apvts.getRawParameterValue("mid");
    trebleParam        = apvts.getRawParameterValue("treble");
    presenceParam      = apvts.getRawParameterValue("presence");
    resonanceParam     = apvts.getRawParameterValue("resonance");
    masterParam        = apvts.getRawParameterValue("master");

    preampTube1Param   = apvts.getRawParameterValue("preampTube1");
    preampTube2Param   = apvts.getRawParameterValue("preampTube2");
    powerTubeParam     = apvts.getRawParameterValue("powerTube");

    biasParam          = apvts.getRawParameterValue("bias");
    sagParam           = apvts.getRawParameterValue("sag");
    varicParam         = apvts.getRawParameterValue("variac");
    mismatchParam      = apvts.getRawParameterValue("mismatch");
    classABParam       = apvts.getRawParameterValue("classAB");

    toneStackModelParam = apvts.getRawParameterValue("toneStackModel");

    compEnabledParam   = apvts.getRawParameterValue("compEnabled");
    compThreshParam    = apvts.getRawParameterValue("compThreshold");
    compRatioParam     = apvts.getRawParameterValue("compRatio");
    compAttackParam    = apvts.getRawParameterValue("compAttack");
    compReleaseParam   = apvts.getRawParameterValue("compRelease");
    compMixParam       = apvts.getRawParameterValue("compMix");

    gateEnabledParam   = apvts.getRawParameterValue("gateEnabled");
    gateThreshParam    = apvts.getRawParameterValue("gateThreshold");
    gateAttackParam    = apvts.getRawParameterValue("gateAttack");
    gateReleaseParam   = apvts.getRawParameterValue("gateRelease");

    cabEnabledParam    = apvts.getRawParameterValue("cabEnabled");
    cabTypeParam       = apvts.getRawParameterValue("cabType");
    speakerTypeParam   = apvts.getRawParameterValue("speakerType");
    micTypeParam       = apvts.getRawParameterValue("micType");
    micDistanceParam   = apvts.getRawParameterValue("micDistance");
    micAngleParam      = apvts.getRawParameterValue("micAngle");
    micPositionParam   = apvts.getRawParameterValue("micPosition");
    roomAmountParam    = apvts.getRawParameterValue("roomAmount");

    oversamplingParam  = apvts.getRawParameterValue("oversampling");
}

IvanAudioProcessor::~IvanAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout IvanAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Channel selection
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "channel", "Channel", juce::StringArray{"Clean", "Crunch", "Lead"}, 0));

    // Main controls
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "inputGain", "Input Gain", juce::NormalisableRange<float>(-20.0f, 20.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "preampGain", "Gain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "master", "Master", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "outputLevel", "Output Level", juce::NormalisableRange<float>(-40.0f, 10.0f, 0.1f), 0.0f));

    // Tone stack
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "bass", "Bass", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mid", "Mid", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "treble", "Treble", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "presence", "Presence", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "resonance", "Resonance", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "toneStackModel", "Tone Stack", juce::StringArray{"Marshall", "Fender", "Vox", "Mesa", "Hiwatt"}, 0));

    // Tube selection
    juce::StringArray preampTubes{"12AX7", "12AU7", "12AT7", "EF86"};
    juce::StringArray powerTubes{"6V6", "EL84", "6L6", "EL34", "KT88", "6550"};

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "preampTube1", "Preamp Tube 1", preampTubes, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "preampTube2", "Preamp Tube 2", preampTubes, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "powerTube", "Power Tube", powerTubes, 3));  // Default EL34

    // Advanced / Under the hood
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "bias", "Bias", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "sag", "Sag", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "variac", "Variac", juce::NormalisableRange<float>(0.3f, 1.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mismatch", "Tube Mismatch", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "classAB", "Class A/AB", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

    // Compressor
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "compEnabled", "Comp Enabled", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compThreshold", "Comp Threshold", juce::NormalisableRange<float>(-60.0f, 0.0f, 0.5f), -20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compRatio", "Comp Ratio", juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 0.5f), 4.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compAttack", "Comp Attack", juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 0.4f), 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compRelease", "Comp Release", juce::NormalisableRange<float>(10.0f, 2000.0f, 1.0f, 0.3f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compMix", "Comp Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));

    // Noise gate
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "gateEnabled", "Gate Enabled", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gateThreshold", "Gate Threshold", juce::NormalisableRange<float>(-96.0f, 0.0f, 0.5f), -40.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gateAttack", "Gate Attack", juce::NormalisableRange<float>(0.01f, 50.0f, 0.01f, 0.4f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gateRelease", "Gate Release", juce::NormalisableRange<float>(5.0f, 2000.0f, 1.0f, 0.3f), 100.0f));

    // Cabinet
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "cabEnabled", "Cab Enabled", true));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "cabType", "Cabinet", juce::StringArray{
            "Open 1x12", "Closed 2x12", "Closed 4x12", "Open 2x12",
            "Open 4x10", "Closed 1x15", "Iso Box"}, 2));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "speakerType", "Speaker", juce::StringArray{
            "Greenback", "V30", "Blue", "G12H-30",
            "Jensen P12R", "JBL D120", "Governor", "Retro 30"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "micType", "Microphone", juce::StringArray{
            "SM57", "MD421", "R121", "U87", "e609", "C414"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "micDistance", "Mic Distance", juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "micAngle", "Mic Angle", juce::NormalisableRange<float>(0.0f, 90.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "micPosition", "Mic Position", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "roomAmount", "Room", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.1f));

    // Quality
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "oversampling", "Quality", juce::StringArray{"Draft (2x)", "Normal (4x)", "HQ (8x)"}, 1));

    return { params.begin(), params.end() };
}

void IvanAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    signalChain.setOversamplingQuality(
        static_cast<ivan::OversamplingQuality>(int(*oversamplingParam)));
    signalChain.prepare(sampleRate, samplesPerBlock);
    updateParametersFromAPVTS();
}

void IvanAudioProcessor::releaseResources() {
    signalChain.reset();
}

bool IvanAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void IvanAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    updateParametersFromAPVTS();
    signalChain.processBlock(buffer);
}

void IvanAudioProcessor::updateParametersFromAPVTS() {
    // Channel
    signalChain.setChannel(int(*channelParam));

    // Main levels
    signalChain.setInputGain(*inputGainParam);
    signalChain.setOutputLevel(*outputLevelParam);
    signalChain.getPreamp().setGain(*preampGainParam);

    // Tone stack
    auto& ts = signalChain.getToneStack();
    ts.setBass(*bassParam);
    ts.setMid(*midParam);
    ts.setTreble(*trebleParam);
    ts.setModel(static_cast<ivan::ToneStackModel>(int(*toneStackModelParam)));

    // Power amp
    auto& pa = signalChain.getPowerAmp();
    pa.setDrive(*masterParam);
    pa.setPresence(*presenceParam);
    pa.setResonance(*resonanceParam);
    pa.setBias(*biasParam);
    pa.setSag(*sagParam);
    pa.setVariac(*varicParam);
    pa.setClassAB_A(*classABParam);

    // Tube selection
    auto mapPreampTube = [](int idx) -> ivan::TubeType {
        switch (idx) {
            case 0: return ivan::TubeType::ECC83_12AX7;
            case 1: return ivan::TubeType::ECC82_12AU7;
            case 2: return ivan::TubeType::ECC81_12AT7;
            case 3: return ivan::TubeType::EF86;
            default: return ivan::TubeType::ECC83_12AX7;
        }
    };
    auto mapPowerTube = [](int idx) -> ivan::TubeType {
        switch (idx) {
            case 0: return ivan::TubeType::_6V6;
            case 1: return ivan::TubeType::EL84;
            case 2: return ivan::TubeType::_6L6;
            case 3: return ivan::TubeType::EL34;
            case 4: return ivan::TubeType::KT88;
            case 5: return ivan::TubeType::_6550;
            default: return ivan::TubeType::EL34;
        }
    };

    signalChain.getPreamp().setPreampTube1(mapPreampTube(int(*preampTube1Param)));
    signalChain.getPreamp().setPreampTube2(mapPreampTube(int(*preampTube2Param)));
    signalChain.getPreamp().setMismatch(*mismatchParam);
    pa.setPowerTubeType(mapPowerTube(int(*powerTubeParam)));

    // Compressor
    auto& comp = signalChain.getCompressor();
    comp.setEnabled(*compEnabledParam > 0.5f);
    comp.setThreshold(*compThreshParam);
    comp.setRatio(*compRatioParam);
    comp.setAttack(*compAttackParam);
    comp.setRelease(*compReleaseParam);
    comp.setMix(*compMixParam);

    // Noise gate
    auto& gate = signalChain.getNoiseGate();
    gate.setEnabled(*gateEnabledParam > 0.5f);
    gate.setThreshold(*gateThreshParam);
    gate.setAttack(*gateAttackParam);
    gate.setRelease(*gateReleaseParam);

    // Cabinet
    auto& cab = signalChain.getCabinetSim();
    cab.setEnabled(*cabEnabledParam > 0.5f);
    cab.setCabinetType(static_cast<ivan::CabinetType>(int(*cabTypeParam)));
    cab.setSpeakerType(static_cast<ivan::SpeakerType>(int(*speakerTypeParam)));
    cab.setMicType(static_cast<ivan::MicType>(int(*micTypeParam)));
    cab.setMicDistance(*micDistanceParam);
    cab.setMicAngle(*micAngleParam);
    cab.setMicPosition(*micPositionParam);
    cab.setRoomAmount(*roomAmountParam);
}

void IvanAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void IvanAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* IvanAudioProcessor::createEditor() {
    return new IvanAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new IvanAudioProcessor();
}
