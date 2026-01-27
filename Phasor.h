// Some of this code was copied from Karl Yerkes' starter code at 
// https://github.com/kybr/Badass-Toy/blob/main/PluginProcessor.cpp

#pragma once
#include <cmath>

class Phasor {
    float frequency_;
    const float sample_rate_;
    float offset_;
    float phase_;

    public:
    Phasor(float hertz = 0.0f, float sample_rate = 44100.0f, float offset = 0.0f)
        : frequency_(hertz / sample_rate), sample_rate_(sample_rate), offset_(offset), phase_(0) {}

    Phasor& operator=(Phasor& other) {
        this->frequency_ = other.frequency_;
        this->offset_ = other.offset_;
        this->phase_ = other.phase_;
        return(*this);
    }

    float operator()() 
    {
        return process();
    }

    void frequency(float hertz) 
    {
        frequency_ = hertz / sample_rate_;
    }

    float process() {
        if (phase_ >= 1.0f) {
            phase_ -= 1.0f;
        }
        float output = phase_ + offset_;
        if (output >= 1.0f) {
            output -= 1.0f;
        }

        phase_ += frequency_; 
        return output;
    }

    void incFrequency(float changeHz)
    {
        frequency_ += changeHz / sample_rate_;
    }
};


inline float sin7(float x) {
    // 7 multiplies + 7 addition/subtraction
    // 14 operations
    double result = x * (x * (x * (x * (x * (x * (66.5723768716453 * x - 233.003319050759) + 275.754490892928) - 106.877929605423) + 0.156842000875713) - 9.85899292126983) + 7.25653181200263) - (8.88178419700125e-16);
    return((float)(result));
}

inline float scaleBeta(float omega)
{
    float x = 0.5f - omega;
    return 13.0f * x * x * x * x;
}

inline float phaseWrap(float phase) {
    return(phase - std::floor(phase));
}