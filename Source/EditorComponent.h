/*
  ==============================================================================

    EditorComponent.h
    Created: 24 Nov 2023 4:18:35pm
    Author:  Kozaróczy Csaba

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "defines.h"
#include "PluginProcessor.h"

class scaleComponent : public juce::Component
{
public:
    scaleComponent();
    ~scaleComponent();
    //==========================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    //==========================================================================
private:
    juce::Label* dbLabels;
};
class barComponent : public juce::Component
{
public:
    barComponent();
    ~barComponent();
    //==========================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    //==========================================================================
    void setLevel(float level_in_dB);
    void setMax(float maxVal);
    void setMin(float minVal);
    void setMark(float markVal);
    void setTopColour(juce::Colour topColour);
    void setBotColour(juce::Colour botColour);
    void setInvert(bool shouldBeInverted);
    bool getInvert();
    //==========================================================================
private:
    float min, max, cur, mark;
    juce::Colour top, bot;
    bool invert;
};

class localComponent : public juce::Component
{
public:
    localComponent(MBComp01AudioProcessor& p, int f_band);
    ~localComponent();
    //==========================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    //==========================================================================
private:
    MBComp01AudioProcessor& audioProcessor;

    juce::Colour background, textColour;
    juce::Slider pre, post, CT, CR, at, rt;
    juce::Label preLabel, postLabel, CTLabel, CRLabel, atLabel, rtLabel;
    int band;
};
class bandSelectComponent : public juce::Component
{
public:
    bandSelectComponent(MBComp01AudioProcessor& p);
    ~bandSelectComponent();
    //==========================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;
    //==========================================================================
    juce::TextButton* getButtons() const;
    int getSelectedBand();
    void setSelectedBand(int band);
    void dim(int selected);
    //==========================================================================
private:
    MBComp01AudioProcessor& audioProcessor;
    juce::TextButton* bands;
    int selectedBand;
};
class knobsComponent : public juce::Component
{
public:
    knobsComponent(MBComp01AudioProcessor& p);
    ~knobsComponent();
    //==========================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;
    //==========================================================================
    juce::TextButton& getSoloButton();
    bool getSolo();
    void toggleSolo();
    //==========================================================================
private:
    MBComp01AudioProcessor& audioProcessor;
    juce::Slider la, f0, f1;
    juce::TextButton solo;
    juce::Label laLabel, f0Label, f1Label;
    bool soloBool;
};
class metersComponent : public juce::Component,
                        public juce::Timer
{
public:
    metersComponent(MBComp01AudioProcessor& p);
    ~metersComponent();
    //==========================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    //==========================================================================
    void setCurBand(int currentBand);
    //==========================================================================
private:
    MBComp01AudioProcessor& audioProcessor;
    scaleComponent scale;
    barComponent in, gain, out;
    juce::Label inLabel, gainLabel, outLabel, scaleLabel;
    int curBand;
};

class headComponent : public juce::Component
{
public:
    headComponent();
    ~headComponent() override;
    //==========================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    //==========================================================================
    void setText(juce::String& textToDisplay);
    juce::String& getText();
    //==========================================================================
private:
    juce::String text;
    float fontSize;
};
class bodyComponent : public juce::Component
{
public:
    bodyComponent(MBComp01AudioProcessor& p);
    ~bodyComponent();
    //==========================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    //==========================================================================
private:
    MBComp01AudioProcessor& audioProcessor;

    bandSelectComponent bandSelect;
    knobsComponent knobs;
    metersComponent meters;
    localComponent** bandPanel;
};