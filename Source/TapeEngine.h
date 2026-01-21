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
    void setFreezeMode(bool freeze);
    
    void setMainDelay(float coarse, float fine);
    void setExtraHeadsSpacing(float spacing) { spacingParam = spacing; }
    void setSpacingSync(bool sync) { spacingSync = sync; }
    void setBpm(double newBpm) { bpm = newBpm; }

    void setExtraHeadsLevels(float newLevels) { extraHeadsLevels = newLevels; }

    void setFeedbackGain(float newGain)
    {
        if (std::abs(newGain - feedbackGain) > 0.001f)
        {
            if (freezeState == FreezeState::Frozen)
                feedbackKnobMoved = true;
                
            feedbackGain = newGain; 
        }
    }

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
    float coarseDelay { 0.5f };
    float fineDelay { 0.0f };
    float spacingParam { 0.2f };
    bool spacingSync { false };
    double bpm { 120.0 };
    float extraHeadsLevels { 0.0f };
    float feedbackGain { 0.0f };
    
    double delayDistanceL { 0.0 };
    double delayDistanceR { 0.0 };
    float lastTargetDistance { -1.0f };

    int currentFilterType { 0 };
    bool isHiFi { false };

    enum class FreezeState
    {
        Normal,
        Filling,
        Frozen
    };
    FreezeState freezeState { FreezeState::Normal };
    int samplesToFill { 0 };
    bool feedbackKnobMoved { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TapeEngine)
};
