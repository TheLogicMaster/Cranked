#pragma once

#include "gen/PlaydateAPI.hpp"
#include "HeapAllocator.hpp"
#include "Rom.hpp"

namespace cranked {

    class Cranked;

    class Audio;

    inline float noteToFrequency(MIDINote note) {
        return NOTE_C4_FREQUENCY * (float) pow(2, (note - NOTE_C4_HALF_STEPS) / 12);
    }

    inline MIDINote noteFromFrequency(float frequency) {
        return NOTE_C4_HALF_STEPS + log2(frequency / NOTE_C4_FREQUENCY) * 12;
    }

    inline float soundFramesToSeconds(int frames) {
        return (float) frames / AUDIO_SAMPLING_RATE;
    }

    inline int soundFramesFromSeconds(float seconds) {
        return (int) round(seconds * AUDIO_SAMPLING_RATE);
    }

    inline int bytesPerSoundFrame(SoundFormat format) {
        switch (format) {
            case SoundFormat::Mono8bit: return 1;
            case SoundFormat::Stereo8bit:
            case SoundFormat::Mono16bit: return 2;
            case SoundFormat::Stereo16bit: return 4;
            case SoundFormat::MonoADPCM: return 2;
            case SoundFormat::StereoADPCM: return 4;
            default: return 0;
        }
    }

    inline bool soundFormatIsStereo(SoundFormat format) {
        return (int)format & 0x1;
    }

    inline bool soundFormatIs16Bit(SoundFormat format) {
        return format >= SoundFormat::Mono16bit;
    }

    struct SoundSource_32 : NativeResource {
        SoundSource_32(Cranked &cranked, ResourceType type, void *address);

        // Don't copy callbacks
        SoundSource_32(const SoundSource_32 &other, ResourceType type, void *address);

        ~SoundSource_32() override;

        virtual void sampleAudio(int16 *left, int16 *right, int length) = 0;

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
        SoundEffect_32(Cranked &cranked, ResourceType type, void *address);

        ~SoundEffect_32() override;

        /// Process the left and right buffers of length samples and return true if changes were made.
        /// Active refers to samples being present in input buffers.
        // Todo: When does right need to be processed? Docs say when on `stereo channel`.
        virtual bool process(int32 *left, int32 *right, int length, bool active) = 0;

        float mixLevel = 1.0f;
        SynthSignalValueRef mixModulator{};
        cref_t userdata{};
    };

    struct PDSynthSignalValue_32 : NativeResource {
        PDSynthSignalValue_32(Cranked &cranked, ResourceType type, void *address);

        ~PDSynthSignalValue_32() override;
    };

    struct SoundChannel_32 : NativeResource {
        explicit SoundChannel_32(Cranked &cranked);

        unordered_resource_set<SoundSource> sources{};
        unordered_resource_set<SoundEffect> effects{};
        float volume = 1.0f;
        float pan = 0.5f;
        SynthSignalValueRef volumeModulator{};
        SynthSignalValueRef panModulator{};
        const SoundChannelSignalRef wetSignal;
        const SoundChannelSignalRef drySignal;
    };

    struct SoundChannelSignal_32 : PDSynthSignalValue_32 {
        SoundChannelSignal_32(SoundChannel_32 *channel, bool wet);

        SoundChannel channel;
        const bool wet;
    };

    struct AudioPlayerBase : SoundSource_32 {
        AudioPlayerBase(Cranked &cranked, ResourceType type, void *address);

        AudioPlayerBase(const AudioPlayerBase &other, ResourceType type, void *address);

        int sampleOffset{};
        float rate = 1.0f;
        bool paused{};
        int repeat{};
        int loops{};
        int loopStart{}, loopEnd{};
        cref_t loopCallback{};
        cref_t loopCallbackUserdata{};
        SynthSignalValueRef rateModulator{};
    };

    struct FilePlayer_32 : AudioPlayerBase {
        explicit FilePlayer_32(Cranked &cranked);

        void sampleAudio(int16 *left, int16 *right, int length) override {
            // Todo
        }

