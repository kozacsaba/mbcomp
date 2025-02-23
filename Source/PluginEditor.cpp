/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "EditorComponent.h"

//==============================================================================
MBComp01AudioProcessorEditor::MBComp01AudioProcessorEditor (MBComp01AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), body(p)
{
    head.setText(juce::String("Multi-Comp by Koza"));
    addAndMakeVisible(head);
    addAndMakeVisible(body);

    setSize (500, 305);
}
MBComp01AudioProcessorEditor::~MBComp01AudioProcessorEditor()
{
}
//==============================================================================
void MBComp01AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(BG_COLOUR);
}
void MBComp01AudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromBottom(5.0f);
    head.setBounds(area.removeFromTop(30.0f));
    body.setBounds(area);
}
