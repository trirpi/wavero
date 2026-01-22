#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"
#include <juce_audio_utils/juce_audio_utils.h>

//==============================================================================
// Custom font holder - loads Playfair Display from binary data
class CustomFonts
{
public:
    CustomFonts()
    {
        // Load static font files from binary data
        regularTypeface = juce::Typeface::createSystemTypefaceFor (
            BinaryData::PlayfairDisplayRegular_ttf,
            BinaryData::PlayfairDisplayRegular_ttfSize);
            
        boldTypeface = juce::Typeface::createSystemTypefaceFor (
            BinaryData::PlayfairDisplayBold_ttf,
            BinaryData::PlayfairDisplayBold_ttfSize);
    }

    juce::Font getFont (float height, bool bold = false) const
    {
        auto typeface = bold ? boldTypeface : regularTypeface;
        if (typeface != nullptr)
        {
            return juce::Font (juce::FontOptions (typeface).withHeight (height));
        }
        return juce::Font (juce::FontOptions (height));
    }

    juce::Font getTitleFont (float height) const
    {
        return getFont (height, true);
    }

    juce::Font getLabelFont (float height) const
    {
        return getFont (height, false);
    }
    
    bool isLoaded() const { return regularTypeface != nullptr && boldTypeface != nullptr; }

private:
    juce::Typeface::Ptr regularTypeface;
    juce::Typeface::Ptr boldTypeface;
};

//==============================================================================
struct GrainParticle
{
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float life = 0.0f;
    float maxLife = 1.0f;
    float size = 2.0f;
    juce::Colour colour;
};

//==============================================================================
class FuturisticSliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    FuturisticSliderLookAndFeel();

    void setCustomFonts (const CustomFonts* fonts) { customFonts = fonts; }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle style, juce::Slider& slider) override;

    juce::Label* createSliderTextBox (juce::Slider& slider) override;

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isMouseOver, bool isButtonDown) override;

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    juce::Font getLabelFont (juce::Label&) override;
    
    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool isMouseOverButton, bool isButtonDown) override;

private:
    const CustomFonts* customFonts = nullptr;
};

//==============================================================================
class WaveformDisplay : public juce::Component,
                        public juce::FileDragAndDropTarget,
                        public juce::Timer
{
public:
    WaveformDisplay (PluginProcessor& p, const CustomFonts& fonts);
    ~WaveformDisplay() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseEnter (const juce::MouseEvent& event) override;
    void mouseExit (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;
    void fileDragEnter (const juce::StringArray& files, int x, int y) override;
    void fileDragExit (const juce::StringArray& files) override;

    void setPositionMarker (float position) { positionMarker = position; }
    void setSprayAmount (float spray) { sprayAmount = spray; }

    std::function<void()> onFileDropped;
    std::function<void()> onClickToLoad;

private:
    PluginProcessor& processor;
    const CustomFonts& customFonts;
    float positionMarker = 0.0f;
    float sprayAmount = 0.0f;
    bool isDragOver = false;
    bool isHovering = false;
    float animationPhase = 0.0f;
    std::vector<GrainParticle> particles;
    std::array<float, 64> grainGlowIntensity {};
    juce::Random random;
    std::array<GrainVisualInfo, 64> cachedGrainVisuals;
    
    // Scrolling for long samples
    float scrollOffset = 0.0f;  // 0.0 to 1.0 representing scroll position
    float lastDragX = 0.0f;
    bool needsScroll = false;
    double cachedSampleDuration = 0.0;

    juce::Colour waveformColour { 0xff00d4ff };
    juce::Colour backgroundColour { 0xff080810 };
    juce::Colour markerColour { 0xffe94560 };
    juce::Colour textColour { 0xffeaeaea };
    juce::Colour grainColour { 0xffff6b6b };
    juce::Colour particleColour { 0xffffbe0b };

    void spawnParticles (float x, float y, int count);
    void updateParticles();
    void drawGrainIndicator (juce::Graphics& g, float x, float progress, float amplitude, float glowIntensity);
};

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor,
                     public juce::Timer,
                     public juce::MidiKeyboardState::Listener
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    
    // MidiKeyboardState::Listener
    void handleNoteOn (juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff (juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;

private:
    PluginProcessor& processorRef;
    CustomFonts customFonts;
    FuturisticSliderLookAndFeel sliderLookAndFeel;

    // Virtual MIDI Keyboard
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;

    WaveformDisplay waveformDisplay;

    // Main controls
    juce::Slider positionSlider;
    juce::Slider grainSizeSlider;
    juce::Slider densitySlider;
    juce::Slider pitchSlider;
    juce::Slider spraySlider;
    juce::Slider gainSlider;

    // ADSR
    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider sustainSlider;
    juce::Slider releaseSlider;

    // Labels
    juce::Label positionLabel, grainSizeLabel, densityLabel, pitchLabel, sprayLabel, gainLabel;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;
    juce::Label titleLabel, subtitleLabel, sampleNameLabel;
    juce::Label envelopeLabel, grainLabel;

    juce::TextButton loadButton { "LOAD" };
    juce::TextButton savePresetButton { "SAVE" };
    juce::TextButton loadPresetButton { "PRESET" };

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> positionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> grainSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> densityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sprayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

#if JUCE_DEBUG
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton { "DBG" };
#endif

    juce::Colour backgroundColour { 0xff050508 };
    juce::Colour panelColour { 0xff0a0a12 };
    juce::Colour accentColour { 0xff00d4ff };
    juce::Colour secondaryColour { 0xffe94560 };
    juce::Colour tertiaryColour { 0xff9d4edd };
    juce::Colour textColour { 0xffe8e8e8 };
    juce::Colour dimTextColour { 0xffb8b8c0 };

    void setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& labelText, int colourIndex);
    void loadSampleFile();
    void updateSampleDisplay();
    void savePreset();
    void loadPreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
