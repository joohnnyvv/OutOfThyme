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

    delayDistanceL += (1.0 - tapeSpeedL);
    delayDistanceR += (1.0 - tapeSpeedR);

    while (delayDistanceL < 0) delayDistanceL += (double)bufferLength;
    delayDistanceL = std::fmod(delayDistanceL, (double)bufferLength);
    while (delayDistanceR < 0) delayDistanceR += (double)bufferLength;
    delayDistanceR = std::fmod(delayDistanceR, (double)bufferLength);

    mainHead.posL = (double)writePos - delayDistanceL;
    mainHead.posR = (double)writePos - delayDistanceR;
    
    while (mainHead.posL < 0) mainHead.posL += (double)bufferLength;
    mainHead.posL = std::fmod(mainHead.posL, (double)bufferLength);
    while (mainHead.posR < 0) mainHead.posR += (double)bufferLength;
    mainHead.posR = std::fmod(mainHead.posR, (double)bufferLength);

    float mainReadL = readFromBuffer(0, mainHead.posL);
    float mainReadR = readFromBuffer(1, mainHead.posR);

    float spacingSamples = 0.0f;
    if (spacingSync)
    {
        double beatDuration = 60.0 / bpm;
        double divisions[] = { 0.03125, 0.0625, 0.125, 0.25, 0.5, 1.0, 2.0 };
        int idx = static_cast<int>(std::clamp(spacingParam * 6.0f, 0.0f, 6.0f));
        double spacingTime = divisions[idx] * beatDuration;
        spacingSamples = static_cast<float>(spacingTime * tapeSpeedL * sampleRate);
    }
    else
    {
        spacingSamples = spacingParam * (float)sampleRate;
    }

    float extraSumL = 0.0f;
    float extraSumR = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        double offset = (double)(i + 1) * spacingSamples;
        double posL = mainHead.posL - offset;
        double posR = mainHead.posR - offset;
        
        while (posL < 0) posL += (double)bufferLength;
        posL = std::fmod(posL, (double)bufferLength);
        
        while (posR < 0) posR += (double)bufferLength;
        posR = std::fmod(posR, (double)bufferLength);

        extraSumL += readFromBuffer(0, posL);
        extraSumR += readFromBuffer(1, posR);
    }
    
    extraSumL *= 0.33f;
    extraSumR *= 0.33f;

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

    float writeSignalL = 0.0f;
    float writeSignalR = 0.0f;
    
    float activeFeedbackGain = feedbackGain;
    
    if (freezeState == FreezeState::Frozen && !feedbackKnobMoved)
        activeFeedbackGain = 1.0f;

    switch (freezeState)
    {
        case FreezeState::Normal:
            feedbackSignalL *= activeFeedbackGain;
            feedbackSignalR *= activeFeedbackGain;
            writeSignalL = filteredInputL + feedbackSignalL;
            writeSignalR = filteredInputR + feedbackSignalR;
            break;

        case FreezeState::Filling:
            writeSignalL = filteredInputL;
            writeSignalR = filteredInputR;
            
            if (--samplesToFill <= 0)
            {
                freezeState = FreezeState::Frozen;
                feedbackKnobMoved = false;
            }
            break;

        case FreezeState::Frozen:
            feedbackSignalL *= activeFeedbackGain;
            feedbackSignalR *= activeFeedbackGain;
            writeSignalL = feedbackSignalL;
            writeSignalR = feedbackSignalR;
            break;
    }

    buffer.setSample(0, writePos, writeSignalL);
    buffer.setSample(1, writePos, writeSignalR);


    writePos = (writePos + 1) % bufferLength;
}

void TapeEngine::setMainDelay(float coarse, float fine)
{
    float newTargetDistance = ((coarse * 2.7f) + (fine * 0.05f)) * (float)sampleRate;
    
    if (std::abs(newTargetDistance - lastTargetDistance) > 1.0f)
    {
        delayDistanceL = (double)newTargetDistance;
        delayDistanceR = (double)newTargetDistance;
        lastTargetDistance = newTargetDistance;
    }
}

void TapeEngine::setFreezeMode(bool freeze)
{
    if (freeze)
    {
        if (freezeState == FreezeState::Normal)
        {
            freezeState = FreezeState::Filling;
            
            double currentDelay = (double)(writePos - mainHead.posL);
            if (currentDelay < 0) currentDelay += (double)bufferLength;
            
            samplesToFill = static_cast<int>(currentDelay);
            if (samplesToFill <= 0) samplesToFill = 1;
        }
    }
    else
    {
        freezeState = FreezeState::Normal;
        feedbackKnobMoved = false;
    }
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
