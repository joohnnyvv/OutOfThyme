#include "TapeEngine.h"

TapeEngine::TapeEngine()
{
}

void TapeEngine::prepare(double newSampleRate, double maxDelaySeconds)
{
    sampleRate = newSampleRate;
    bufferLength = static_cast<int>(sampleRate * maxDelaySeconds);
    
    if (bufferLength < 1024) bufferLength = 1024;
    
    buffer.setSize(2, bufferLength);
    buffer.clear();
    
    writePos = 0;
    mainHead.posL = 0.0;
    mainHead.posR = 0.0;
    
    for (auto& head : extraHeads)
    {
        head.posL = 0.0;
        head.posR = 0.0;
    }

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 1;
    spec.numChannels = 1;

    inputFilterL.prepare(spec);
    inputFilterR.prepare(spec);
    outputFilterL.prepare(spec);
    outputFilterR.prepare(spec);
    feedbackFilterL.prepare(spec);
    feedbackFilterR.prepare(spec);

    inputFilterL.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    inputFilterR.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    inputFilterL.setCutoffFrequency(std::min(20000.0, sampleRate * 0.45));
    inputFilterR.setCutoffFrequency(std::min(20000.0, sampleRate * 0.45));
}

void TapeEngine::processSample(float inputL, float inputR, float& outputL, float& outputR)
{
    float filteredInputL = inputL;
    float filteredInputR = inputR;
    
    if (isHiFi)
    {
        filteredInputL = inputFilterL.processSample(0, inputL);
        filteredInputR = inputFilterR.processSample(0, inputR);
    }

    float mainReadL = readFromBuffer(0, mainHead.posL);
    float mainReadR = readFromBuffer(1, mainHead.posR);

    float extraSumL = 0.0f;
    float extraSumR = 0.0f;

    if (extraHeadsLevels > 0.001f)
    {
        for (const auto& head : extraHeads)
        {
            extraSumL += readFromBuffer(0, head.posL);
            extraSumR += readFromBuffer(1, head.posR);
        }
    }

    float outputCombinedL = mainReadL + (extraSumL * extraHeadsLevels);
    float outputCombinedR = mainReadR + (extraSumR * extraHeadsLevels);

    if (currentFilterType != 0)
    {
        outputL = outputFilterL.processSample(0, outputCombinedL);
        outputR = outputFilterR.processSample(0, outputCombinedR);
    }
    else
    {
        outputL = outputCombinedL;
        outputR = outputCombinedR;
    }

    float extraFeedbackWeight = std::max(0.0f, (extraHeadsLevels - 0.5f) * 2.0f);
    
    float feedbackInputL = mainReadL + (extraSumL * extraHeadsLevels * extraFeedbackWeight);
    float feedbackInputR = mainReadR + (extraSumR * extraHeadsLevels * extraFeedbackWeight);

    float feedbackSignalL = feedbackInputL;
    float feedbackSignalR = feedbackInputR;

    if (currentFilterType != 0)
    {
        feedbackSignalL = feedbackFilterL.processSample(0, feedbackInputL);
        feedbackSignalR = feedbackFilterR.processSample(0, feedbackInputR);
    }

    feedbackSignalL *= feedbackGain;
    feedbackSignalR *= feedbackGain;

    float writeSignalL = freezeMode ? feedbackSignalL : (filteredInputL + feedbackSignalL);
    float writeSignalR = freezeMode ? feedbackSignalR : (filteredInputR + feedbackSignalR);

    buffer.setSample(0, writePos, writeSignalL);
    buffer.setSample(1, writePos, writeSignalR);

    mainHead.posL = std::fmod(mainHead.posL + tapeSpeedL, (double)bufferLength);
    mainHead.posR = std::fmod(mainHead.posR + tapeSpeedR, (double)bufferLength);

    for (int i = 0; i < 3; ++i)
    {
        extraHeads[i].posL = std::fmod(mainHead.posL + (double)(i + 1) * extraHeadsSpacing, (double)bufferLength);
        extraHeads[i].posR = std::fmod(mainHead.posR + (double)(i + 1) * extraHeadsSpacing, (double)bufferLength);
    }

    writePos = (writePos + 1) % bufferLength;
}

void TapeEngine::setMainDelay(double delaySamplesL, double delaySamplesR)
{
    mainHead.posL = (double)writePos - delaySamplesL;
    while (mainHead.posL < 0) mainHead.posL += (double)bufferLength;
    mainHead.posL = std::fmod(mainHead.posL, (double)bufferLength);
    
    mainHead.posR = (double)writePos - delaySamplesR;
    while (mainHead.posR < 0) mainHead.posR += (double)bufferLength;
    mainHead.posR = std::fmod(mainHead.posR, (double)bufferLength);
}

void TapeEngine::setFilter(int type, float cutoffHz, float resonance)
{
    currentFilterType = type;
    
    if (type == 0) return;

    auto filterType = (type == 2) ? juce::dsp::StateVariableTPTFilterType::highpass 
                                  : juce::dsp::StateVariableTPTFilterType::lowpass;

    outputFilterL.setType(filterType);
    outputFilterR.setType(filterType);
    feedbackFilterL.setType(filterType);
    feedbackFilterR.setType(filterType);
    
    outputFilterL.setCutoffFrequency(cutoffHz);
    outputFilterR.setCutoffFrequency(cutoffHz);
    feedbackFilterL.setCutoffFrequency(cutoffHz);
    feedbackFilterR.setCutoffFrequency(cutoffHz);
    
    outputFilterL.setResonance(resonance);
    outputFilterR.setResonance(resonance);
    feedbackFilterL.setResonance(resonance);
    feedbackFilterR.setResonance(resonance);
}



float TapeEngine::readFromBuffer(int channel, double pos) const
{
    if (isHiFi)
    {
        int pos1 = static_cast<int>(pos);
        int pos2 = (pos1 + 1) % bufferLength;
        float fraction = (float)(pos - (double)pos1);

        float s1 = buffer.getSample(channel, pos1);
        float s2 = buffer.getSample(channel, pos2);

        return s1 + fraction * (s2 - s1);
    }
    else
    {
        return buffer.getSample(channel, static_cast<int>(pos) % bufferLength);
    }
}
