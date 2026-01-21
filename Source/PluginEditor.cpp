#include "PluginProcessor.h"
#include "PluginEditor.h"

OutOfThymeAudioProcessorEditor::OutOfThymeAudioProcessorEditor (OutOfThymeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
}

OutOfThymeAudioProcessorEditor::~OutOfThymeAudioProcessorEditor()
{
}

void OutOfThymeAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Out Of Thyme", getLocalBounds(), juce::Justification::centred, 1);
}

void OutOfThymeAudioProcessorEditor::resized()
{
}