        FileRef file{};
        bool stopOnUnderrun{};
        bool underran{};
        float leftFadeTarget{}, rightFadeTarget{};
        float leftFadeSpeed{}, rightFadeSpeed{};
        cref_t volumeFadeCallback{};
    };

    struct SamplePlayer_32 : AudioPlayerBase {
        explicit SamplePlayer_32(Cranked &cranked);

        SamplePlayer copy();

        void sampleAudio(int16 *left, int16 *right, int length) override {
            // Todo
        }

        AudioSampleRef sample{};
        float oneShotLeftVolume{}, oneShotRightVolume{};
        int oneShotTime{};
        float oneShotRate{};
        bool reversed{}; // Ping pong'd backwards
    };

    struct PDSynthSignal_32 : PDSynthSignalValue_32 {
        PDSynthSignal_32(Cranked &cranked, ResourceType type, void *address);
        ~PDSynthSignal_32() override;

        /// Returns the signal value at the end of the frame. Called with the number of remaining samples in ioSamples.
        /// To set an interframe value, set ioSamples to sample offset and interframeValue to said value.
        // virtual float stepSignal(int32 &ioSamples, float &interframeValue) = 0; // Todo: Offset from beginning? Detect with just a changed value?

        /// -1 in length means indefinite.
        // virtual void handleNoteOn(MIDINote note, float vel, float length) = 0; // Todo: Velocity?

        /// Stopped means stop completely, otherwise just released.
        // virtual void handleNoteOff(bool stopped, int32_t offset) = 0; // Todo: Offset in frame?

        float offset{};
        float scale{};
    };

    struct PDSynthLFO_32 : PDSynthSignal_32 {
        PDSynthLFO_32(Cranked &cranked, LFOType type);

        // float stepSignal(int32 &ioSamples, float &interframeValue) override {
        //     return 1; // Todo
        // }
        //
        // void handleNoteOn(MIDINote note, float vel, float length) override {}
        //
        // void handleNoteOff(bool stopped, int32_t offset) override {}

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

        void sampleAudio(int16 *left, int16 *right, int length) override {
            // Todo
        }

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

        void sampleAudio(int16 *left, int16 *right, int length) override {
            // Todo
        }

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

        void sampleAudio(int16 *left, int16 *right, int length) override {
            // Todo
        }

        SynthInstrumentRef instrument{};
        bool muted{};
    };

    struct SoundSequence_32 : SoundSource_32 {
        explicit SoundSequence_32(Cranked &cranked);

        void sampleAudio(int16 *left, int16 *right, int length) override {
            // Todo
        }

        int time{};
        float tempo{}; // Steps per second
        int currentLoop{};
        int loops{};
        int loopStart{}, loopEnd{};
    };

    struct TwoPoleFilter_32 : SoundEffect_32 {
        explicit TwoPoleFilter_32(Cranked &cranked);

        bool process(int32 *left, int32 *right, int length, bool active) override {
            return false; // Todo
        }

        TwoPoleFilterType type{};
        float frequency{};
        SynthSignalValueRef frequencyModulator{};
        float gain{};
        float resonance{};
        SynthSignalValueRef resonanceModulator{};
    };

    struct OnePoleFilter_32 : SoundEffect_32 {
        explicit OnePoleFilter_32(Cranked &cranked);

        bool process(int32 *left, int32 *right, int length, bool active) override {
            return false; // Todo
        }

        float cutoffFrequency{};
        SynthSignalValueRef cutoffFrequencyModulator{};
    };

    struct BitCrusher_32 : SoundEffect_32 {
        explicit BitCrusher_32(Cranked &cranked);

        bool process(int32 *left, int32 *right, int length, bool active) override {
            return false; // Todo
        }

        float amount{};
        SynthSignalValueRef amountModulator{};
        float undersampling{};
        SynthSignalValueRef undersamplingModulator{};
    };

    struct RingModulator_32 : SoundEffect_32 {
        explicit RingModulator_32(Cranked &cranked);

