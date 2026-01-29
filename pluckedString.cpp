#include "Phasor.h"
#include "pluckedString.h"
#include <cstdlib>

// Constructor
PluckedString::PluckedString(float sRate, float damp, int wave, size_t tableLength) : sampleRate(sRate)
{
    this->frequencyHz = 440.0;
    Phasor created = Phasor(440.0, sampleRate, 0.0);
    this->wavetable = DelayLine();
    this->wavetable.resize(tableLength);
    this->tableSize = tableLength;
    this->damping = damp;
    this->waveform = wave;
}

void PluckedString::setFrequency(float hertz) { this->frequencyHz = hertz; }
void PluckedString::setDamping(float factor) { this->damping = factor; }
void PluckedString::setWaveform(int wave) { this->waveform = wave; }

// Get the next sample
float PluckedString::operator()() {
    // Delay length in samples (p)
    float delaySamples = (sampleRate) / (this->frequencyHz);

    // Read the two delayed samples
    float y_p   = wavetable.read(delaySamples);
    float y_p_1 = wavetable.read(delaySamples + 1.0f);

    // Karplus–Strong averaging
    float current = damping * 0.5f * (y_p + y_p_1);

    // Write back into delay line
    wavetable.write(current);

    return(current);
}

void PluckedString::pluck() {
    // Random noise
    if (waveform == 0) {
        float randomValue;
        for (size_t i = 0; i < tableSize; i++) {
            randomValue = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2.0f - 1.0f;
            wavetable.write(randomValue);
        }
    }
    // Sine wave
    else if (waveform == 1) {
        Phasor p(frequencyHz, sampleRate, 0.0);
        float phase;
        for (size_t i = 0; i < tableSize; i++) {
            phase = p();
            wavetable.write(sin7(phase));
        }
    }
    // Saw wave 
    else if (waveform == 2) {
        Phasor p(frequencyHz, sampleRate, 0.0);
        for (size_t i = 0; i < tableSize; i++) {
            wavetable.write(p());
        }
    }
}
