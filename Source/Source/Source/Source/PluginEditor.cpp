#include "PluginProcessor.h"
#include "PluginEditor.h"

HuntericeAbyssEditor::HuntericeAbyssEditor(HuntericeAbyssProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p),
      masterVolAttach(p.apvts, "MASTER_VOL", masterVolSlider),
      attackAttach(p.apvts, "ATTACK", attackSlider),
      decayAttach(p.apvts, "DECAY", decaySlider),
      sustainAttach(p.apvts, "SUSTAIN", sustainSlider),
      releaseAttach(p.apvts, "RELEASE", releaseSlider)
{
    setLookAndFeel(&lookAndFeel);
    
    addAndMakeVisible(masterVolSlider);
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(decaySlider);
    addAndMakeVisible(sustainSlider);
    addAndMakeVisible(releaseSlider);

    masterVolSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    masterVolSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    
    auto setupLinear = [](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    };
    setupLinear(attackSlider); setupLinear(decaySlider); setupLinear(sustainSlider); setupLinear(releaseSlider);

    setSize(600, 400);
}

HuntericeAbyssEditor::~HuntericeAbyssEditor() { setLookAndFeel(nullptr); }

void HuntericeAbyssEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour::fromUInt(0xFF121212));
    g.setColour(juce::Colour::fromUInt(0xFF8B0000));
    g.setFont(32.0f);
    g.drawText("HUNTERICE ABYSS", getLocalBounds().removeFromTop(60).reduced(20), juce::Justification::centredLeft);
    g.setColour(juce::Colour::fromUInt(0xFFE0E0E0));
    g.setFont(14.0f);
    g.drawText("STAGE 2: MINIMAL WORKING FOUNDATION", getLocalBounds().removeFromTop(30).removeFromTop(20).reduced(20, 0), juce::Justification::centredLeft);
}

void HuntericeAbyssEditor::resized() {
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(80);
    masterVolSlider.setBounds(area.removeFromLeft(150).reduced(10));
    area.removeFromLeft(20);
    auto envArea = area;
    attackSlider.setBounds(envArea.removeFromTop(40).reduced(0, 10));
    decaySlider.setBounds(envArea.removeFromTop(40).reduced(0, 10));
    sustainSlider.setBounds(envArea.removeFromTop(40).reduced(0, 10));
    releaseSlider.setBounds(envArea.removeFromTop(40).reduced(0, 10));
}
