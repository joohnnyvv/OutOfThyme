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

    // DSP loop will go here
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
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"inputGain", 1}, "Input Gain", 0.0f, 1.0f, 0.5f));

    return layout;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OutOfThymeAudioProcessor();
}
