#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Phasor.h"
#include <iostream>

static float FREQUENCY_HZ = 440.0f;
Phasor phasor(FREQUENCY_HZ);
float prevOutput = 0.0f;

// INPUT HANDLING
juce::AudioProcessorValueTreeState::ParameterLayout
AudioPluginAudioProcessor::createParameterLayout()
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
        juce::StringArray { "Saw", "Pulse" },
        0 // default = Saw
    ));

    return { params.begin(), params.end() };
}


//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                       apvts(*this, nullptr, "PARAMETERS", createParameterLayout())

{
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    int totalNumInputChannels  = getTotalNumInputChannels();
    int totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear output channels with no input
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Parameter processing
    // Set frequency
    FREQUENCY_HZ = apvts.getRawParameterValue("frequency")->load();
    phasor.frequency(FREQUENCY_HZ);
    // Set gain
    float gainDb = apvts.getRawParameterValue("outputGain")->load();
    float gainLinear = juce::Decibels::decibelsToGain(gainDb);
    // Set scalar for harmonics
    float betaSliderScalar = apvts.getRawParameterValue("filter")->load();
    // Which waveform?
    int waveform = apvts.getRawParameterValue("waveform")->load();

    // Actual signal processing
    float omega = FREQUENCY_HZ / SAMPLE_RATE;
    float beta  = betaSliderScalar * scaleBeta(omega);    
    float DC = 0.376f - omega*0.752f; // calculate DC compensation
    float norm = 1.0f - 2.0f*omega; // calculate normalization

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        // SAW
        if (waveform == 0) {
            float phase = phaseWrap(phasor() + (beta * prevOutput));
            float current = (prevOutput + sin7(phase)) * 0.5f;
            current = (current + DC) * norm;

            prevOutput = current;

            for (int channel = 0; channel < totalNumInputChannels; ++channel)
                buffer.getWritePointer(channel)[sample] = (current * gainLinear);
        }
        // IMPULSE
        else if (waveform == 1) {
            float phase = phaseWrap(phasor() + (beta * prevOutput * prevOutput));
            float current = (prevOutput * 0.45f + sin7(phase) * 0.55f);
            current = (current + DC) * norm;

            prevOutput = current;

            for (int channel = 0; channel < totalNumInputChannels; ++channel)
                buffer.getWritePointer(channel)[sample] = (current * gainLinear);
        }
        // SQUARE
        else {
            float phase = 0.0;
        }
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
