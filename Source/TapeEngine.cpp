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

    filterL.prepare(spec);
    filterR.prepare(spec);
    
    filterL.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filterR.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

void TapeEngine::processSample(float inputL, float inputR, float& outputL, float& outputR)
{
    float mainL = readFromBuffer(0, mainHead.posL);
    float mainR = readFromBuffer(1, mainHead.posR);

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

    float headSumL = mainL + (extraSumL * extraHeadsLevels);
    float headSumR = mainR + (extraSumR * extraHeadsLevels);

    float filteredL = filterL.processSample(0, headSumL);
    float filteredR = filterR.processSample(0, headSumR);

    outputL = filteredL;
    outputR = filteredR;

    float extraFeedbackWeight = std::max(0.0f, (extraHeadsLevels - 0.5f) * 2.0f);
    
    float feedbackInputL = mainL + (extraSumL * extraHeadsLevels * extraFeedbackWeight);
    float feedbackInputR = mainR + (extraSumR * extraHeadsLevels * extraFeedbackWeight);

    float feedbackSignalL = filterL.processSample(0, feedbackInputL) * feedbackGain;
    float feedbackSignalR = filterR.processSample(0, feedbackInputR) * feedbackGain;

    float writeSignalL = freezeMode ? feedbackSignalL : (inputL + feedbackSignalL);
    float writeSignalR = freezeMode ? feedbackSignalR : (inputR + feedbackSignalR);

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
    auto filterType = juce::dsp::StateVariableTPTFilterType::lowpass;
    if (type == 2) filterType = juce::dsp::StateVariableTPTFilterType::highpass;
    else if (type == 0) filterType = juce::dsp::StateVariableTPTFilterType::bandpass;

    filterL.setType(filterType);
    filterR.setType(filterType);
    
    filterL.setCutoffFrequency(cutoffHz);
    filterR.setCutoffFrequency(cutoffHz);
    
    filterL.setResonance(resonance);
    filterR.setResonance(resonance);
}

void TapeEngine::setExtraHeadsSpacing(double spacingSamples)
{
    extraHeadsSpacing = spacingSamples;
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
