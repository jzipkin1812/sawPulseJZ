#pragma once

#include "Phasor.h"
#include "quasi.h"

class Quasi {
    private:
    int waveform;
    float prevOutput;
    float prevOutput2;
    float virtualFilter;
    Phasor myPhasor;
    const float sampleRate;
    float frequencyHz;

    public:
    // Constructor
    Quasi(float sRate, float virtualFilter = 0.5, float hertz = 0.0, int waveform = 0);
    // Change the frequency
    void setFrequency(float hertz);

    // Change the virtual filter 
    void setVirtualFilter(float t);

    // Change the waveform
    void setWaveform(int w);

    // Get the next sample
    float operator()();

    // Reset the oscillator at the beginning of MIDI note input
    void resetOscillator();
};