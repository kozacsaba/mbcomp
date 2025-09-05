/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "defines.h"

//==============================================================================
MBComp01AudioProcessor::MBComp01AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#endif
    comps(nullptr), filters(nullptr),
    gLvl(new float[4]), iLvl(new float[4]), oLvl(new float[4]),
    supportBuffer(nullptr), supportBufferSize(0), solo(MAS),
    at(new   juce::AudioParameterFloat* [4]),
    rt(new   juce::AudioParameterFloat* [4]),
    CT(new   juce::AudioParameterFloat* [4]),
    CR(new   juce::AudioParameterFloat* [4]),
    pre(new  juce::AudioParameterFloat* [4]),
    post(new juce::AudioParameterFloat* [4])
{
    std::string bandName;
    for (int band = 0; band < 4; band++)
    {
        switch (band)
        {
        case LOW: bandName = "Low"; break;
        case MID: bandName = "Mid"; break;
        case HHI: bandName = "High"; break;
        case MAS: bandName = "Master"; break;
        default: bandName = "ERROR";
        }

        MBComp01AudioProcessor::addParameter(at[band] =
            new juce::AudioParameterFloat("at" + bandName, bandName + "Attack Time", minat, maxat, defat));
        MBComp01AudioProcessor::addParameter(rt[band] =
            new juce::AudioParameterFloat("rt" + bandName, bandName + "Release Time", minat, maxat, defrt));
        MBComp01AudioProcessor::addParameter(CT[band] =
            new juce::AudioParameterFloat("CT" + bandName, bandName + "Treshold", minCT, maxCT, defCT));
        MBComp01AudioProcessor::addParameter(CR[band] =
            new juce::AudioParameterFloat("CR" + bandName, bandName + "Ratio", minCR, maxCR, defCR));

        MBComp01AudioProcessor::addParameter(post[band] =
            new juce::AudioParameterFloat("post" + bandName, bandName + "Post Compression Gain", minpost, maxpost, defpost));
        MBComp01AudioProcessor::addParameter(pre[band] =
            new juce::AudioParameterFloat("pre" + bandName, bandName + "Pre Compression Gain", minpre, maxpre, defpre));

        iLvl[band] = 0;
        gLvl[band] = 0;
        oLvl[band] = 0;
    }

    MBComp01AudioProcessor::addParameter(f0 =
        new juce::AudioParameterFloat("splitf0", "Low", minf, maxf, deff0));
    MBComp01AudioProcessor::addParameter(f1 =
        new juce::AudioParameterFloat("splitf1", "High", minf, maxf, deff1));
    MBComp01AudioProcessor::addParameter(la =
        new juce::AudioParameterFloat("la", "Lookahead", minla, maxla, defla));
}
MBComp01AudioProcessor::~MBComp01AudioProcessor()
{
    delete[] at;
    delete[] rt;
    delete[] CT;
    delete[] CR;

    delete[] pre;
    delete[] post;

    delete[] iLvl;
    delete[] oLvl;
    delete[] gLvl;
}
//==============================================================================
const juce::String MBComp01AudioProcessor::getName() const
{
    return JucePlugin_Name;
}
bool MBComp01AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}
bool MBComp01AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}
bool MBComp01AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}
double MBComp01AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}
int MBComp01AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}
int MBComp01AudioProcessor::getCurrentProgram()
{
    return 0;
}
void MBComp01AudioProcessor::setCurrentProgram (int index)
{
}
const juce::String MBComp01AudioProcessor::getProgramName (int index)
{
    return {};
}
void MBComp01AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}
//==============================================================================
void MBComp01AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // setting up fx modules
    int chnum = getTotalNumInputChannels();
    filters = new Allpass * [chnum];
    comps = new Compressor * [chnum];

    for (int ch = 0; ch < chnum; ch++)
    {
        filters[ch] = new Allpass[2];
        filters[ch][0].setfc(f0);
        filters[ch][1].setfc(f1);
        filters[ch][0].setfs(sampleRate);
        filters[ch][1].setfs(sampleRate);

        comps[ch] = new Compressor[4];
        for (int band = 0; band < 4; band++)
        {
            comps[ch][band].setat(at[band]);
            comps[ch][band].setrt(rt[band]);
            comps[ch][band].setCT(CT[band]);
            comps[ch][band].setCR(CR[band]);

            //if (band == MAS)
            comps[ch][band].setla(la);
            //else
            //    comps[ch][band].setla( nullptr );
            comps[ch][band].setfs(sampleRate);
        }
    }

    // setting up support buffer
    supportBuffer = new float* [3];
    supportBufferSize = 256;
    for (int band = 0; band < 3; band++)
    {
        supportBuffer[band] = new float[supportBufferSize];
    }
}
void MBComp01AudioProcessor::releaseResources()
{
    int chnum = getTotalNumInputChannels();

    for (int ch = 0; ch < chnum; ch++)
    {
        delete[] filters[ch];
        delete[] comps[ch];
    }
    for (int band = 0; band < 3; band++)
        delete[] supportBuffer[band];

    delete[] filters;
    delete[] comps;
    delete[] supportBuffer;
}
#ifndef JucePlugin_PreferredChannelConfigurations
bool MBComp01AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif
void MBComp01AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    //==========================================================================
    // code supplied by framework
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    int bufferSize = buffer.getNumSamples();

    // clearing unpaired output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    //==========================================================================
    // check and adjust supportBuffer size
    if (supportBufferSize != bufferSize)
    {
        for (int band = 0; band < 3; band++)
        {
            delete[] supportBuffer[band];
            supportBuffer[band] = new float[bufferSize];
        }
        supportBufferSize = bufferSize;
    }

    //==========================================================================
    // display :: init levels
    for (int band = 0; band < 4; band++)
    {
        iLvl[band] = 0;
        oLvl[band] = 0;
        gLvl[band] = 0;
    }

    //==========================================================================
    // process audio
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        for (int i = 0; i < bufferSize; i++)
        {
            supportBuffer[LOW][i] = channelData[i];
            supportBuffer[MID][i] = channelData[i];
            // the high band will be filtered from the mid band, not the input
        }

        //======================================================================
        // wire up fx modules to main buffer
        // filters
        filters[channel][0].setIn(channelData);
        filters[channel][1].setIn(supportBuffer[MID]);

        filters[channel][0].setOut(supportBuffer[LOW]);
        filters[channel][0].setNeg(supportBuffer[MID]);
        filters[channel][1].setOut(supportBuffer[MID]);
        filters[channel][1].setNeg(supportBuffer[HHI]);

        // compressors
        for (int band = 0; band < 3; band++)
        {
            comps[channel][band].setInputBuffer(supportBuffer[band]);
            comps[channel][band].setOutputBuffer(supportBuffer[band]);
        }
        comps[channel][MAS].setInputBuffer(channelData);
        comps[channel][MAS].setOutputBuffer(channelData);

        //======================================================================
        // Filtering
        filters[channel][0].process(bufferSize);
        for (int i = 0; i < bufferSize; i++)
        {
            supportBuffer[LOW][i] /= 2;
            supportBuffer[HHI][i] = (supportBuffer[MID][i] /= 2);
        }
        filters[channel][1].process(bufferSize);
        for (int i = 0; i < bufferSize; i++)
        {
            supportBuffer[MID][i] /= 2;
            supportBuffer[HHI][i] /= 2;
        }

        //======================================================================
        // Compression
        for (int band = 0; band < 3; band++)
        {
            for (int i = 0; i < bufferSize; i++)
                supportBuffer[band][i] *= pow(10, *pre[band] / 20);
            iLvl[band] = calculateRMS(supportBuffer[band], bufferSize);
            comps[channel][band].process(bufferSize);
            oLvl[band] = calculateRMS(supportBuffer[band], bufferSize); // EXCLUING POST
            gLvl[band] = comps[channel][band].getGRMS();
        }

        //======================================================================
        // Addition for output (Mixing)
        for (int i = 0; i < bufferSize; i++)
        {
            channelData[i] = 0;
            for (int band = 0; band < 3; band++)
                if(solo == MAS || solo == band) 
                    channelData[i] += supportBuffer[band][i] * pow(10, *post[band] / 20);
        }

        //======================================================================
        // Master compression
        buffer.applyGain( pow(10, *pre[MAS]/20) );
        iLvl[MAS] += buffer.getRMSLevel(channel, 0, bufferSize);
        comps[channel][MAS].process(bufferSize);
        buffer.applyGain( pow(10, *post[MAS]/20) );

        // summing for display
        oLvl[MAS] += buffer.getRMSLevel(channel, 0, bufferSize);
        gLvl[MAS] += comps[channel][MAS].getGRMS();
    }

    // calcuating levels
    for (int band = 0; band < 4; band++)
    {
        iLvl[band] /= totalNumInputChannels;
        oLvl[band] /= totalNumInputChannels;
        gLvl[band] /= totalNumInputChannels;
    }
}
//==============================================================================
bool MBComp01AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}
juce::AudioProcessorEditor* MBComp01AudioProcessor::createEditor()
{
    return new MBComp01AudioProcessorEditor(*this);     // will be used in final version
    //return new juce::GenericAudioProcessorEditor(*this);    // tmp solution
}
//==============================================================================
void MBComp01AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("MBComp"));

    std::string bandName;
    for (int band = 0; band < 4; band++)
    {
        switch (band)
        {
        case 0: bandName = "Low"; break;
        case 1: bandName = "Mid"; break;
        case 2: bandName = "High"; break;
        case 3: bandName = "Master"; break;
        default: bandName = "ERROR";
        }

        xml->setAttribute(juce::String("at"+bandName), (double)*at[band]);
        xml->setAttribute(juce::String("rt"+bandName), (double)*rt[band]);
        xml->setAttribute(juce::String("CT"+bandName), (double)*CT[band]);
        xml->setAttribute(juce::String("CR"+bandName), (double)*CR[band]);
        xml->setAttribute(juce::String("pre" +bandName), (double)*pre [band]);
        xml->setAttribute(juce::String("post"+bandName), (double)*post[band]);
    }

    xml->setAttribute("la", (double)*la);
    xml->setAttribute("f0", (double)*f0);
    xml->setAttribute("f1", (double)*f1);
    copyXmlToBinary(*xml, destData);
}
void MBComp01AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName("MBComp"))
        {
            std::string bandName;
            for (int band = 0; band < 4; band++)
            {
                switch (band)
                {
                case 0: bandName = "Low"; break;
                case 1: bandName = "Mid"; break;
                case 2: bandName = "High"; break;
                case 3: bandName = "Master"; break;
                default: bandName = "ERROR";
                }

                *at[band] = (float)xmlState->getDoubleAttribute(juce::String("at"+bandName), defat);
                *rt[band] = (float)xmlState->getDoubleAttribute(juce::String("rt"+bandName), defrt);
                *CT[band] = (float)xmlState->getDoubleAttribute(juce::String("CT"+bandName), defCT);
                *CR[band] = (float)xmlState->getDoubleAttribute(juce::String("CR"+bandName), defCR);

                *pre[band] = (float)xmlState->getDoubleAttribute(juce::String("pre"+bandName), defpre);
                *post[band] = (float)xmlState->getDoubleAttribute(juce::String("post"+bandName), defpost);
            }

            *la = (float)xmlState->getDoubleAttribute("la", defla);
            *f0 = (float)xmlState->getDoubleAttribute("f0", deff0);
            *f1 = (float)xmlState->getDoubleAttribute("f1", deff1);
        }
    }
}
//==============================================================================
float MBComp01AudioProcessor::getILvl(int band)
{
    return iLvl[band];
}
float MBComp01AudioProcessor::getOLvl(int band)
{
    return oLvl[band];
}
float MBComp01AudioProcessor::getGLvl(int band)
{
    return gLvl[band];
}

