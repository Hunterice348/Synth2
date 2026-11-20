#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AbyssLookAndFeel : public juce::LookAndFeel_V4 {
public:
    AbyssLookAndFeel() {
        setColour(juce::Slider::thumbColourId, juce::Colour::fromUInt(0xFF8B0000));
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour::fromUInt(0xFF8B0000));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour::fromUInt(0xFF333333));
        setColour(juce::Slider::textBoxTextColourId, juce::Colour::fromUInt(0xFFE0E0E0));
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromUInt(0xFF1A1A1A));
    }
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = 6.0f;
        auto arcRadius = radius - lineW * 0.5f;
        
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour::fromUInt(0xFF333333));
        g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        juce::Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);
        g.setColour(juce::Colour::fromUInt(0xFF8B0000));
        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        juce::Point<float> thumbPoint(bounds.getCentreX() + arcRadius * std::cos(toAngle - juce::MathConstants<float>::halfPi),
                                      bounds.getCentreY() + arcRadius * std::sin(toAngle - juce::MathConstants<float>::halfPi));
        g.setColour(juce::Colour::fromUInt(0xFFE0E0E0));
        g.fillEllipse(juce::Rectangle<float>(lineW * 2.0f, lineW * 2.0f).withCentre(thumbPoint));
    }
};

class HuntericeAbyssEditor : public juce::AudioProcessorEditor {
public:
    HuntericeAbyssEditor(HuntericeAbyssProcessor&);
    ~HuntericeAbyssEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    HuntericeAbyssProcessor& processorRef;
    AbyssLookAndFeel lookAndFeel;
    
    juce::Slider masterVolSlider, attackSlider, decaySlider, sustainSlider, releaseSlider;
    
    juce::AudioProcessorValueTreeState::SliderAttachment masterVolAttach, attackAttach, decayAttach, sustainAttach, releaseAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HuntericeAbyssEditor)
};
