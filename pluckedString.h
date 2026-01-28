#pragma once

#include "Phasor.h"

class PluckedString {
    private:
    DelayLine wavetable;
    const float sampleRate;
    float frequencyHz;
    size_t tableSize;

    public:
    // Constructor
    PluckedString(float sRate, size_t tableLength = 4096);
    // Change the frequency
    void setFrequency(float hertz);

    // Get the next sample
    float operator()();

    // Reset the oscillator at the beginning of MIDI note input
    void pluck();
};