#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>

// To help me learn the Editor and UI APIs I used ChatGPT. 
// It generated some boiler plate code and I modified/wrote my own.
// Here are the prompts I used:
// 1. Now let's add sliders to the plugin to change the output DB and frequency
// 2. What is this avpts thing you keep mentioning? my compiler isn't recognizing it
// 3. Let's add a button that gives me a boolean variable I can then use to switch between saw and impulse mode
// 4. Instead of pulse and saw, let's do pulse, saw, and square. I already have the audio processing done, what's an easy way to do the UI

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    // Frequency slider
    freqSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    freqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    freqSlider.setSkewFactorFromMidPoint(1000.0f);
    addAndMakeVisible(freqSlider);

    // Gain slider
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(gainSlider);

    // Virtual filter slider
    filterSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    filterSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(filterSlider);

    // Slider labels
    freqLabel.setText("Frequency", juce::dontSendNotification);
    freqLabel.setJustificationType(juce::Justification::centred);
    freqLabel.attachToComponent(&freqSlider, false);
    addAndMakeVisible(freqLabel);

    gainLabel.setText("Output (dB)", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.attachToComponent(&gainSlider, false);
    addAndMakeVisible(gainLabel);

    filterLabel.setText("Filter", juce::dontSendNotification);
    filterLabel.setJustificationType(juce::Justification::centred);
    filterLabel.attachToComponent(&filterSlider, false);
    addAndMakeVisible(filterLabel);


    // Attachments for sliders
    freqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "frequency", freqSlider);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "outputGain", gainSlider);

    filterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "filter", filterSlider);

    // Waveform selector
    waveformBox.addItem("Saw", 1);
    waveformBox.addItem("Pulse", 2);
    waveformBox.addItem("Square", 3);

    addAndMakeVisible(waveformBox);

    waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        (p.apvts, "waveform", waveformBox);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 300);


}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colour::fromRGB(100, 0, 100));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("$9999 SYNTHESIZER BY JAVIN", getLocalBounds(), juce::Justification::centredBottom, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getLocalBounds().reduced(20);
    auto top = area.removeFromTop(area.getHeight() / 2);

    
    int sliderWidth = top.getWidth() / 3;

    freqSlider.setBounds(top.removeFromLeft(sliderWidth));
    gainSlider.setBounds(top.removeFromLeft(sliderWidth));
    filterSlider.setBounds(top.removeFromLeft(sliderWidth));

    auto bottom = getLocalBounds().reduced(20);
    bottom.removeFromTop(getHeight() / 2); // skip top area with sliders
    auto w = bottom.getWidth() / 4;
    waveformBox.setBounds(bottom.removeFromTop(30));



}
