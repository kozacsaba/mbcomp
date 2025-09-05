/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "EditorComponent.h"

//==============================================================================
/**
*/
class MBComp01AudioProcessorEditor  : public juce::AudioProcessorEditor
{
juce::String HEADER_TEXT = "Multi-Comp by Koza";

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