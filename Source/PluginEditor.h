#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class OutOfThymeAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    OutOfThymeAudioProcessorEditor (OutOfThymeAudioProcessor&);
    ~OutOfThymeAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    OutOfThymeAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutOfThymeAudioProcessorEditor)
};