juce::AudioParameterFloat* MBComp01AudioProcessor::getat(int band)
{
    return at[band];
}
juce::AudioParameterFloat* MBComp01AudioProcessor::getrt(int band)
{
    return rt[band];
}
juce::AudioParameterFloat* MBComp01AudioProcessor::getCT(int band)
{
    return CT[band];
}
juce::AudioParameterFloat* MBComp01AudioProcessor::getCR(int band)
{
    return CR[band];
}
juce::AudioParameterFloat* MBComp01AudioProcessor::getpre(int band)
{
    return pre[band];
}
juce::AudioParameterFloat* MBComp01AudioProcessor::getpost(int band)
{
    return post[band];
}

juce::AudioParameterFloat* MBComp01AudioProcessor::getla()
{
    return la;
}
juce::AudioParameterFloat* MBComp01AudioProcessor::getf0()
{
    return f0;
}
juce::AudioParameterFloat* MBComp01AudioProcessor::getf1()
{
    return f1;
}

void MBComp01AudioProcessor::setSolo(int soloBand)
{
    solo = soloBand;
}
//==============================================================================
float MBComp01AudioProcessor::calculateRMS(float* buffer, int bufferSize) const
{
    float rms = 0;
    for (int i = 0; i < bufferSize; i++)
    {
        rms += (buffer[i] * buffer[i]);
    }
    rms /= (float)bufferSize;
    return sqrt(rms);
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MBComp01AudioProcessor();
}