#include "PluginProcessor.h"
#include "PluginEditor.h"

// --- Voice Engine ---
void HuntericeAbyssProcessor::Voice::noteOn(int note, double vel, double sr, double att, double dec, double sus, double rel) {
    midiNote = note;
    frequency = 440.0 * std::pow(2.0, (note - 69) / 12.0);
    phase = 0.0;
    targetAmp = vel;
    envStage = EnvStage::Attack;
    envValue = 0.0;
    sampleRate = sr;
    updateEnvRates(sr, att, dec, sus, rel);
    active = true;
}

void HuntericeAbyssProcessor::Voice::noteOff() {
    if (envStage != EnvStage::Idle) envStage = EnvStage::Release;
}

void HuntericeAbyssProcessor::Voice::updateEnvRates(double sr, double att, double dec, double sus, double rel) {
    sampleRate = sr;
    attackRate = (att > 0.001) ? (1.0 / (att * sr)) : 1.0;
    decayRate = (dec > 0.001) ? ((1.0 - sus) / (dec * sr)) : 1.0;
    sustainLevel = sus;
    releaseRate = (rel > 0.001) ? (1.0 / (rel * sr)) : 1.0;
}

double HuntericeAbyssProcessor::Voice::processSample() {
    if (!active) return 0.0;

    switch (envStage) {
        case EnvStage::Attack:
            envValue += attackRate;
            if (envValue >= 1.0) { envValue = 1.0; envStage = EnvStage::Decay; }
            break;
        case EnvStage::Decay:
            envValue -= decayRate;
            if (envValue <= sustainLevel) { envValue = sustainLevel; envStage = EnvStage::Sustain; }
            break;
        case EnvStage::Sustain:
            envValue = sustainLevel;
            break;
        case EnvStage::Release:
            envValue -= releaseRate;
            if (envValue <= 0.0) { envValue = 0.0; envStage = EnvStage::Idle; active = false; }
            break;
        default: break;
    }

    double t = phase;
    double sample = 2.0 * t - 1.0;
    double dt = frequency / sampleRate;
    
    // PolyBLEP Anti-Aliasing
    if (t < dt) {
        double x = t / dt;
        sample += x + x - x * x - 1.0;
    } else if (t > 1.0 - dt) {
        double x = (t - 1.0) / dt;
        sample += x * x + x + x + 1.0;
    }
    
    phase += dt;
    if (phase >= 1.0) phase -= 1.0;

    return sample * envValue * targetAmp;
}

// --- Audio Processor ---
HuntericeAbyssProcessor::HuntericeAbyssProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout()) {}

HuntericeAbyssProcessor::~HuntericeAbyssProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout HuntericeAbyssProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MASTER_VOL", "Master Volume", 0.0f, 1.0f, 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ATTACK", "Attack", 0.001f, 5.0f, 0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DECAY", "Decay", 0.001f, 5.0f, 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SUSTAIN", "Sustain", 0.0f, 1.0f, 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("RELEASE", "Release", 0.001f, 10.0f, 0.5f));
    return { params.begin(), params.end() };
}

void HuntericeAbyssProcessor::prepareToPlay(double sr, int) { sampleRate = sr; }
void HuntericeAbyssProcessor::releaseResources() {}

void HuntericeAbyssProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    float att = apvts.getRawParameterValue("ATTACK")->load();
    float dec = apvts.getRawParameterValue("DECAY")->load();
    float sus = apvts.getRawParameterValue("SUSTAIN")->load();
    float rel = apvts.getRawParameterValue("RELEASE")->load();
    float masterVol = apvts.getRawParameterValue("MASTER_VOL")->load();

    for (auto& v : voices) if (v.active) v.updateEnvRates(sampleRate, att, dec, sus, rel);

    for (const auto metadata : midiMessages) {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            Voice* freeVoice = nullptr;
            for (auto& v : voices) if (!v.active) { freeVoice = &v; break; }
            if (!freeVoice) freeVoice = &voices[0]; // Voice stealing
            freeVoice->noteOn(msg.getNoteNumber(), msg.getFloatVelocity(), sampleRate, att, dec, sus, rel);
        } else if (msg.isNoteOff()) {
            for (auto& v : voices) {
                if (v.active && v.midiNote == msg.getNoteNumber()) { v.noteOff(); break; }
            }
        }
    }

    auto* leftData = buffer.getWritePointer(0);
    auto* rightData = buffer.getWritePointer(1);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        float mix = 0.0f;
        for (auto& v : voices) if (v.active) mix += (float)v.processSample();
        leftData[sample] = mix * masterVol;
        rightData[sample] = mix * masterVol;
    }
}

juce::AudioProcessorEditor* HuntericeAbyssProcessor::createEditor() { return new HuntericeAbyssEditor(*this); }
bool HuntericeAbyssProcessor::hasEditor() const { return true; }
const juce::String HuntericeAbyssProcessor::getName() const { return JucePlugin_Name; }
bool HuntericeAbyssProcessor::acceptsMidi() const { return true; }
bool HuntericeAbyssProcessor::producesMidi() const { return false; }
bool HuntericeAbyssProcessor::isMidiEffect() const { return false; }
double HuntericeAbyssProcessor::getTailLengthSeconds() const { return 0.0; }
int HuntericeAbyssProcessor::getNumPrograms() { return 1; }
int HuntericeAbyssProcessor::getCurrentProgram() { return 0; }
void HuntericeAbyssProcessor::setCurrentProgram(int) {}
const juce::String HuntericeAbyssProcessor::getProgramName(int) { return {}; }
void HuntericeAbyssProcessor::changeProgramName(int, const juce::String&) {}

void HuntericeAbyssProcessor::getStateInformation(juce::MemoryBlock& destData) {
    std::unique_ptr<juce::XmlElement> xml(apvts.copyState().createXml());
    copyXmlToBinary(*xml, destData);
}

void HuntericeAbyssProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new HuntericeAbyssProcessor(); }
