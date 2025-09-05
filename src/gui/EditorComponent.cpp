/*
  ==============================================================================

    EditorComponent.cpp
    Created: 24 Nov 2023 4:18:35pm
    Author:  Kozaróczy Csaba

  ==============================================================================
*/

#include "defines.h"
#include "EditorComponent.h"
#include "PluginProcessor.h"

//==============================================================================
// headComponent
headComponent::headComponent() = default;
headComponent::~headComponent() = default;

void headComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkcyan);

    g.setColour(juce::Colours::white);
    g.setFont(fontSize);
    g.drawText(text, getLocalBounds(), juce::Justification::centred, true);
}
void headComponent::resized()
{
    fontSize = juce::jmax(getHeight()-10.0f, 20.0f);
}

void headComponent::setText(juce::String& textToDisplay)
{
    text = textToDisplay;
}
juce::String& headComponent::getText()
{
    return text;
}


//==============================================================================
// bodyComponent
bodyComponent::bodyComponent(MBComp01AudioProcessor& p)
    : audioProcessor(p), bandSelect(p), knobs(p), meters(p)
{
    bandPanel = new localComponent * [4];
    for (int band = 0; band < 4; band++)
    {
        bandPanel[band] = new localComponent(p, band);
    }

    for (int band = 0; band < 4; band++)
    {
        addAndMakeVisible(*bandPanel[band]);
        bandSelect.getButtons()[band].onClick = [this, band]
            // ONCLICK CALLBACK (changing band)
            {
                // Choosing band panel to make show
                int const btn_target = band;
                for (int panel = 0; panel < 4; panel++)
                    bandPanel[panel]->setVisible(btn_target == panel);

                // setting current panel in child components
                meters.setCurBand(btn_target);
                bandSelect.setSelectedBand(btn_target);

                // implementing solo function
                if (knobs.getSolo())
                    audioProcessor.setSolo(btn_target);
                else
                    audioProcessor.setSolo(MAS);

                // dimming inactive band buttons (visual only)
                bandSelect.dim(btn_target);
            };
    }

    knobs.getSoloButton().onClick = [this]
        {
            knobs.toggleSolo();
            knobs.repaint();

            if (knobs.getSolo())
                audioProcessor.setSolo(bandSelect.getSelectedBand());
            else
                audioProcessor.setSolo(MAS);
        };

    addAndMakeVisible(bandSelect);
    addAndMakeVisible(knobs);
    addAndMakeVisible(meters);
};
bodyComponent::~bodyComponent()
{
    for (int band = 0; band < 4; band++)
    {
        delete bandPanel[band];
    }
    delete[] bandPanel;
}

void bodyComponent::paint(juce::Graphics& g)
{
    // INDICATE ERROR
    g.fillAll(juce::Colours::black);
}
void bodyComponent::resized() 
{
    auto area = getLocalBounds();
    auto sectionWidth = area.getWidth() / 4;
    bandSelect.setBounds( area.removeFromLeft( sectionWidth ) );
    knobs.setBounds( area.removeFromLeft( sectionWidth ) );
    meters.setBounds( area.removeFromLeft( sectionWidth ) );
    for (int band = 0; band < 4; band++)
        bandPanel[band]->setBounds(area);
};


//==============================================================================
// bandSelect
bandSelectComponent::bandSelectComponent(MBComp01AudioProcessor& p)
    : audioProcessor(p), selectedBand(MAS)
{
    bands = new juce::TextButton[4];

    bands[MAS].setButtonText("Master");
    bands[LOW].setButtonText("Low");
    bands[MID].setButtonText("Mid");
    bands[HHI].setButtonText("High");

    for (int band = 0; band < 4; band++)
        addAndMakeVisible(bands[band]);

    dim(MAS);
}
bandSelectComponent::~bandSelectComponent()
{
    delete[] bands;
}

void bandSelectComponent::paint(juce::Graphics& g)
{
    g.fillAll(BG_COLOUR);
}
void bandSelectComponent::resized() 
{
    auto area = getLocalBounds();
    auto height = area.getHeight() / 4;
    auto margin = 5;

    for (int band = 0; band < 4; band++)
    {
        bands[band].setBounds( area.removeFromTop( height ).reduced( margin ) );
    }
}

