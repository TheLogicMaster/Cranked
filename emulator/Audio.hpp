#pragma once

#include "gen/PlaydateAPI.hpp"
#include "PlaydateTypes.hpp"
#include "Constants.hpp"
#include "HeapAllocator.hpp"

#include <cstdint>
#include <set>
#include <vector>

class Emulator;
class Audio;

struct SoundSource_32 {
    explicit SoundSource_32(Audio *audio);
    SoundSource_32(const SoundSource_32 &other) = delete;
    SoundSource_32(SoundSource_32 &&other) = delete;
    virtual ~SoundSource_32() = default;

    const Audio *audio;
    float leftVolume = 1.0f, rightVolume = 1.0f;
    bool playing{};
    cref_t completionCallback{};
    cref_t completionCallbackUserdata{};
};

struct AudioSample_32 {
    AudioSample_32(Audio *audio, int size);
    AudioSample_32(const AudioSample_32 &other) = delete;
    AudioSample_32(AudioSample_32 &&other) = delete;

    const Audio *audio;
    vheap_vector<uint8_t> data;
    uint32_t sampleRate{};
    SoundFormat format{};
};

struct SoundEffect_32 {
    explicit SoundEffect_32(Audio *audio);
    SoundEffect_32(const SoundEffect_32 &other) = delete;
    SoundEffect_32(SoundEffect_32 &&other) = delete;
    virtual ~SoundEffect_32() = default;

    const Audio *audio;
    float mixLevel = 1.0f;
    PDSynthSignalValue_32 *mixModulator{};
    cref_t userdata{};
};

struct PDSynthSignalValue_32 {
    explicit PDSynthSignalValue_32(Audio *audio);
    PDSynthSignalValue_32(const PDSynthSignalValue_32 &other) = delete;
    PDSynthSignalValue_32(PDSynthSignalValue_32 &&other) = delete;
    virtual ~PDSynthSignalValue_32() = default;

//    virtual float stepSignal() = 0; // Todo: ioSamples and interframe value parameters? Note on/off functions?

    const Audio *audio;
};

struct SoundChannel_32 {
    struct Signal : PDSynthSignalValue_32 {
        explicit Signal(SoundChannel_32 *channel, bool wet);
//        float stepSignal() override;
        SoundChannel_32 *channel;
        const bool wet;
    };

    explicit SoundChannel_32(Audio *audio);
    SoundChannel_32(const SoundChannel_32 &other) = delete;
    SoundChannel_32(SoundChannel_32 &&other) = delete;
    virtual ~SoundChannel_32() = default;

    Audio *audio;
    std::vector<SoundSource_32 *> sources{};
    std::vector<SoundEffect_32 *> effects{};
    float volume = 1.0f;
    float pan = 0.5f;
    PDSynthSignalValue_32 *volumeModulator{};
    PDSynthSignalValue_32 *panModulator{};
    Signal wetSignal{this, true};
    Signal drySignal{this, false};
};

struct FilePlayer_32 : SoundSource_32 {
    explicit FilePlayer_32(Audio *audio);

    SDFile_32 *file{};
    int sampleOffset{};
    float rate = 1.0f;
    int repeat{};
    int loops{};
    int loopStart{}, loopEnd{};
    bool stopOnUnderrun{};
    bool underran{};
    float leftFadeTarget{}, rightFadeTarget{};
    float leftFadeSpeed{}, rightFadeSpeed{};
    cref_t volumeFadeCallback{};
    cref_t loopCallback{};
    cref_t loopCallbackUserdata{};
};

struct SamplePlayer_32 : SoundSource_32 {
    explicit SamplePlayer_32(Audio *audio);

    AudioSample_32 *sample{};
    int sampleOffset{};
    float rate = 1.0f;
    int repeat{};
    int loops{};
    int loopStart{}, loopEnd{};
    cref_t loopCallback{};
    cref_t loopCallbackUserdata{};
};

struct PDSynthSignal_32 : PDSynthSignalValue_32 {
    explicit PDSynthSignal_32(Audio *audio);
};

struct PDSynthLFO_32 : PDSynthSignal_32 {
    PDSynthLFO_32(Audio *audio, LFOType type);

    LFOType type;
    float rate{};
    float phase{};
    float startPhase{};
    float center{};
    float depth{};
    cref_t function{};
    bool functionInterpolate{};
    cref_t functionUserdata{};
    std::vector<float> arpeggiationSteps{};
    int holdOffSamples{}, rampTimeSamples{};
    bool reTrigger{};
    bool global{};
};

struct PDSynthEnvelope_32 : PDSynthSignal_32 {
    explicit PDSynthEnvelope_32(Audio *audio, float attack = 0, float decay = 0, float sustain = 0, float release = 0);

    float attack, decay, sustain, release;
    float curvature{};
    float velocitySensitivity{};
    float rateScaling{};
    MIDINote rateStart{}, rateEnd{};
    bool legato{};
    bool reTrigger{};
};

struct NoteEvent {
    bool on;
    float frequency;
    float vel;
    int length;
};

struct PDSynth_32 : SoundSource_32 {
    explicit PDSynth_32(Audio *audio);

