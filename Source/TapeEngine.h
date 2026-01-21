#pragma once

#include <JuceHeader.h>
#include <array>

class TapeEngine
{
public:
    TapeEngine();
    ~TapeEngine() = default;

    /** Allocates the massive circular buffer. 
        maxDelaySeconds should be around 128 to handle extreme slowdowns. */
    void prepare(double sampleRate, double maxDelaySeconds = 128.0);

    /** Processes a single stereo sample. 
        Following the order: Read -> Filter -> Feedback -> Write. */
    void processSample(float inputL, float inputR, float& outputL, float& outputR);

    // --- Parameter Setters ---
    void setTapeSpeed(double speedL, double speedR) { tapeSpeedL = speedL; tapeSpeedR = speedR; }
    void setInterpolationMode(bool useHiFi) { isHiFi = useHiFi; }
    void setFreezeMode(bool freeze) { freezeMode = freeze; }
    
    /** Sets the delay time for the main head in samples. */
    void setMainDelay(double delaySamplesL, double delaySamplesR);

    /** Sets the spacing for extra heads relative to the main head. */
    void setExtraHeadsSpacing(double spacingSamples) { extraHeadsSpacing = spacingSamples; }

    /** Sets the relative volume of the extra heads. */
    void setExtraHeadsLevels(float newLevels) { extraHeadsLevels = newLevels; }

    /** Sets the feedback gain (0.0 to 1.0+). */
    void setFeedbackGain(float newGain) { feedbackGain = newGain; }

    /** Sets the filter parameters. 
        type: 0 = Bypass, 1 = LP, 2 = HP. (Simplification of Thyme's morphing filter) */
    void setFilter(int type, float cutoffHz, float resonance);

private:
    struct ReadHead
    {
        double posL { 0.0 };
        double posR { 0.0 };
    };

    /** Reads a sample with interpolation from the buffer. */
    float readFromBuffer(int channel, double pos) const;

    juce::AudioBuffer<float> buffer;
    int bufferLength { 0 };
    int writePos { 0 };

    ReadHead mainHead;
    std::array<ReadHead, 3> extraHeads;

    juce::dsp::StateVariableTPTFilter<float> filterL, filterR;

    double sampleRate { 44100.0 };
    double tapeSpeedL { 1.0 }, tapeSpeedR { 1.0 };
    double extraHeadsSpacing { 0.0 };
    float extraHeadsLevels { 0.0f };
    float feedbackGain { 0.0f };
    bool isHiFi { false };
    bool freezeMode { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TapeEngine)
};
