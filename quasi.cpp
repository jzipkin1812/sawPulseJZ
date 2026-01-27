#include "Phasor.h"
#include "quasi.h"

// Constructor
Quasi::Quasi(float sRate, float vFilter, float hertz, int wave) : sampleRate(sRate)
{
    prevOutput2 = prevOutput = 0.0f;
    this->waveform = wave;
    this->frequencyHz = hertz;
    Phasor created = Phasor(hertz, sampleRate, 0.0);
    this->myPhasor = created;
    this->virtualFilter = vFilter;

}

// Change the frequency
void Quasi::setFrequency(float hertz) { myPhasor.frequency(hertz); }

// Change the virtual filter 
void Quasi::setVirtualFilter(float t) { this->virtualFilter = t; }

// Change the waveform
void Quasi::setWaveform(int w) { this->waveform = w; }

// Get the next sample
float Quasi::operator()() {
    float phasorOutput = myPhasor();
    float current;

    float omega = frequencyHz / sampleRate;
    float beta  = virtualFilter * scaleBeta(omega);    
    float DC = 0.376f - omega*0.752f; // calculate DC compensation
    float norm = 1.0f - 2.0f*omega; // calculate normalization

    // SAW
    if (waveform == 0) {
        float phase = phaseWrap(phasorOutput + (beta * prevOutput));
        current = (prevOutput + sin7(phase)) * 0.5f;
        current = (current + DC) * norm;

        prevOutput = current;
    }
    // IMPULSE
    else if (waveform == 1) {
        float phase = phaseWrap(phasorOutput + ((beta / 2.0f) * prevOutput * prevOutput));
        current = (prevOutput * 0.45f + sin7(phase) * 0.55f);
        current = (current + DC) * norm;

        prevOutput = current;
    }
    // SQUARE
    else {
        // First saw
        float phase1 = phaseWrap(phasorOutput + (beta * prevOutput));
        float saw1 = (sin7(phase1) + prevOutput) * 0.5f;
        prevOutput = saw1;

        // Second saw
        float phase2 = phaseWrap(phasorOutput + (beta * prevOutput2) + 0.5f);
        float saw2 = (sin7(phase2) + prevOutput2) * 0.5f;
        prevOutput2 = saw2;

        // Subtract
        current = saw1 - saw2;
        current *= norm * 0.6f;
    }

    return(current);
}

void Quasi::resetOscillator() {
    prevOutput = 0.0f;
    prevOutput2 = 0.0f;
}
