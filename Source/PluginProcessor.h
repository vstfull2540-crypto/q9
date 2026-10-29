#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <memory>
#include <vector>

class NamModelPair;

class ArmNongMuangLA2AAudioProcessor final : public juce::AudioProcessor {
public:
    ArmNongMuangLA2AAudioProcessor();
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "LA-2A Arm Nong Muang"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    bool loadNamFile(const juce::File& file, juce::String& result);
    void unloadNamFile();
    juce::String getNamStatus() const;
    float getVuValueDb() const noexcept { return vuValueDb.load(std::memory_order_relaxed); }
    float getGainReductionDb() const noexcept { return gainReductionDb.load(std::memory_order_relaxed); }
    int getMeterMode() const noexcept { return meterMode.load(std::memory_order_relaxed); }
    bool isNamLoaded() const noexcept { return namLoaded.load(std::memory_order_relaxed); }
    juce::String getLoadedNamName() const;

private:
    std::atomic<std::shared_ptr<NamModelPair>> namModel { nullptr };
    juce::String namPath;
    juce::String namStatus { "Built-in LA-2A compressor | Select a .nam model to enable NAM" };
    std::atomic<bool> namLoaded { false };
    std::atomic<float> vuValueDb { -20.0f };
    std::atomic<float> gainReductionDb { 0.0f };
    std::atomic<int> meterMode { 2 };
    double currentSampleRate { 48000.0 };
    int currentMaxBlockSize { 512 };
    std::vector<double> namInputLeft, namInputRight, namOutputLeft, namOutputRight;
    double envelopeLeft { 0.0 }, envelopeRight { 0.0 };
    float blockInputDb { -60.0f }, blockOutputDb { -60.0f };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArmNongMuangLA2AAudioProcessor)
};
