#pragma once

#include <JuceHeader.h>
#include <array>

class TapeEngine
{
public:
    TapeEngine();
    ~TapeEngine() = default;

    void prepare(double sampleRate, double maxDelaySeconds = 128.0);

    void processSample(float inputL, float inputR, float& outputL, float& outputR);

    void setTapeSpeed(double speedL, double speedR) { tapeSpeedL = speedL; tapeSpeedR = speedR; }
    void setInterpolationMode(bool useHiFi) { isHiFi = useHiFi; }
    void setFreezeMode(bool freeze) { freezeMode = freeze; }
    
    void setMainDelay(double delaySamplesL, double delaySamplesR);

    void setExtraHeadsSpacing(double spacingSamples) { extraHeadsSpacing = spacingSamples; }

    void setExtraHeadsLevels(float newLevels) { extraHeadsLevels = newLevels; }

    void setFeedbackGain(float newGain) { feedbackGain = newGain; }

    void setFilter(int type, float cutoffHz, float resonance);

private:
    struct ReadHead
    {
        double posL { 0.0 };
        double posR { 0.0 };
    };

    float readFromBuffer(int channel, double pos) const;

    juce::AudioBuffer<float> buffer;
    int bufferLength { 0 };
    int writePos { 0 };

    ReadHead mainHead;
    std::array<ReadHead, 3> extraHeads;

    juce::dsp::StateVariableTPTFilter<float> inputFilterL, inputFilterR;
    juce::dsp::StateVariableTPTFilter<float> outputFilterL, outputFilterR;
    juce::dsp::StateVariableTPTFilter<float> feedbackFilterL, feedbackFilterR;

    double sampleRate { 44100.0 };
    double tapeSpeedL { 1.0 }, tapeSpeedR { 1.0 };
    double extraHeadsSpacing { 0.0 };
    float extraHeadsLevels { 0.0f };
    float feedbackGain { 0.0f };
    int currentFilterType { 0 }; // 0: Bypass, 1: LP, 2: HP
    bool isHiFi { false };
    bool freezeMode { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TapeEngine)
};