    SoundWaveform waveform{};
    bool generatorStereo{};
    cref_t generatorUserdata{};
    cref_t generatorRenderFunc{};
    cref_t generatorNoteOnFunc{};
    cref_t generatorReleaseFunc{};
    cref_t generatorSetParameterFunc{};
    cref_t generatorDeallocFunc{};
    cref_t generatorCopyUserdataFunc{};
    AudioSample_32 *sample{};
    uint32_t sampleSustainStart{}, sampleSustainEnd{};
    int waveTableLog2size{};
    int waveTableColumns{}, waveTableRows{};
    PDSynthEnvelope_32 envelope;
    float transposeHalfSteps{};
    PDSynthSignalValue_32 *frequencyModulator{};
    PDSynthSignalValue_32 *amplitudeModulator{};
    std::multimap<uint32_t, NoteEvent> noteEvents{};
    float parameters[4]{};
    PDSynthSignalValue_32 *parameterModulators[4]{};
    int playingTime{};
    float instrumentStartFrequency{}, instrumentEndFrequency{};
    float instrumentTranspose{}; // Todo: Is this supposed to be separate?
};

struct ControlSignal_32 : PDSynthSignal_32 {
    struct Event {
        float value;
        bool interpolate;
    };

    explicit ControlSignal_32(Audio *audio);

    std::multimap<int32_t, Event> events{};
    int controllerNumber{};
};

struct PDSynthInstrument_32 : SoundSource_32 {
    explicit PDSynthInstrument_32(Audio *audio);

//    void playNote(float frequency, float vel, float length, uint32_t when);
//    void setNoteOff(float frequency, uint32_t when);
//    void setAllNotesOff(uint32_t when);
//    int countVoicesPlaying();

    std::vector<PDSynth_32 *> voices{};
    float pitchBend{};
    float pitchBendRangeHalfSteps{};
    float transposeHalfSteps{};
};

struct SequenceTrack_32 : SoundSource_32 {
    explicit SequenceTrack_32(Audio *audio);

    PDSynthInstrument_32 *instrument{};
    bool muted{};
};

struct SoundSequence_32 : SoundSource_32 {
    explicit SoundSequence_32(Audio *audio);

    int time{};
};

struct TwoPoleFilter_32 : SoundEffect_32 {
    explicit TwoPoleFilter_32(Audio *audio);

    TwoPoleFilterType type{};
    float frequency{};
    PDSynthSignalValue_32 *frequencyModulator{};
    float gain{};
    float resonance{};
    PDSynthSignalValue_32 *resonanceModulator{};
};

struct OnePoleFilter_32 : SoundEffect_32 {
    explicit OnePoleFilter_32(Audio *audio);

    float cutoffFrequency{};
    PDSynthSignalValue_32 *cutoffFrequencyModulator{};
};

struct BitCrusher_32 : SoundEffect_32 {
    explicit BitCrusher_32(Audio *audio);

    float amount{};
    PDSynthSignalValue_32 *amountModulator{};
    float undersampling{};
    PDSynthSignalValue_32 *undersamplingModulator{};
};

struct RingModulator_32 : SoundEffect_32 {
    explicit RingModulator_32(Audio *audio);

    float frequency{};
    PDSynthSignalValue_32 *frequencyModulator{};
};

struct DelayLine_32 : SoundEffect_32 {
    explicit DelayLine_32(Audio *audio, int length, bool stereo);

    const bool stereo;
    std::vector<int32_t> data;
    float feedback{};
};

struct DelayLineTap_32 : SoundSource_32 {
    explicit DelayLineTap_32(Audio *audio, DelayLine_32 *delayLine, int delay);

    DelayLine_32 * const delayLine;
    int delayFrames{};
};

struct Overdrive_32 : SoundEffect_32 {
    explicit Overdrive_32(Audio *audio);

    float gain{};
    float limit{};
    PDSynthSignalValue_32 *limitModulator{};
    float offset{};
    PDSynthSignalValue_32 *offsetModulator{};
};

class Audio {
public:
    explicit Audio(Emulator *emulator);

    void sampleAudio(int16_t *samples, int len);

    void reset();

    SoundChannel_32 *allocateChannel();

    void freeChannel(SoundChannel_32 *channel);

    template<class T, typename ...Args>
    T *allocateSoundSource(Args ...args);

    void freeSoundSource(SoundSource_32 *source);

    template<class T, typename ...Args>
    T *allocateSoundEffect(Args ...args);

    void freeSoundEffect(SoundEffect_32 *effect);

    template<class T, typename ...Args>
    T *allocateSynthSignal(Args ...args);

    void freeSynthSignal(PDSynthSignalValue_32 *signal);

    AudioSample_32 *allocateAudioSample(int size);

    void freeAudioSample(AudioSample_32 *sample);

    Emulator *emulator;
    HeapAllocator &heap;
    SoundChannel_32 *mainChannel;
    int sampleTime{};
    cref_t lastError{};
    std::set<SoundSource_32 *> soundSources{};
    std::set<SoundChannel_32 *> soundChannels{};
    std::set<SoundEffect_32 *> soundEffects{};
    std::set<AudioSample_32 *> audioSamples{};
    std::set<PDSynthSignalValue_32 *> synthSignals{};
};
