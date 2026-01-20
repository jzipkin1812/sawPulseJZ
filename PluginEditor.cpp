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
    auto gradient = juce::ColourGradient(
        juce::Colour::fromRGB(12, 12, 56),
        juce::Point<float>(0.0, 0.0),
        juce::Colour::fromRGB(100, 0, 160),
        juce::Point<float>(getWidth(), getHeight()),
        false
    );

    juce::Image gradientImage(juce::Image::ARGB, getWidth(), getHeight(), true);
    juce::Graphics tg(gradientImage);
    tg.setGradientFill(gradient);
    tg.fillAll();
    g.drawImage(gradientImage, getLocalBounds().toFloat());

    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawFittedText("THE DELUXE $9999 SYNTHESIZER BY JAVIN", getLocalBounds(), juce::Justification::centredBottom, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getLocalBounds().reduced(30);
    auto top = area.removeFromTop(int(area.getHeight() / 1.6));

    // Sliders
    int sliderWidth = top.getWidth() / 3;
    freqSlider.setBounds(top.removeFromLeft(sliderWidth));
    gainSlider.setBounds(top.removeFromLeft(sliderWidth));
    filterSlider.setBounds(top.removeFromLeft(sliderWidth));

    // Waveform selector box
    auto bottom = getLocalBounds().reduced(20);
    bottom.removeFromTop(getHeight() / 1.6);
    auto w = bottom.getWidth() / 4;
    waveformBox.setBounds(bottom.removeFromTop(30));
}
