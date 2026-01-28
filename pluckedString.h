#pragma once

#include "Phasor.h"

class PluckedString {
    private:
    DelayLine wavetable;
    const float sampleRate;
    float frequencyHz;
    size_t tableSize;
    float damping;
    int waveform;

    public:
    // Constructor
    PluckedString(float sRate, float damp = 1.0f, int wave = 0, size_t tableLength = 4096);
    // Change the frequency
    void setFrequency(float hertz);
    // Change damping
    void setDamping(float factor);
    // Change waveform for plucks
    void setWaveform(int wave);

    // Get the next sample
    float operator()();

    // Reset the oscillator at the beginning of MIDI note input
    void pluck();
};