/*
  ==============================================================================

    Compressor.h
    Created: 4 Nov 2023 2:35:27pm
    Author:  Kozaróczy Csaba

  ==============================================================================
*/

#pragma once

#define RMS_A_TIME 5
#define RMS_R_TIME 130

#include<JuceHeader.h>
#include"CircularBuffer.h"
#include"math.h"

class Compressor {
public:
    //==================================================================
    Compressor(float* InputBuffer = nullptr, float* OutputBuffer = nullptr) :
        IBuffer(InputBuffer), OBuffer(OutputBuffer),
        at(nullptr), rt(nullptr), la(nullptr), CT(nullptr), CR(nullptr),
        xrms(0), g(1), target(1), fs(0), grms(0)
    {
    }
    ~Compressor()
    {
    }
    //==================================================================
    void process(int BufferSize)
    {
        float la_time = 1;
        if (la != nullptr)
            la_time = *la;
        delayBuffer.resize(la_time * fs / 1000);
        grms = 0;

        // TIME COEFFS, *1000 bc of [ms]
        // all four coeffs are close, but not equal to 0
        float cat = 1 - exp(-2.2 / fs / *at * 1000);
        float crt = 1 - exp(-2.2 / fs / *rt * 1000);
        float rms_attack = 1 - exp(-1 / fs / RMS_A_TIME * 1000);
        float rms_release = 1 - exp(-1 / fs / RMS_R_TIME * 1000);

        for (int i = 0; i < BufferSize; i++) {
            // smooth xrms function
            float x2 = IBuffer[i] > 0 ? IBuffer[i] : (-1 * IBuffer[i]);
            if (x2 > xrms)
                xrms = (1 - rms_attack) * xrms + rms_attack * x2;
            else
                xrms = (1 - rms_release) * xrms + rms_release * x2;

            float X = 20 * log10(xrms);
            // static compressor characteristic
            float G = (1 - 1 / *CR) * (*CT - X);
            if (G > 0) G = 0;
            target = pow(10, G / 20);          // current gain target

            if (target < g)                    // attack / release ?
                g = (1 - cat) * g + cat * target; // we need to reduce less => attack
            else
                g = (1 - crt) * g + crt * target; // we need to reduce more => release
            // handling circular buffer
            OBuffer[i] = g * delayBuffer.push(IBuffer[i]);
            grms += (g * g);
        }
        grms /= BufferSize;
        grms = sqrt(grms);
    }
    void clearIn(int BufferSize)
    {
        for (int i = 0; i < BufferSize; i++)
            IBuffer[i] = 0;
    }
    void clearOut(int BufferSize)
    {
        for (int i = 0; i < BufferSize; i++)
            OBuffer[i] = 0;
    }
    //==================================================================
    float* getInputBuffer() const
    {
        return IBuffer;
    }
    float* getOutputBuffer() const
    {
        return OBuffer;
    }
    float getGRMS() const
    {
        return grms;
    }
    //==================================================================
    void setInputBuffer(float* bufferPointer)
    {
        IBuffer = bufferPointer;
    }
    void setOutputBuffer(float* bufferPointer)
    {
        OBuffer = bufferPointer;
    }
    void setat(juce::AudioParameterFloat* param_ptr)
    {
        at = param_ptr;
    }
    void setrt(juce::AudioParameterFloat* param_ptr)
    {
        rt = param_ptr;
    }
    void setla(juce::AudioParameterFloat* param_ptr)
    {
        la = param_ptr;
        float la_time = 1;
        if (la != nullptr)
            la_time = *la;
        delayBuffer.resize(la_time * fs / 1000);
    }
    void setCT(juce::AudioParameterFloat* param_ptr)
    {
        CT = param_ptr;
    }
    void setCR(juce::AudioParameterFloat* param_ptr)
    {
        CR = param_ptr;
    }
    void setfs(double SampleRate)
    {
        if (fs < 0) throw("negative sample rate");

        fs = SampleRate;
        delayBuffer.resize(*la * fs / 1000);
    }

private:
    //==================================================================
    juce::AudioParameterFloat* at;
    juce::AudioParameterFloat* rt;
    juce::AudioParameterFloat* la;
    juce::AudioParameterFloat* CT;
    juce::AudioParameterFloat* CR;
    
    float*                  IBuffer;
    float*                  OBuffer;
    CircularBuffer<float>   delayBuffer;

    float xrms;
    float g;
    float target;

    double fs;
    float grms;
};