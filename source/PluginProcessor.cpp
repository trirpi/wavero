#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// GrainEnvelope Implementation
//==============================================================================
void GrainEnvelope::setParameters (float attack, float decay, float sustain, float release, double sampleRate)
{
    attackRate = attack > 0.001f ? 1.0f / (attack * static_cast<float> (sampleRate)) : 1.0f;
    decayRate = decay > 0.001f ? 1.0f / (decay * static_cast<float> (sampleRate)) : 1.0f;
    sustainLevel = sustain;
    releaseRate = release > 0.001f ? 1.0f / (release * static_cast<float> (sampleRate)) : 1.0f;
}

void GrainEnvelope::noteOn()
{
    state = State::Attack;
    currentValue = 0.0f;  // Reset to start fresh attack
}

void GrainEnvelope::noteOff()
{
    if (state != State::Idle)
        state = State::Release;
}

float GrainEnvelope::getNextValue()
{
    switch (state)
    {
        case State::Idle:
            return 0.0f;

        case State::Attack:
            currentValue += attackRate;
            if (currentValue >= 1.0f)
            {
                currentValue = 1.0f;
                state = State::Decay;
            }
            break;

        case State::Decay:
            currentValue -= decayRate * (1.0f - sustainLevel);
            if (currentValue <= sustainLevel)
            {
                currentValue = sustainLevel;
                state = State::Sustain;
            }
            break;

        case State::Sustain:
            currentValue = sustainLevel;
            break;

        case State::Release:
            // Use linear decay for reliable fade-out (exponential can get stuck)
            currentValue -= releaseRate;
            if (currentValue <= 0.0f)
            {
                currentValue = 0.0f;
                state = State::Idle;
            }
            break;
    }
    return currentValue;
}

void GrainEnvelope::reset()
{
    state = State::Idle;
    currentValue = 0.0f;
}

//==============================================================================
// GranularVoice Implementation
//==============================================================================
void GranularVoice::prepare (double sampleRate)
{
    currentSampleRate = sampleRate;
    samplesUntilNextGrain = 0;
    for (auto& grain : grains)
        grain.active = false;
}

void GranularVoice::setEnvelopeParams (float a, float d, float s, float r)
{
    envelope.setParameters (a, d, s, r, currentSampleRate);
}

void GranularVoice::startNote (int midiNote, float velocity)
{
    currentNote = midiNote;
    noteVelocity = velocity;
    // Calculate pitch multiplier from MIDI note (middle C = 60 is 1.0x)
    notePitchMultiplier = std::pow (2.0f, (midiNote - 60) / 12.0f);
    envelope.noteOn();
    samplesUntilNextGrain = 0;
}

void GranularVoice::stopNote()
{
    envelope.noteOff();
}

float GranularVoice::getInterpolatedSample (const juce::AudioBuffer<float>& buffer, float position, int channel)
{
    if (buffer.getNumSamples() == 0)
        return 0.0f;

    int ch = juce::jmin (channel, buffer.getNumChannels() - 1);
    int numSamples = buffer.getNumSamples();

    while (position < 0.0f)
        position += static_cast<float> (numSamples);
    while (position >= static_cast<float> (numSamples))
        position -= static_cast<float> (numSamples);

    int index0 = static_cast<int> (position);
    int index1 = (index0 + 1) % numSamples;
    float frac = position - static_cast<float> (index0);

    auto* data = buffer.getReadPointer (ch);
    return data[index0] + frac * (data[index1] - data[index0]);
}

float GranularVoice::applyGrainEnvelope (float progress)
{
    return 0.5f * (1.0f - std::cos (juce::MathConstants<float>::twoPi * progress));
}

void GranularVoice::triggerGrain (const juce::AudioBuffer<float>& sampleBuffer, float position,
                                   float grainSizeMs, float pitch, float spray)
{
    for (auto& grain : grains)
    {
        if (!grain.active)
        {
            int numSamples = sampleBuffer.getNumSamples();
            if (numSamples == 0)
                return;

            float sprayOffset = (random.nextFloat() - 0.5f) * 2.0f * spray;
            float adjustedPosition = juce::jlimit (0.0f, 1.0f, position + sprayOffset);

            grain.startSample = static_cast<int> (adjustedPosition * static_cast<float> (numSamples));
            grain.currentSample = 0;
            grain.lengthInSamples = juce::jmax (1, static_cast<int> ((grainSizeMs / 1000.0f) * currentSampleRate));
            grain.pitch = pitch;
            grain.gain = 0.6f + random.nextFloat() * 0.4f;
            grain.pan = 0.3f + random.nextFloat() * 0.4f;
            grain.active = true;
            return;
        }
    }
}

