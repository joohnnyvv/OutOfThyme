#include "PluginProcessor.h"
#include "PluginEditor.h"

OutOfThymeAudioProcessorEditor::OutOfThymeAudioProcessorEditor (OutOfThymeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    backgroundImage = juce::ImageCache::getFromMemory (BinaryData::bg_png, BinaryData::bg_pngSize);
    setSize (1200, 628);
}

OutOfThymeAudioProcessorEditor::~OutOfThymeAudioProcessorEditor()
{
}

void OutOfThymeAudioProcessorEditor::paint (juce::Graphics& g)
{
    if (backgroundImage.isValid())
    {
        g.drawImageWithin (backgroundImage, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::fillDestination);
    }
    else
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
        g.setColour (juce::Colours::white);
        g.drawFittedText ("Background Image Not Found", getLocalBounds(), juce::Justification::centred, 1);
    }
}

void OutOfThymeAudioProcessorEditor::resized()
{
}
