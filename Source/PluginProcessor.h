#pragma once
#include <JuceHeader.h>

class HuntericeAbyssProcessor : public juce::AudioProcessor {
public:
    HuntericeAbyssProcessor();
    ~HuntericeAbyssProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    struct Voice {
        bool active = false;
        int midiNote = -1;
        double phase = 0.0;
        double frequency = 0.0;
        double targetAmp = 0.0;
        double sampleRate = 44100.0;
        
        enum class EnvStage { Idle, Attack, Decay, Sustain, Release };
        EnvStage envStage = EnvStage::Idle;
        double envValue = 0.0;
        double attackRate = 0.0;
        double decayRate = 0.0;
        double sustainLevel = 0.0;
        double releaseRate = 0.0;

        void noteOn(int note, double vel, double sr, double att, double dec, double sus, double rel);
        void noteOff();
        void updateEnvRates(double sr, double att, double dec, double sus, double rel);
        double processSample();
    };

    static constexpr int MAX_VOICES = 16;
    Voice voices[MAX_VOICES];
    double sampleRate = 44100.0;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HuntericeAbyssProcessor)
};
