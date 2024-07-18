#pragma once

#include "gen/PlaydateAPI.hpp"
#include "HeapAllocator.hpp"
#include "NativeResource.hpp"

namespace cranked {

    class Cranked;

    class Audio;

    inline float noteToFrequency(MIDINote note) {
        return NOTE_C4_FREQUENCY * (float) pow(2, (note - NOTE_C4_HALF_STEPS) / 12);
    }

    inline MIDINote noteFromFrequency(float frequency) {
        return NOTE_C4_HALF_STEPS + log2(frequency / NOTE_C4_FREQUENCY) * 12;
    }

    inline float framesToSeconds(int frames) {
        return (float) frames / AUDIO_SAMPLING_RATE;
    }

    inline int framesFromSeconds(float seconds) {
        return (int) round(seconds * AUDIO_SAMPLING_RATE);
    }

    struct SoundSource_32 : NativeResource {
        explicit SoundSource_32(Cranked &cranked);

        // Don't copy callbacks
        SoundSource_32(const SoundSource_32 &other) : NativeResource(cranked), leftVolume(other.leftVolume), rightVolume(other.rightVolume), playing(other.playing) {}

        ~SoundSource_32() override;

        float leftVolume = 1.0f, rightVolume = 1.0f;
        bool playing{};
        cref_t completionCallback{};
        cref_t completionCallbackUserdata{};
    };

    struct AudioSample_32 : NativeResource {
        AudioSample_32(Cranked &cranked, int size);

        vheap_vector<uint8> data;
        uint32 sampleRate{};
        SoundFormat soundFormat{};
    };

    struct SoundEffect_32 : NativeResource {
        explicit SoundEffect_32(Cranked &cranked);

        ~SoundEffect_32() override;

        float mixLevel = 1.0f;
        SynthSignalValueRef mixModulator{};
        cref_t userdata{};
    };

    struct PDSynthSignalValue_32 : NativeResource {
        explicit PDSynthSignalValue_32(Cranked &cranked);

        ~PDSynthSignalValue_32() override;

//    virtual float stepSignal() = 0; // Todo: ioSamples and interframe value parameters? Note on/off functions?
    };

    struct SoundChannel_32 : NativeResource {
        struct Signal : PDSynthSignalValue_32 {
            explicit Signal(SoundChannel_32 *channel, bool wet);

//        float stepSignal() override;
            SoundChannel channel;
            const bool wet;
        };

        explicit SoundChannel_32(Cranked &cranked);

        ~SoundChannel_32() override;

        unordered_resource_set<SoundSource> sources{};
        unordered_resource_set<SoundEffect> effects{};
        float volume = 1.0f;
        float pan = 0.5f;
        SynthSignalValueRef volumeModulator{};
        SynthSignalValueRef panModulator{};
        Signal wetSignal{this, true};
        Signal drySignal{this, false};
    };

    struct AudioPlayerBase : SoundSource_32 {
        explicit AudioPlayerBase(Cranked &cranked);

        AudioPlayerBase(const AudioPlayerBase &other) : SoundSource_32(other), rate(other.rate) {}

        int sampleOffset{};
        float rate = 1.0f;
        bool paused{};
        int repeat{};
        int loops{};
        int loopStart{}, loopEnd{};
        cref_t loopCallback{};
        cref_t loopCallbackUserdata{};
    };

    struct FilePlayer_32 : AudioPlayerBase {
        explicit FilePlayer_32(Cranked &cranked);

        FileRef file{};
        bool stopOnUnderrun{};
        bool underran{};
        float leftFadeTarget{}, rightFadeTarget{};
        float leftFadeSpeed{}, rightFadeSpeed{};
        cref_t volumeFadeCallback{};
    };

    struct SamplePlayer_32 : AudioPlayerBase {
        explicit SamplePlayer_32(Cranked &cranked);

        AudioSampleRef sample{};
    };

    struct PDSynthSignal_32 : PDSynthSignalValue_32 {
        explicit PDSynthSignal_32(Cranked &cranked);

        int offset{};
        float scale{};
    };

    struct PDSynthLFO_32 : PDSynthSignal_32 {
        PDSynthLFO_32(Cranked &cranked, LFOType type);

        LFOType type;
        float rate{};
        float phase{};
        float startPhase{};
        float center{};
        float depth{};
        cref_t func{};
        bool functionInterpolate{};
        cref_t functionUserdata{};
        vector<float> arpeggiationSteps{};
        int holdOffSamples{}, rampTimeSamples{};
        bool reTrigger{};
        bool global{};
    };

    struct PDSynthEnvelope_32 : PDSynthSignal_32 {
        explicit PDSynthEnvelope_32(Cranked &cranked, float attack = 0, float decay = 0, float sustain = 0, float release = 0);

        float attack, decay, sustain, release;
        float curvature{};
        float velocitySensitivity{};
        float rateScaling{};
        float rateStart{}, rateEnd{};
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
        explicit PDSynth_32(Cranked &cranked);