void GranularVoice::processGrains (const juce::AudioBuffer<float>& sampleBuffer, double sampleSampleRate,
                                    float* leftOut, float* rightOut, int numSamples,
                                    float position, float grainSizeMs, float density, float basePitch,
                                    float spray, float gain, std::array<GrainVisualInfo, 16>& visuals)
{
    // Clear visuals when not active so indicators disappear
    if (sampleBuffer.getNumSamples() == 0 || !envelope.isActive() || currentSampleRate <= 0.0)
    {
        for (auto& vis : visuals)
            vis.active = false;
        return;
    }

    float pitchWithNote = basePitch * notePitchMultiplier;
    float sampleRateRatio = currentSampleRate > 0.0 
        ? static_cast<float> (sampleSampleRate / currentSampleRate) 
        : 1.0f;
    int samplesPerGrain = juce::jmax (1, static_cast<int> (currentSampleRate / juce::jmax (0.1f, density)));

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float envValue = envelope.getNextValue();
        if (envValue < 0.0001f)
            continue;

        if (samplesUntilNextGrain <= 0)
        {
            triggerGrain (sampleBuffer, position, grainSizeMs, pitchWithNote * sampleRateRatio, spray);
            samplesUntilNextGrain = samplesPerGrain;
        }
        samplesUntilNextGrain--;

        float outputL = 0.0f;
        float outputR = 0.0f;

        for (size_t i = 0; i < grains.size(); ++i)
        {
            auto& grain = grains[i];
            auto& vis = visuals[i];

            if (grain.active && grain.lengthInSamples > 0)
            {
                float progress = static_cast<float> (grain.currentSample) / static_cast<float> (grain.lengthInSamples);
                float grainEnv = applyGrainEnvelope (juce::jlimit (0.0f, 1.0f, progress));

                float readPosition = static_cast<float> (grain.startSample) +
                                    static_cast<float> (grain.currentSample) * grain.pitch;

                float sampleValue = getInterpolatedSample (sampleBuffer, readPosition, 0) * grainEnv * grain.gain;

                vis.active = true;
                // Show actual read position so visualization moves with playback
                // Use modulo to wrap around if pitch causes position to exceed buffer
                int numSamplesInBuffer = sampleBuffer.getNumSamples();
                if (numSamplesInBuffer > 0)
                {
                    float wrappedPosition = std::fmod (readPosition, static_cast<float> (numSamplesInBuffer));
                    if (wrappedPosition < 0.0f)
                        wrappedPosition += static_cast<float> (numSamplesInBuffer);
                    vis.position = wrappedPosition / static_cast<float> (numSamplesInBuffer);
                }
                else
                {
                    vis.position = 0.0f;
                }
                vis.progress = juce::jlimit (0.0f, 1.0f, progress);
                vis.amplitude = juce::jlimit (0.0f, 1.0f, std::abs (sampleValue * envValue));

                outputL += sampleValue * (1.0f - grain.pan);
                outputR += sampleValue * grain.pan;

                grain.currentSample++;
                if (grain.currentSample >= grain.lengthInSamples)
                {
                    grain.active = false;
                    vis.active = false;
                }
            }
            else
            {
                vis.active = false;
            }
        }

        leftOut[sample] += outputL * gain * envValue * noteVelocity;
        rightOut[sample] += outputR * gain * envValue * noteVelocity;
    }
}

