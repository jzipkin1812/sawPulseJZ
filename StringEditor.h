#pragma once

#include "StringProcessor.h"

//==============================================================================
class StringPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit StringPluginAudioProcessorEditor (StringPluginAudioProcessor&);
    ~StringPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    juce::Slider freqSlider;
    juce::Slider gainSlider;
    juce::Slider filterSlider;

    juce::ComboBox waveformBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterAttachment;

    juce::Label freqLabel;
    juce::Label gainLabel;
    juce::Label filterLabel;



private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    StringPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StringPluginAudioProcessorEditor)
};