juce::TextButton* bandSelectComponent::getButtons() const
{
    return bands;
}
int bandSelectComponent::getSelectedBand()
{
    return selectedBand;
}
void bandSelectComponent::setSelectedBand(int band)
{
    selectedBand = band;
}
void bandSelectComponent::dim(int selected)
{
    for (int band = 0; band < 4; band++)
    {
        juce::Colour baseColour;
        juce::Colour textColour = juce::Colours::black;
        switch (band)
        {
        case LOW: baseColour = juce::Colours::red;      break;
        case MID: baseColour = juce::Colours::orange;   break;
        case HHI: baseColour = juce::Colours::yellow;   break;
        case MAS: baseColour = juce::Colours::green;    break;
        default: baseColour = juce::Colours::black;
        }

        if (band != selected)
        {
            baseColour = baseColour.withSaturation(0.5).withBrightness(0.3);
            textColour = juce::Colours::white;
        }

        bands[band].setColour(juce::TextButton::buttonColourId, baseColour);
        bands[band].setColour(juce::TextButton::textColourOffId, textColour);
        bands[band].setColour(juce::TextButton::textColourOnId, textColour);
    }
}


//==============================================================================
// knobs
knobsComponent::knobsComponent(MBComp01AudioProcessor& p)
    : audioProcessor(p), soloBool(false)
{
    la.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    f0.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    f1.setSliderStyle(juce::Slider::RotaryVerticalDrag);

    la.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    f0.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    f1.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);

    la.setTextValueSuffix(" ms");
    f0.setTextValueSuffix(" Hz");
    f1.setTextValueSuffix(" Hz");

    la.setNumDecimalPlacesToDisplay(2);
    f0.setNumDecimalPlacesToDisplay(0);
    f1.setNumDecimalPlacesToDisplay(0);

    la.setNormalisableRange(juce::NormalisableRange<double>(minla, maxla));
    f0.setNormalisableRange(juce::NormalisableRange<double>(minf, maxf));
    f1.setNormalisableRange(juce::NormalisableRange<double>(minf, maxf));

    la.setSkewFactorFromMidPoint(sqrt(maxla));
    f0.setSkewFactorFromMidPoint(sqrt(minf * maxf));
    f1.setSkewFactorFromMidPoint(sqrt(minf * maxf));

    la.setPopupDisplayEnabled(true, true, this, -1);
    f0.setPopupDisplayEnabled(true, true, this, -1);
    f1.setPopupDisplayEnabled(true, true, this, -1);

    la.setDoubleClickReturnValue(true, defla);
    f0.setDoubleClickReturnValue(true, deff0);
    f1.setDoubleClickReturnValue(true, deff1);

    laLabel.setText("Lookahead Time", juce::dontSendNotification);
    f0Label.setText("Low Split", juce::dontSendNotification);
    f1Label.setText("High Split", juce::dontSendNotification);

    laLabel.setJustificationType(juce::Justification::centred);
    f0Label.setJustificationType(juce::Justification::centred);
    f1Label.setJustificationType(juce::Justification::centred);

    solo.setButtonText("Solo");

    la.onValueChange = [this] { *(audioProcessor.getla()) = la.getValue(); };
    f0.onValueChange = [this] 
        {
            *(audioProcessor.getf0()) = f0.getValue();
            if (f0.getValue() > f1.getValue())
                f1.setValue(f0.getValue(), juce::dontSendNotification);
        };
    f1.onValueChange = [this] 
        { 
            *(audioProcessor.getf1()) = f1.getValue();
            if (f1.getValue() < f0.getValue())
                f0.setValue(f1.getValue(), juce::dontSendNotification);
        };

    addAndMakeVisible(la);
    addAndMakeVisible(f0);
    addAndMakeVisible(f1);
    addAndMakeVisible(solo);
    addAndMakeVisible(laLabel);
    addAndMakeVisible(f0Label);
    addAndMakeVisible(f1Label);
}
knobsComponent::~knobsComponent() = default;

void knobsComponent::paint(juce::Graphics& g) 
{
    g.fillAll(BG_COLOUR);
    juce::Colour soloColour;
    if (soloBool)
        soloColour = juce::Colours::red;
    else
        soloColour = juce::Colours::darkgrey;
    solo.setColour(juce::TextButton::buttonColourId, soloColour);
}
void knobsComponent::resized() 
{
    auto area = getLocalBounds();

    auto knobAndLabel = area.removeFromTop(area.getHeight() / 2);
    laLabel.setBounds(knobAndLabel.removeFromBottom(CHAR_H));
    la.setBounds(knobAndLabel);

    solo.setBounds( area.removeFromBottom( area.getHeight() / 2 ).reduced(3) );

    knobAndLabel = area.removeFromLeft(area.getWidth() / 2);
    f0Label.setBounds(knobAndLabel.removeFromBottom(CHAR_H));
    f0.setBounds(knobAndLabel);

    f1Label.setBounds(area.removeFromBottom(CHAR_H));
    f1.setBounds( area );

    //laLabel.setBounds(la.getRight() - la.getWidth(), la.getBottom()-10, la.getWidth(), CHAR_H);
}

