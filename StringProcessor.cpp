#include "StringProcessor.h"
#include "StringEditor.h"
#include "Phasor.h"
#include <iostream>

// INPUT HANDLING
juce::AudioProcessorValueTreeState::ParameterLayout
StringPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Frequency (Hz)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "frequency",
        "Frequency",
        juce::NormalisableRange<float>(20.0f, 2000.0f, 0.01f, 0.5f),
        440.0f
    ));

    // Output gain (dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "outputGain",
        "Output Gain",
        juce::NormalisableRange<float>(-40.0f, 6.0f, 0.01f),
        -20.0f
    ));

    // Filter (scalar)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filter",
        "Filter",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f
    ));

    // Saw, Square, or Impulse?
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "waveform",
        "Waveform",
        juce::StringArray { "Saw", "Pulse", "Square" },
        0 // default = Saw
    ));

    return { params.begin(), params.end() };
}


//==============================================================================
StringPluginAudioProcessor::StringPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                       apvts(*this, nullptr, "PARAMETERS", createParameterLayout()),
                       stringWaveform(44100.0f)

{
    currentNote = -1;
    velocity = 0.0;
}

StringPluginAudioProcessor::~StringPluginAudioProcessor()
{
}

//==============================================================================
const juce::String StringPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StringPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool StringPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool StringPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double StringPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StringPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int StringPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StringPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String StringPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void StringPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void StringPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
    // prevOutput = 0.0f;
    // prevOutput2 = 0.0f;
    // FREQUENCY_HZ = 440.0f;
    // phasor = Phasor(440.0f);
}

void StringPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool StringPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void StringPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    // Handle MIDI input; determine whether we're playing and what note
    for (const auto metadata : midiMessages)
    { 
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            currentNote = msg.getNoteNumber();
            stringWaveform.setFrequency((float)(msg.getMidiNoteInHertz(currentNote)));
            velocity = msg.getVelocity();

            stringWaveform.pluck();
        }
        else if (msg.isNoteOff())
        {
            if (msg.getNoteNumber() == currentNote)
                currentNote = -1;
        }
    }

    // No midi note = no playback
    if (currentNote < 0)
    {
        buffer.clear();
        return;
    }
    
    // Declare channel data
    juce::ScopedNoDenormals noDenormals;
    int totalNumInputChannels  = getTotalNumInputChannels();
    int totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear output channels with no input
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Parameter processing

    // Set gain
    float gainDb = apvts.getRawParameterValue("outputGain")->load();
    float gainLinear = juce::Decibels::decibelsToGain(gainDb);
    float velocityLinear = ((float)(velocity) / 127.0f);
    // Set scalar for harmonics
    float betaSliderScalar = apvts.getRawParameterValue("filter")->load();
    // Which waveform?
    int waveform = (int)(apvts.getRawParameterValue("waveform")->load());

    // Actual signal processing
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float outputSample = stringWaveform();
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            buffer.getWritePointer(channel)[sample] = (outputSample * gainLinear * velocityLinear);

    }
}

//==============================================================================
bool StringPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* StringPluginAudioProcessor::createEditor()
{
    return new StringPluginAudioProcessorEditor (*this);
}

//==============================================================================
// this boiler plate XML saver was copied from
// https://github.com/kybr/Badass-Toy/blob/main/PluginProcessor.cpp
// thanks
void StringPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void StringPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StringPluginAudioProcessor();
}
