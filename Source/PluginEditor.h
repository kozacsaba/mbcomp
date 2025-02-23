/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "EditorComponent.h"

//==============================================================================
/**
*/
class MBComp01AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    MBComp01AudioProcessorEditor (MBComp01AudioProcessor&);
    ~MBComp01AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MBComp01AudioProcessor& audioProcessor;

    headComponent head;
    bodyComponent body;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MBComp01AudioProcessorEditor)
};