juce::TextButton& knobsComponent::getSoloButton()
{
    return solo;
}
bool knobsComponent::getSolo()
{
    return soloBool;
}
void knobsComponent::toggleSolo()
{
    soloBool = !soloBool;
}


//==============================================================================
// meters
metersComponent::metersComponent(MBComp01AudioProcessor& p)
    : audioProcessor(p), curBand(MAS)
{
    startTimerHz(24);

    inLabel   .setText("In",   juce::dontSendNotification);
    outLabel  .setText("Out",  juce::dontSendNotification);
    gainLabel .setText("Gain", juce::dontSendNotification);
    scaleLabel.setText("dB",   juce::dontSendNotification);

    inLabel   .setJustificationType(juce::Justification::centred);
    outLabel  .setJustificationType(juce::Justification::centred);
    gainLabel .setJustificationType(juce::Justification::centred);
    scaleLabel.setJustificationType(juce::Justification::centred);

    gain.setInvert(true);
    in.setMark(*(audioProcessor.getCT(MAS)));

    addAndMakeVisible(in);
    addAndMakeVisible(out);
    addAndMakeVisible(gain);
    addAndMakeVisible(scale);

    addAndMakeVisible(inLabel);
    addAndMakeVisible(outLabel);
    addAndMakeVisible(gainLabel);
    addAndMakeVisible(scaleLabel);
};
metersComponent::~metersComponent() = default;

void metersComponent::paint(juce::Graphics& g)
{
    g.fillAll(BG_COLOUR);
}
void metersComponent::resized() 
{ 
    auto area = getLocalBounds();
    auto width = area.getWidth() / 4;

    auto quarter = area.removeFromLeft(width);
    scaleLabel.setBounds(quarter.removeFromBottom(CHAR_H));
    scale.setBounds(quarter);

    quarter = area.removeFromLeft(width);
    inLabel.setBounds(quarter.removeFromBottom(CHAR_H));
    in.setBounds(quarter);

    quarter = area.removeFromLeft(width);
    gainLabel.setBounds(quarter.removeFromBottom(CHAR_H));
    gain.setBounds(quarter);

    quarter = area.removeFromLeft(width);
    outLabel.setBounds(quarter.removeFromBottom(CHAR_H));
    out.setBounds(quarter);
}
void metersComponent::timerCallback()
{
    // update values
    float iDB = juce::Decibels::gainToDecibels(audioProcessor.getILvl(curBand));
    float oDB = juce::Decibels::gainToDecibels(audioProcessor.getOLvl(curBand));
    float gDB = juce::Decibels::gainToDecibels(audioProcessor.getGLvl(curBand));

    in.setLevel(iDB);
    out.setLevel(oDB);
    gain.setLevel(gDB);

    in.setMark(*(audioProcessor.getCT(curBand)));

    in.repaint();
    out.repaint();
    gain.repaint();
}

void metersComponent::setCurBand(int currentBand)
{
    curBand = currentBand;
}


