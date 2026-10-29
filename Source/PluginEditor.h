#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class ArmNongMuangLA2AAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit ArmNongMuangLA2AAudioProcessorEditor(ArmNongMuangLA2AAudioProcessor&);
    ~ArmNongMuangLA2AAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    class LeverButton final : public juce::TextButton {
    public:
        std::function<void(bool)> onLeverDragState;
        void mouseDown(const juce::MouseEvent& e) override { dragged = false; juce::TextButton::mouseDown(e); }
        void mouseDrag(const juce::MouseEvent& e) override {
            if (e.getDistanceFromDragStart() > 4) {
                dragged = true;
                const bool upper = e.getPosition().y < getHeight() / 2;
                if (getToggleState() != upper) {
                    setToggleState(upper, juce::dontSendNotification);
                    if (onLeverDragState) onLeverDragState(upper);
                }
            }
            if (!dragged) juce::TextButton::mouseDrag(e);
        }
        void mouseUp(const juce::MouseEvent& e) override {
            if (!dragged) juce::TextButton::mouseUp(e);
            else { juce::Button::setState(juce::Button::buttonNormal); repaint(); }
        }
    private:
        bool dragged { false };
    };
    class NumericRotarySlider final : public juce::Slider {
    public:
        void mouseDoubleClick(const juce::MouseEvent& event) override;
    };
    class InvisibleLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override {}
        void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override {}
    };
    class SwitchLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
        void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override {}
    };
    class PointerLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    };
    void timerCallback() override;
    void setupRotary(juce::Slider&, const juce::String&);
    void addHitButton(juce::TextButton&, const juce::String&);
    void loadModelDialog();
    void setPower(bool);
    void setLimit(bool);
    void cycleMeter();
    void saveSnapshot(int slot);
    void restoreSnapshot(int slot);
    ArmNongMuangLA2AAudioProcessor& processor;
    juce::Image background;
    InvisibleLookAndFeel invisibleLookAndFeel;
    SwitchLookAndFeel switchLookAndFeel;
    PointerLookAndFeel pointerLookAndFeel;
    NumericRotarySlider gain, peak;
    juce::Slider attack, release;
    juce::ToggleButton powerState, limitState;
    juce::TextButton topPower, preset, modelPicker, buttonA, buttonB, copyAB, settings;
    LeverButton topMeterSwitch, meterSwitch, bottomPower, limitSwitch;
    juce::Label status;
    juce::Label switchStatus;
    juce::Label gainValueLabel, peakValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment, peakAttachment, attackAttachment, releaseAttachment;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::ValueTree snapshotA, snapshotB;
    bool powerOn { true }, limitOn { false };
    float smoothedVuDb { 0.0f };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArmNongMuangLA2AAudioProcessorEditor)
};
