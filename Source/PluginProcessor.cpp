#include "PluginProcessor.h"
#include "PluginEditor.h"

OutOfThymeAudioProcessor::OutOfThymeAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

OutOfThymeAudioProcessor::~OutOfThymeAudioProcessor()
{
}

const juce::String OutOfThymeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OutOfThymeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool OutOfThymeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool OutOfThymeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double OutOfThymeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OutOfThymeAudioProcessor::getNumPrograms()
{
    return 1;
}

int OutOfThymeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OutOfThymeAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String OutOfThymeAudioProcessor::getProgramName (int index)
{
    return {};
}

void OutOfThymeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void OutOfThymeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    tapeEngine.prepare(sampleRate, 128.0); // Allocate for 128s to handle extreme slowdowns
}

void OutOfThymeAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OutOfThymeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void OutOfThymeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    float speed = *apvts.getRawParameterValue("tapeSpeed");
    float speedOffset = *apvts.getRawParameterValue("speedSpread");
    tapeEngine.setTapeSpeed(speed, speed + speedOffset);
    
    tapeEngine.setInterpolationMode(*apvts.getRawParameterValue("hiFi") > 0.5f);
    tapeEngine.setFreezeMode(*apvts.getRawParameterValue("freeze") > 0.5f);
    
    float delayCommon = *apvts.getRawParameterValue("delayCoarse") * getSampleRate();
    tapeEngine.setMainDelay(delayCommon, delayCommon);
    
    tapeEngine.setExtraHeadsSpacing(*apvts.getRawParameterValue("spacing") * getSampleRate());
    tapeEngine.setExtraHeadsLevels(*apvts.getRawParameterValue("levels"));
    tapeEngine.setFeedbackGain(*apvts.getRawParameterValue("feedback"));

    float filtVal = *apvts.getRawParameterValue("filter");
    int type = 0;
    float cutoff = 1000.0f;
    
    if (filtVal < 0.45f) 
    {
        type = 1; // LP
        float norm = filtVal / 0.45f;
        cutoff = norm * 15000.0f + 20.0f;
    }
    else if (filtVal > 0.55f)
    {
        type = 2; // HP
        float norm = (filtVal - 0.55f) / 0.45f;
        cutoff = norm * 15000.0f + 20.0f;
    }
    
    tapeEngine.setFilter(type, cutoff, 0.707f);

    float inputGain = *apvts.getRawParameterValue("inputGain");
    float mix = *apvts.getRawParameterValue("mix");
    float outputVolume = *apvts.getRawParameterValue("volume");

    auto* channelDataL = buffer.getWritePointer(0);
    auto* channelDataR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float dryL = channelDataL[sample] * inputGain;
        float dryR = channelDataR ? (channelDataR[sample] * inputGain) : dryL;
        
        float wetL, wetR;
        tapeEngine.processSample(dryL, dryR, wetL, wetR);

        channelDataL[sample] = (dryL * (1.0f - mix) + wetL * mix) * outputVolume;
        if (channelDataR) 
            channelDataR[sample] = (dryR * (1.0f - mix) + wetR * mix) * outputVolume;
    }
}

bool OutOfThymeAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* OutOfThymeAudioProcessor::createEditor()
{
    return new OutOfThymeAudioProcessorEditor (*this);
}

void OutOfThymeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void OutOfThymeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout OutOfThymeAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"inputGain", 1}, "Input Gain", 0.0f, 10.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"tapeSpeed", 1}, "Tape Speed", 0.1f, 2.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"speedSpread", 1}, "Speed Spread", -0.5f, 0.5f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"delayCoarse", 1}, "Delay Coarse", 0.01f, 2.7f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"spacing", 1}, "Spacing", 0.0f, 1.0f, 0.2f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"levels", 1}, "Levels", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"feedback", 1}, "Feedback", 0.0f, 1.2f, 0.3f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"filter", 1}, "Filter Morph", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"mix", 1}, "Dry/Wet Mix", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"volume", 1}, "Output Volume", 0.0f, 2.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"hiFi", 1}, "HiFi Mode", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"freeze", 1}, "Freeze Mode", false));

    return layout;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OutOfThymeAudioProcessor();
}
