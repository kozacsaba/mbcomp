/*
  ==============================================================================

    Splitter.h
    Created: 4 Nov 2023 2:35:38pm
    Author:  Kozaróczy Csaba

  ==============================================================================
*/

#pragma once

#define _USE_MATH_DEFINES

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>
#include "math.h"

class Allpass {
public:
    //==================================================================
    Allpass(float* InputBuffer = nullptr, float* OutputBuffer = nullptr, float* NegativeOutputBuffer = nullptr) :
        InBuf(InputBuffer), Out(OutputBuffer), NegOut(NegativeOutputBuffer),
        prevOutput(0), prevInput(0)
    {
    }
    ~Allpass()
    {
    }
    //==================================================================
    // This function ADDS the filtered input to the output.
    // To get the clean filtered signal, use clearOut() first!
    void process(int BufferSize)
    {
        const float tmp_const = std::tan(M_PI * *fc / fs);
        const float c = (tmp_const - 1) / (tmp_const + 1);
        float input;  // local copy of input
        float output; // allpass filtered signal

        for (int i = 0; i < BufferSize; i++)
        {
            // in case InBuf == Out
            input = InBuf[i];
            output = -c * prevOutput + c * input + prevInput;
            Out[i] += output;
            if (NegOut != nullptr) NegOut[i] -= output;
            prevInput = input;
            prevOutput = output;
        }
    }
    void clearIn(int BufferSize)
    {
        for (int i = 0; i < BufferSize; i++)
        {
            InBuf[i] = 0;
        }
    }
    void clearOut(int BufferSize)
    {
        for (int i = 0; i < BufferSize; i++)
        {
            Out[i] = 0;
            if (NegOut != nullptr) NegOut[i] = 0;
        }
    }
    //==================================================================
    float* getInputBuffer() const
    {
        return InBuf;
    }
    float* getOutBuffer() const
    {
        return Out;
    }
    float* getNegOutBuf() const
    {
        return NegOut;
    }
    //==================================================================
    void setIn(float* bufferPointer)
    {
        InBuf = bufferPointer;
    }
    void setOut(float* bufferPointer)
    {
        Out = bufferPointer;
    }
    void setNeg(float* bufferPointer)
    {
        NegOut = bufferPointer;
    }
    void setfc(juce::AudioParameterFloat* param_ptr)
    {
        fc = param_ptr;
    }
    void setfs(float sampleRate)
    {
        fs = sampleRate;
    }

private:
    //==================================================================
    juce::AudioParameterFloat* fc;
    
    float* InBuf;
    float* Out;
    float* NegOut;

    float prevOutput;
    float prevInput;
    float fs;
};