        SoundWaveform waveform{};
        bool generatorStereo{};
        cref_t generatorUserdata{};
        cref_t generatorRenderFunc{};
        cref_t generatorNoteOnFunc{};
        cref_t generatorReleaseFunc{};
        cref_t generatorSetParameterFunc{};
        cref_t generatorDeallocFunc{};
        cref_t generatorCopyUserdataFunc{};
        AudioSampleRef sample{};
        uint32 sampleSustainStart{}, sampleSustainEnd{};
        int waveTableLog2size{};
        int waveTableColumns{}, waveTableRows{};
        SynthEnvelopeRef envelope;
        float transposeHalfSteps{};
        SynthSignalValueRef frequencyModulator{};
        SynthSignalValueRef amplitudeModulator{};
        multimap<uint32, NoteEvent> noteEvents{};
        float parameters[4]{};
        SynthSignalValueRef parameterModulators[4]{};
        int playingTime{};
        float instrumentStartFrequency{}, instrumentEndFrequency{};
        float instrumentTranspose{}; // Todo: Is this supposed to be separate?
    };

    struct ControlSignal_32 : PDSynthSignal_32 {
        struct Event {
            float value;
            bool interpolate;
        };

        explicit ControlSignal_32(Cranked &cranked);

        multimap<int32, Event> events{};
        int controllerNumber{};
    };

    struct PDSynthInstrument_32 : SoundSource_32 {
        explicit PDSynthInstrument_32(Cranked &cranked);

//    void playNote(float frequency, float vel, float length, uint32 when);
//    void setNoteOff(float frequency, uint32 when);
//    void setAllNotesOff(uint32 when);
//    int countVoicesPlaying();

        unordered_resource_set<Synth> voices{};
        float pitchBend{};
        float pitchBendRangeHalfSteps{};
        float transposeHalfSteps{};
    };

    struct SequenceTrack_32 : SoundSource_32 {
        explicit SequenceTrack_32(Cranked &cranked);

        SynthInstrumentRef instrument{};
        bool muted{};
    };

    struct SoundSequence_32 : SoundSource_32 {
        explicit SoundSequence_32(Cranked &cranked);

        int time{};
        float tempo{}; // Steps per second
        int currentLoop{};
        int loops{};
        int loopStart{}, loopEnd{};
    };

    struct TwoPoleFilter_32 : SoundEffect_32 {
        explicit TwoPoleFilter_32(Cranked &cranked);

        TwoPoleFilterType type{};
        float frequency{};
        SynthSignalValueRef frequencyModulator{};
        float gain{};
        float resonance{};
        SynthSignalValueRef resonanceModulator{};
    };

    struct OnePoleFilter_32 : SoundEffect_32 {
        explicit OnePoleFilter_32(Cranked &cranked);

        float cutoffFrequency{};
        SynthSignalValueRef cutoffFrequencyModulator{};
    };

    struct BitCrusher_32 : SoundEffect_32 {
        explicit BitCrusher_32(Cranked &cranked);

        float amount{};
        SynthSignalValueRef amountModulator{};
        float undersampling{};
        SynthSignalValueRef undersamplingModulator{};
    };

    struct RingModulator_32 : SoundEffect_32 {
        explicit RingModulator_32(Cranked &cranked);

        float frequency{};
        SynthSignalValueRef frequencyModulator{};
    };

    struct DelayLine_32 : SoundEffect_32 {
        explicit DelayLine_32(Cranked &cranked, int length, bool stereo);

        const bool stereo;
        vector<int32> data;
        float feedback{};
    };

    struct DelayLineTap_32 : SoundSource_32 {
        explicit DelayLineTap_32(Cranked &cranked, DelayLine_32 *delayLine, int delay);

        DelayLineRef delayLine;
        int delayFrames{};
        SynthSignalValueRef delayModulator{};
        bool channelsFlipped{};
    };

    struct Overdrive_32 : SoundEffect_32 {
        explicit Overdrive_32(Cranked &cranked);

        float gain{};
        float limit{};
        SynthSignalValueRef limitModulator{};
        float offset{};
        SynthSignalValueRef offsetModulator{};
    };

    class Audio {
    public:
        explicit Audio(Cranked &cranked);

        void sampleAudio(int16 *samples, int len);

        void reset();

        void init();

        template<class T, typename ...Args> requires derived_from<T, SoundSource_32>
        T *allocateSource(Args ...args) {
            T *source = heap.construct<T>(cranked, args...);
            mainChannel->sources.emplace(source);
            return source;
        }

        template<class T, typename ...Args>
        T *allocateReferencedSource(Args ...args) {
            T *source = allocateSource<T>(args...);
            source->reference();
            return source;
        }

        SoundChannel allocateChannel() {
            auto channel = heap.construct<SoundChannel_32>(cranked);
            activeChannels.emplace(channel);
            return channel;
        }

        Cranked &cranked;
        HeapAllocator &heap;
        SoundChannelRef mainChannel;
        unordered_resource_set<SoundChannel> activeChannels{};
        int sampleTime{};
        cref_t lastError{};
        cref_t micCallback{};
        cref_t micCallbackUserdata{};
        MicSource micCallbackSource{};
        cref_t headsetStateCallback{};
        bool headphonesConnected{}, headsetMicConnected{};
        bool headphoneOutputActive{}, speakerOutputActive{};
    };

}
