#include "Phasor.h"
#include "pluckedString.h"
#include <cstdlib>

// Constructor
PluckedString::PluckedString(float sRate, size_t tableLength) : sampleRate(sRate)
{
    this->frequencyHz = 440.0;
    Phasor created = Phasor(440.0, sampleRate, 0.0);
    this->wavetable = DelayLine();
    this->wavetable.resize(tableLength);
    this->tableSize = tableLength;

}

// Change the frequency
void PluckedString::setFrequency(float hertz) { this->frequencyHz = hertz; }

// Get the next sample
float PluckedString::operator()() {
    // Delay length in samples (p)
    float delaySamples = (sampleRate) / (this->frequencyHz);

    // Read the two delayed samples
    float y_p   = wavetable.read(delaySamples);
    float y_p_1 = wavetable.read(delaySamples + 1.0f);

    // Karplus–Strong averaging (lowpass = decay)
    float current = 0.5f * (y_p + y_p_1);

    // Write back into delay line
    wavetable.write(current);

    return(current);
}

void PluckedString::pluck() {
    float randomValue;
    // This could be wrong or correct
    for (int i = 0; i < tableSize; i++) {
        randomValue = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2.0f - 1.0f;
        wavetable.write(randomValue);
    }
}
