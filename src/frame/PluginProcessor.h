/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "processors/Compressor.h"
#include "processors/Allpass.h"

//==============================================================================
/**
*/
class MBComp01AudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    MBComp01AudioProcessor();
    ~MBComp01AudioProcessor() override;
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    //==============================================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    //==============================================================================
    float getILvl(int);
    float getOLvl(int);
    float getGLvl(int);

    juce::AudioParameterFloat* getat(int band);
    juce::AudioParameterFloat* getrt(int band);
    juce::AudioParameterFloat* getCT(int band);
    juce::AudioParameterFloat* getCR(int band);
    juce::AudioParameterFloat* getpre(int band);
    juce::AudioParameterFloat* getpost(int band);

    juce::AudioParameterFloat* getla();
    juce::AudioParameterFloat* getf0();
    juce::AudioParameterFloat* getf1();

    void setSolo(int soloBand);

private:
    //==============================================================================
    float calculateRMS(float* buffer, int bufferSize) const;
    //==============================================================================
    // different for each band -> array of pointers
    juce::AudioParameterFloat** at;
    juce::AudioParameterFloat** rt;
    juce::AudioParameterFloat** CT;
    juce::AudioParameterFloat** CR;
    juce::AudioParameterFloat** pre;
    juce::AudioParameterFloat** post;

    // global parameters
    juce::AudioParameterFloat* la;
    juce::AudioParameterFloat* f0;
    juce::AudioParameterFloat* f1;

    // internal
    Compressor** comps; // 3 per each channel
    Allpass** filters;  // 2 per each channel
    float** supportBuffer; // used for each channel, 3 buffs / ch
    int supportBufferSize;
    int solo;

    // display
    float* iLvl;
    float* oLvl;
    float* gLvl;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MBComp01AudioProcessor)
};