//==============================================================================
// PluginProcessor Implementation
//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       parameters (*this, nullptr, "PARAMETERS",
           {
               std::make_unique<juce::AudioParameterFloat> (
                   juce::ParameterID { POSITION_ID, 1 },
                   "Position",
                   juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f),
                   0.25f),

               std::make_unique<juce::AudioParameterFloat> (
                   juce::ParameterID { GRAIN_SIZE_ID, 1 },
                   "Grain Size",
                   juce::NormalisableRange<float> (10.0f, 3000.0f, 1.0f, 0.35f),
                   1000.0f,  // Default to 1 second (middle position)
                   juce::AudioParameterFloatAttributes().withLabel ("ms")),

               std::make_unique<juce::AudioParameterFloat> (
                   juce::ParameterID { DENSITY_ID, 1 },
                   "Density",
                   juce::NormalisableRange<float> (1.0f, 50.0f, 0.1f, 0.5f),
                   25.0f,  // Higher density for smoother sound
                   juce::AudioParameterFloatAttributes().withLabel ("Hz")),

               std::make_unique<juce::AudioParameterFloat> (
                   juce::ParameterID { PITCH_ID, 1 },
                   "Pitch",
                   juce::NormalisableRange<float> (0.25f, 4.0f, 0.01f, 0.43f),  // Skew adjusted so 1.0 (0 st) is at center
                   1.0f,
                   juce::AudioParameterFloatAttributes().withLabel ("st")),

               std::make_unique<juce::AudioParameterFloat> (
                   juce::ParameterID { SPRAY_ID, 1 },
                   "Spray",
                   juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
                   0.5f),  // Default to middle

               std::make_unique<juce::AudioParameterFloat> (
                   juce::ParameterID { GAIN_ID, 1 },
                   "Gain",
                   juce::NormalisableRange<float> (0.0f, 2.0f, 0.01f),
                   1.0f),

               std::make_unique<juce::AudioParameterFloat> (
                   juce::ParameterID { ATTACK_ID, 1 },
                   "Attack",
                   juce::NormalisableRange<float> (0.001f, 2.0f, 0.001f, 0.4f),
                   0.05f,
                   juce::AudioParameterFloatAttributes().withLabel ("s")),

               std::make_unique<juce::AudioParameterFloat> (
                   juce::ParameterID { DECAY_ID, 1 },
                   "Decay",
                   juce::NormalisableRange<float> (0.001f, 2.0f, 0.001f, 0.4f),
                   0.05f,  // Shorter decay
                   juce::AudioParameterFloatAttributes().withLabel ("s")),

               std::make_unique<juce::AudioParameterFloat> (
                   juce::ParameterID { SUSTAIN_ID, 1 },
                   "Sustain",
                   juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
                   0.8f),

               std::make_unique<juce::AudioParameterFloat> (
                   juce::ParameterID { RELEASE_ID, 1 },
                   "Release",
                   juce::NormalisableRange<float> (0.001f, 5.0f, 0.001f, 0.4f),
                   0.2f,  // Shorter release
                   juce::AudioParameterFloatAttributes().withLabel ("s"))
           })
{
    formatManager.registerBasicFormats();
    loadingThread = std::make_unique<juce::TimeSliceThread> ("Sample Loader");
    loadingThread->startThread();
}

PluginProcessor::~PluginProcessor()
{
    loadingThread->stopThread (1000);
}

const juce::String PluginProcessor::getName() const { return JucePlugin_Name; }
bool PluginProcessor::acceptsMidi() const { return true; }
bool PluginProcessor::producesMidi() const { return false; }
bool PluginProcessor::isMidiEffect() const { return false; }
double PluginProcessor::getTailLengthSeconds() const { return 2.0; }
int PluginProcessor::getNumPrograms() { return 1; }
int PluginProcessor::getCurrentProgram() { return 0; }
void PluginProcessor::setCurrentProgram (int index) { juce::ignoreUnused (index); }
const juce::String PluginProcessor::getProgramName (int index) { juce::ignoreUnused (index); return {}; }
void PluginProcessor::changeProgramName (int index, const juce::String& newName) { juce::ignoreUnused (index, newName); }

void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    for (auto& voice : voices)
        voice.prepare (sampleRate);

    juce::ignoreUnused (samplesPerBlock);
}

void PluginProcessor::releaseResources() {}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void PluginProcessor::loadSampleAsync (const juce::File& file)
{
    if (loadingSample.load())
        return;

    loadingSample.store (true);

    // Load on background thread
    juce::Thread::launch ([this, file]()
    {
        loadSampleInternal (file);
        loadingSample.store (false);
    });
}

void PluginProcessor::loadSampleInternal (const juce::File& file)
{
    auto* reader = formatManager.createReaderFor (file);

    if (reader != nullptr)
    {
        loadingBuffer.setSize (static_cast<int> (reader->numChannels),
                               static_cast<int> (reader->lengthInSamples));
        reader->read (&loadingBuffer, 0, static_cast<int> (reader->lengthInSamples), 0, true, true);

        // Swap buffers (atomic from audio thread perspective)
        {
            const juce::ScopedLock sl (getCallbackLock());
            std::swap (sampleBuffer, loadingBuffer);
            sampleSampleRate = reader->sampleRate;
            loadedSampleName = file.getFileNameWithoutExtension();
            loadedSamplePath = file.getFullPathName();
        }

        delete reader;
        sampleReady.store (true);
    }
}

GranularVoice* PluginProcessor::findFreeVoice()
{
    for (auto& voice : voices)
    {
        if (!voice.isActive())
            return &voice;
    }
    // Steal oldest voice
    return &voices[0];
}

GranularVoice* PluginProcessor::findVoiceForNote (int midiNote)
{
    for (auto& voice : voices)
    {
        if (voice.isActive() && voice.getCurrentNote() == midiNote)
            return &voice;
    }
    return nullptr;
}

