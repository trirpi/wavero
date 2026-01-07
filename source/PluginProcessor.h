#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>

#if (MSVC)
#include "ipps.h"
#endif

//==============================================================================
// ADSR Envelope
class GrainEnvelope
{
public:
    void setParameters (float attack, float decay, float sustain, float release, double sampleRate);
    void noteOn();
    void noteOff();
    float getNextValue();
    bool isActive() const { return state != State::Idle; }
    void reset();

private:
    enum class State { Idle, Attack, Decay, Sustain, Release };
    State state = State::Idle;

    float attackRate = 0.0f;
    float decayRate = 0.0f;
    float sustainLevel = 1.0f;
    float releaseRate = 0.0f;
    float currentValue = 0.0f;
};

//==============================================================================
struct Grain
{
    int startSample = 0;
    int currentSample = 0;
    int lengthInSamples = 0;
    float gain = 1.0f;
    float pitch = 1.0f;
    float pan = 0.5f;
    bool active = false;
};

struct GrainVisualInfo
{
    float position = 0.0f;
    float progress = 0.0f;
    float amplitude = 0.0f;
    bool active = false;
};

//==============================================================================
// Voice for polyphony
class GranularVoice
{
public:
    void prepare (double sampleRate);
    void startNote (int midiNote, float velocity);
    void stopNote();
    bool isActive() const { return envelope.isActive(); }
    int getCurrentNote() const { return currentNote; }

    void processGrains (const juce::AudioBuffer<float>& sampleBuffer, double sampleSampleRate,
                        float* leftOut, float* rightOut, int numSamples,
                        float position, float grainSizeMs, float density, float basePitch,
                        float spray, float gain, std::array<GrainVisualInfo, 16>& visuals);

    void setEnvelopeParams (float a, float d, float s, float r);

private:
    static constexpr int maxGrains = 16;  // More grains for smoother sound
    std::array<Grain, maxGrains> grains;
    GrainEnvelope envelope;

    double currentSampleRate = 44100.0;
    int samplesUntilNextGrain = 0;
    int currentNote = -1;
    float noteVelocity = 1.0f;
    float notePitchMultiplier = 1.0f;
    juce::Random random;

    void triggerGrain (const juce::AudioBuffer<float>& sampleBuffer, float position,
                       float grainSizeMs, float pitch, float spray);
    float getInterpolatedSample (const juce::AudioBuffer<float>& buffer, float position, int channel);
    float applyGrainEnvelope (float progress);
};

//==============================================================================
class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Parameter access
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

    // Sample loading (thread-safe)
    void loadSampleAsync (const juce::File& file);
    bool hasSampleLoaded() const { return sampleBuffer.getNumSamples() > 0; }
    const juce::AudioBuffer<float>& getSampleBuffer() const { return sampleBuffer; }
    juce::String getSampleName() const { return loadedSampleName; }
    juce::String getSamplePath() const { return loadedSamplePath; }

    // Grain visualization (thread-safe copy)
    std::array<GrainVisualInfo, 64> getGrainVisuals() const;
    
    // Add MIDI message from UI (thread-safe)
    void addMidiMessage (const juce::MidiMessage& message);

    // Thread-safe sample info for UI display
    int getSampleNumSamples() const { return sampleBuffer.getNumSamples(); }
    int getSampleNumChannels() const { return sampleBuffer.getNumChannels(); }

    // Preset management
    void savePreset (const juce::File& file);
    void loadPreset (const juce::File& file);
    juce::StringArray getPresetNames() const;

    // Parameter IDs
    static constexpr const char* POSITION_ID = "position";
    static constexpr const char* GRAIN_SIZE_ID = "grainSize";
    static constexpr const char* DENSITY_ID = "density";
    static constexpr const char* PITCH_ID = "pitch";
    static constexpr const char* SPRAY_ID = "spray";
    static constexpr const char* GAIN_ID = "gain";
    static constexpr const char* ATTACK_ID = "attack";
    static constexpr const char* DECAY_ID = "decay";
    static constexpr const char* SUSTAIN_ID = "sustain";
    static constexpr const char* RELEASE_ID = "release";

private:
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioFormatManager formatManager;

    // Sample storage (double-buffered for thread safety)
    juce::AudioBuffer<float> sampleBuffer;
    juce::AudioBuffer<float> loadingBuffer;
    double sampleSampleRate = 44100.0;
    juce::String loadedSampleName;
    juce::String loadedSamplePath;
    std::atomic<bool> sampleReady { false };
    std::atomic<bool> loadingSample { false };

    // Polyphonic voices
    static constexpr int numVoices = 8;
    std::array<GranularVoice, numVoices> voices;
    std::array<std::array<GrainVisualInfo, 16>, numVoices> voiceVisuals;

    // Playback state
    double currentSampleRate = 44100.0;
    
    // MIDI from UI keyboard
    juce::MidiBuffer pendingMidiMessages;
    juce::CriticalSection midiLock;

    // Background loading
    std::unique_ptr<juce::TimeSliceThread> loadingThread;

    void loadSampleInternal (const juce::File& file);
    void handleMidiEvent (const juce::MidiMessage& message);
    GranularVoice* findFreeVoice();
    GranularVoice* findVoiceForNote (int midiNote);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