//==============================================================================
// local
localComponent::localComponent(MBComp01AudioProcessor& p, int f_band)
    : audioProcessor(p), band(f_band)
{
    pre .setSliderStyle(juce::Slider::RotaryVerticalDrag);
    post.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    CT  .setSliderStyle(juce::Slider::RotaryVerticalDrag);
    CR  .setSliderStyle(juce::Slider::RotaryVerticalDrag);
    at  .setSliderStyle(juce::Slider::RotaryVerticalDrag);
    rt  .setSliderStyle(juce::Slider::RotaryVerticalDrag);

    pre .setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    post.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    CT  .setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    CR  .setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    at  .setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    rt  .setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);

    pre .setTextValueSuffix(" dB");
    post.setTextValueSuffix(" dB");
    CT  .setTextValueSuffix(" dB");
    CR  .setTextValueSuffix(":1");
    at  .setTextValueSuffix(" ms");
    rt  .setTextValueSuffix(" ms");

    pre .setNumDecimalPlacesToDisplay(0);
    post.setNumDecimalPlacesToDisplay(0);
    CT  .setNumDecimalPlacesToDisplay(0);
    CR  .setNumDecimalPlacesToDisplay(0);
    at  .setNumDecimalPlacesToDisplay(2);
    rt  .setNumDecimalPlacesToDisplay(2);

    pre .setNormalisableRange(juce::NormalisableRange<double>(minpre, maxpre));
    post.setNormalisableRange(juce::NormalisableRange<double>(minpost, maxpost));
    at  .setNormalisableRange(juce::NormalisableRange<double>(minat, maxat));
    rt  .setNormalisableRange(juce::NormalisableRange<double>(minrt, maxrt));
    CT  .setNormalisableRange(juce::NormalisableRange<double>(minCT, maxCT));
    CR  .setNormalisableRange(juce::NormalisableRange<double>(minCR, maxCR));

    at  .setSkewFactorFromMidPoint(sqrt(minat * maxat));
    rt  .setSkewFactorFromMidPoint(sqrt(minrt * maxrt));
    CR  .setSkewFactorFromMidPoint(sqrt(maxCR));

    pre .setPopupDisplayEnabled(true, true, this, -1);
    post.setPopupDisplayEnabled(true, true, this, -1);
    at  .setPopupDisplayEnabled(true, true, this, -1);
    rt  .setPopupDisplayEnabled(true, true, this, -1);
    CT  .setPopupDisplayEnabled(true, true, this, -1);
    CR  .setPopupDisplayEnabled(true, true, this, -1);

    pre .setDoubleClickReturnValue(true, defpre);
    post.setDoubleClickReturnValue(true, defpost);
    at  .setDoubleClickReturnValue(true, defat);
    rt  .setDoubleClickReturnValue(true, defrt);
    CT  .setDoubleClickReturnValue(true, defCT);
    CR  .setDoubleClickReturnValue(true, defCR);

    preLabel .setText("Pre Gain", juce::dontSendNotification);
    postLabel.setText("Post Gain", juce::dontSendNotification);
    atLabel  .setText("Attack", juce::dontSendNotification);
    rtLabel  .setText("Release", juce::dontSendNotification);
    CTLabel  .setText("Threshold", juce::dontSendNotification);
    CRLabel  .setText("Ratio", juce::dontSendNotification);

    preLabel .setJustificationType(juce::Justification::centred);
    postLabel.setJustificationType(juce::Justification::centred);
    atLabel  .setJustificationType(juce::Justification::centred);
    rtLabel  .setJustificationType(juce::Justification::centred);
    CTLabel  .setJustificationType(juce::Justification::centred);
    CRLabel  .setJustificationType(juce::Justification::centred);

    post.onValueChange = [this] { *(audioProcessor.getpost(band)) = post.getValue(); };
    pre .onValueChange = [this] { *(audioProcessor.getpre (band)) = pre .getValue(); };
    at  .onValueChange = [this] { *(audioProcessor.getat  (band)) = at  .getValue(); };
    rt  .onValueChange = [this] { *(audioProcessor.getrt  (band)) = rt  .getValue(); };
    CT  .onValueChange = [this] { *(audioProcessor.getCT  (band)) = CT  .getValue(); };
    CR  .onValueChange = [this] { *(audioProcessor.getCR  (band)) = CR  .getValue(); };

    addAndMakeVisible(pre);
    addAndMakeVisible(post);
    addAndMakeVisible(CT);
    addAndMakeVisible(CR);
    addAndMakeVisible(at);
    addAndMakeVisible(rt);

    addAndMakeVisible(preLabel);
    addAndMakeVisible(postLabel);
    addAndMakeVisible(atLabel);
    addAndMakeVisible(rtLabel);
    addAndMakeVisible(CRLabel);
    addAndMakeVisible(CTLabel);

    switch (band)
    {
    case LOW: background = juce::Colours::red   .withSaturation(0.5);
        textColour = juce::Colours::black; break;
    case MID: background = juce::Colours::orange.withSaturation(0.5); 
        textColour = juce::Colours::black; break;
    case HHI: background = juce::Colours::yellow.withSaturation(0.5); 
        textColour = juce::Colours::black; break;
    case MAS: background = juce::Colours::green .withSaturation(0.5); 
        textColour = juce::Colours::white; break;
    default:  background = juce::Colours::grey;
    }
}
localComponent::~localComponent() = default;