        bool process(int32 *left, int32 *right, int length, bool active) override {
            return false; // Todo
        }

        float frequency{};
        SynthSignalValueRef frequencyModulator{};
    };

    struct DelayLine_32 : SoundEffect_32 {
        explicit DelayLine_32(Cranked &cranked, int length, bool stereo);

        bool process(int32 *left, int32 *right, int length, bool active) override {
            return false; // Todo
        }

        const bool stereo;
        vector<int32> data;
        float feedback{};
    };

    struct DelayLineTap_32 : SoundSource_32 {
        explicit DelayLineTap_32(Cranked &cranked, DelayLine delayLine, int delay);

        void sampleAudio(int16 *left, int16 *right, int length) override {
            // Todo
        }

        DelayLineRef delayLine;
        int delayFrames{};
        SynthSignalValueRef delayModulator{};
        bool channelsFlipped{};
    };

    struct Overdrive_32 : SoundEffect_32 {
        explicit Overdrive_32(Cranked &cranked);

        bool process(int32 *left, int32 *right, int length, bool active) override {
            return false; // Todo
        }

        float gain{};
        float limit{};
        SynthSignalValueRef limitModulator{};
        float offset{};
        SynthSignalValueRef offsetModulator{};
    };

    struct CustomSoundSource_32 : SoundSource_32 {
        CustomSoundSource_32(Cranked &cranked, cref_t func, void *userdata, bool stereo);

        void sampleAudio(int16 *left, int16 *right, int length) override {
            // Todo
        }

        cref_t func;
        void *userdata;
        bool stereo;
    };

    struct CustomSoundEffect_32 : SoundEffect_32 {
        CustomSoundEffect_32(Cranked &cranked, cref_t func, void *userdata);

        bool process(int32 *left, int32 *right, int length, bool active) override {
            return false; // Todo
        }

        cref_t func;
        void *userdata;
    };

    struct CustomSoundSignal_32 : PDSynthSignal_32 {
        CustomSoundSignal_32(Cranked &cranked, cref_t stepFunc, cref_t noteOnFunc, cref_t noteOffFunc, cref_t deallocFunc, void *userdata);

        cref_t stepFunc, noteOnFunc, noteOffFunc, deallocFunc;
        void *userdata;
    };

    class Audio {
    public:
        explicit Audio(Cranked &cranked);

        void sampleAudio(int16 *samples, int len);

        void reset();

        void init();

        // Todo: Change to take in pointer type as template param
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
            channels.emplace(channel);
            return channel;
        }

        AudioSample loadSample(const string &path);

        [[nodiscard]] SoundSource sourceFromDerived(void *derived) const {
            if (auto it = sourceMappings.find(derived); it != sourceMappings.end())
                return it->second;
            return (SoundSource)derived;
        }

        [[nodiscard]] SoundEffect effectFromDerived(void *derived) const {
            if (auto it = effectMappings.find(derived); it != effectMappings.end())
                return it->second;
            return (SoundEffect)derived;
        }

        [[nodiscard]] SynthSignalValue signalFromDerived(void *derived) const {
            if (auto it = signalMappings.find(derived); it != signalMappings.end())
                return it->second;
            return (SynthSignalValue)derived;
        }

        Cranked &cranked;
        HeapAllocator &heap;
        SoundChannelRef mainChannel;
        unordered_resource_set<SoundChannel> channels{};

        int sampleTime{};
        cref_t lastError{};
        cref_t micCallback{};
        cref_t micCallbackUserdata{};
        MicSource micCallbackSource{};
        cref_t headsetStateCallback{};
        bool headphonesConnected{}, headsetMicConnected{};
        bool headphoneOutputActive{}, speakerOutputActive{};

        // Mappings from derived object addresses to base addresses (Protect against different class layouts)
        unordered_map<void *, SoundSource> sourceMappings{};
        unordered_map<void *, SoundEffect> effectMappings{};
        unordered_map<void *, SynthSignalValue> signalMappings{};
    };

}