void PluginProcessor::handleMidiEvent (const juce::MidiMessage& message)
{
    // Note: isNoteOn() returns true even for velocity 0, which should be treated as note-off
    if (message.isNoteOn() && message.getVelocity() > 0)
    {
        // Check if this note is already playing - don't retrigger
        auto* existingVoice = findVoiceForNote (message.getNoteNumber());
        if (existingVoice != nullptr)
            return;  // Note already playing, ignore duplicate note-on
        
        auto* voice = findFreeVoice();
        if (voice != nullptr)
        {
            float attack = parameters.getRawParameterValue (ATTACK_ID)->load();
            float decay = parameters.getRawParameterValue (DECAY_ID)->load();
            float sustain = parameters.getRawParameterValue (SUSTAIN_ID)->load();
            float release = parameters.getRawParameterValue (RELEASE_ID)->load();

            voice->setEnvelopeParams (attack, decay, sustain, release);
            voice->startNote (message.getNoteNumber(), message.getFloatVelocity());
        }
    }
    else if (message.isNoteOff() || (message.isNoteOn() && message.getVelocity() == 0))
    {
        // Handle both note-off and note-on with velocity 0
        auto* voice = findVoiceForNote (message.getNoteNumber());
        if (voice != nullptr)
            voice->stopNote();
    }
    else if (message.isAllNotesOff() || message.isAllSoundOff())
    {
        for (auto& voice : voices)
            voice.stopNote();
    }
}

void PluginProcessor::addMidiMessage (const juce::MidiMessage& message)
{
    const juce::ScopedLock sl (midiLock);
    pendingMidiMessages.addEvent (message, 0);
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    if (!hasSampleLoaded())
        return;

    // Handle MIDI from DAW/host
    for (const auto metadata : midiMessages)
        handleMidiEvent (metadata.getMessage());
    
    // Handle MIDI from UI keyboard
    {
        const juce::ScopedLock sl (midiLock);
        for (const auto metadata : pendingMidiMessages)
            handleMidiEvent (metadata.getMessage());
        pendingMidiMessages.clear();
    }

    // Get parameters
    float position = parameters.getRawParameterValue (POSITION_ID)->load();
    float grainSizeMs = parameters.getRawParameterValue (GRAIN_SIZE_ID)->load();
    float density = parameters.getRawParameterValue (DENSITY_ID)->load();
    float pitch = parameters.getRawParameterValue (PITCH_ID)->load();
    float spray = parameters.getRawParameterValue (SPRAY_ID)->load();
    float gain = parameters.getRawParameterValue (GAIN_ID)->load();

    auto* leftChannel = buffer.getWritePointer (0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : leftChannel;

    // Process all voices
    for (size_t v = 0; v < voices.size(); ++v)
    {
        voices[v].processGrains (sampleBuffer, sampleSampleRate,
                                  leftChannel, rightChannel, buffer.getNumSamples(),
                                  position, grainSizeMs, density, pitch, spray, gain,
                                  voiceVisuals[v]);
    }
}

std::array<GrainVisualInfo, 64> PluginProcessor::getGrainVisuals() const
{
    std::array<GrainVisualInfo, 64> result {};
    int idx = 0;
    for (const auto& vv : voiceVisuals)
    {
        for (const auto& vis : vv)
        {
            if (idx < 64)
                result[static_cast<size_t> (idx++)] = vis;
        }
    }
    return result;
}

bool PluginProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    // Add sample path to state
    state.setProperty ("samplePath", loadedSamplePath, nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName (parameters.state.getType()))
    {
        auto newState = juce::ValueTree::fromXml (*xmlState);
        parameters.replaceState (newState);

        // Reload sample if path exists
        juce::String savedPath = newState.getProperty ("samplePath", "").toString();
        if (savedPath.isNotEmpty())
        {
            juce::File file (savedPath);
            if (file.existsAsFile())
                loadSampleAsync (file);
        }
    }
}

void PluginProcessor::savePreset (const juce::File& file)
{
    auto state = parameters.copyState();
    state.setProperty ("samplePath", loadedSamplePath, nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    xml->writeTo (file);
}

void PluginProcessor::loadPreset (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml != nullptr)
    {
        auto newState = juce::ValueTree::fromXml (*xml);
        parameters.replaceState (newState);

        juce::String savedPath = newState.getProperty ("samplePath", "").toString();
        if (savedPath.isNotEmpty())
        {
            juce::File sampleFile (savedPath);
            if (sampleFile.existsAsFile())
                loadSampleAsync (sampleFile);
        }
    }
}

juce::StringArray PluginProcessor::getPresetNames() const
{
    juce::StringArray names;
    auto presetDir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                         .getChildFile ("Wavero/Presets");

    if (presetDir.exists())
    {
        for (auto& file : presetDir.findChildFiles (juce::File::findFiles, false, "*.wvpreset"))
            names.add (file.getFileNameWithoutExtension());
    }

    return names;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