void localComponent::paint(juce::Graphics& g)
{
    g.fillAll(background);
    preLabel .setColour(juce::Label::textColourId, textColour);
    postLabel.setColour(juce::Label::textColourId, textColour);
    atLabel  .setColour(juce::Label::textColourId, textColour);
    rtLabel  .setColour(juce::Label::textColourId, textColour);
    CTLabel  .setColour(juce::Label::textColourId, textColour);
    CRLabel  .setColour(juce::Label::textColourId, textColour);
}
void localComponent::resized() 
{ 
    auto left = getLocalBounds();
    auto right = left.removeFromRight(left.getWidth() / 2);
    auto height = left.getHeight() / 3;

    auto knobAndLabel = left.removeFromTop(height);
    preLabel.setBounds(knobAndLabel.removeFromBottom(CHAR_H));
    pre.setBounds( knobAndLabel );
    knobAndLabel = right.removeFromTop(height);
    postLabel.setBounds(knobAndLabel.removeFromBottom(CHAR_H));
    post.setBounds( knobAndLabel );

    knobAndLabel = left.removeFromTop(height);
    CTLabel.setBounds(knobAndLabel.removeFromBottom(CHAR_H));
    CT.setBounds( knobAndLabel );
    knobAndLabel = right.removeFromTop(height);
    CRLabel.setBounds(knobAndLabel.removeFromBottom(CHAR_H));
    CR.setBounds( knobAndLabel );

    atLabel.setBounds(left.removeFromBottom(CHAR_H));
    at.setBounds( left );
    rtLabel.setBounds(right.removeFromBottom(CHAR_H));
    rt.setBounds( right );
}


//==============================================================================
// barComponent
barComponent::barComponent()
    : min(-80), max(10), cur(-80), mark(100), invert(false),
    top(juce::Colours::darkgrey), bot(juce::Colours::lightgrey)
{
    return;
};
barComponent::~barComponent() = default;

void barComponent::paint(juce::Graphics& g) 
{
    auto area = getLocalBounds().reduced(5).toFloat();

    // full bar
    g.setColour(top);
    g.fillRoundedRectangle(area, 5);

    // current level
    g.setColour(bot);
    if (invert)
        g.fillRoundedRectangle(area.removeFromTop((1 - cur) * area.getHeight()), 5);
    else
        g.fillRoundedRectangle(area.removeFromBottom(cur * area.getHeight()), 5);

    // drawing marker
    if (mark != 100)
    {
        auto area = getLocalBounds().reduced(5).toFloat();
        g.setColour(juce::Colours::red);

        float ycoord = juce::jmap(mark, max, min, 0.0f, 1.0f) * area.getHeight();
        g.drawLine(5, ycoord, area.getWidth()+5, ycoord, 2);
    }
}
void barComponent::resized() { return; }

void barComponent::setLevel(float level_in_dB)
{
    if (level_in_dB > max) level_in_dB = max;
    if (level_in_dB < min) level_in_dB = min;
    cur = juce::jmap(level_in_dB, min, max, 0.0f, 1.0f);
}
void barComponent::setMax(float maxVal)
{
    max = maxVal;
    if (max < min)
        min = max - 10;
}
void barComponent::setMin(float minVal)
{
    min = minVal;
    if (min > max)
        max = min + 10;
}
void barComponent::setMark(float markVal)
{
    mark = markVal;
}
void barComponent::setTopColour(juce::Colour topColour)
{
    top = topColour;
}
void barComponent::setBotColour(juce::Colour botColour)
{
    bot = botColour;
}
void barComponent::setInvert(bool shouldBeInverted)
{
    invert = shouldBeInverted;
}
bool barComponent::getInvert()
{
    return invert;
}


//==============================================================================
// scaleComponent
scaleComponent::scaleComponent() 
{
    dbLabels = new juce::Label[4];

    for (int n = 0; n < 4; n++)
        addAndMakeVisible(dbLabels[n]);
};
scaleComponent::~scaleComponent()
{
    delete[] dbLabels;
}

void scaleComponent::paint(juce::Graphics& g) 
{ 
    auto area = getLocalBounds();
    float w = area.getWidth();
    float h = area.getHeight();
    float d_h = h / 9;
    float d_w = 5;
    g.setColour(juce::Colours::white);

    for (int n = 1; n < 9; n++)
    {
        g.drawLine(w - d_w, h - n * d_h, w, h - n * d_h, 1);
    }

    for (int n = 0; n < 4; n++)
    {
        juce::String labelText;
        labelText += -20 * n;
        dbLabels[n].setText(labelText, juce::dontSendNotification);
        dbLabels[n].setJustificationType(juce::Justification::right);

        float mid_height = (2 * n +1) * d_h;
        dbLabels[n].setBounds(0, mid_height - CHAR_H / 2, 23, CHAR_H);
        dbLabels[n].setFont(10);
    }
};
void scaleComponent::resized() { return; }