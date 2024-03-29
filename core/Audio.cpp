#include "Audio.hpp"
#include "Emulator.hpp"

SoundSource_32::SoundSource_32(Audio *audio) : audio(audio) {}

AudioSample_32::AudioSample_32(Audio *audio, int size) : audio(audio), data(vheap_vector<uint8_t>(size, audio->heap.allocator<uint8_t>())) {}

SoundEffect_32::SoundEffect_32(Audio *audio) : audio(audio) {}

SoundChannel_32::SoundChannel_32(Audio *audio) : audio(audio) {}

SoundChannel_32::Signal::Signal(SoundChannel_32 *channel, bool wet) : PDSynthSignalValue_32(channel->audio), channel(channel), wet(wet) {}

FilePlayer_32::FilePlayer_32(Audio *audio) : SoundSource_32(audio) {}

SamplePlayer_32::SamplePlayer_32(Audio *audio) : SoundSource_32(audio) {}

PDSynthSignalValue_32::PDSynthSignalValue_32(Audio *audio) : audio(audio) {}

PDSynthSignal_32::PDSynthSignal_32(Audio *audio) : PDSynthSignalValue_32(audio) {}

SequenceTrack_32::SequenceTrack_32(Audio *audio) : SoundSource_32(audio) {}

SoundSequence_32::SoundSequence_32(Audio *audio) : SoundSource_32(audio) {}

TwoPoleFilter_32::TwoPoleFilter_32(Audio *audio) : SoundEffect_32(audio) {}

OnePoleFilter_32::OnePoleFilter_32(Audio *audio) : SoundEffect_32(audio) {}

BitCrusher_32::BitCrusher_32(Audio *audio) : SoundEffect_32(audio) {}

RingModulator_32::RingModulator_32(Audio *audio) : SoundEffect_32(audio) {}

DelayLine_32::DelayLine_32(Audio *audio, int length, bool stereo) : SoundEffect_32(audio), data((stereo ? 2 : 1) * length), stereo(stereo) {}

DelayLineTap_32::DelayLineTap_32(Audio *audio, DelayLine_32 *delayLine, int delay) : SoundSource_32(audio), delayLine(delayLine), delayFrames(delay) {}

Overdrive_32::Overdrive_32(Audio *audio) : SoundEffect_32(audio) {}

PDSynthLFO_32::PDSynthLFO_32(Audio *audio, LFOType type) : PDSynthSignal_32(audio), type(type) {}

PDSynthEnvelope_32::PDSynthEnvelope_32(Audio *audio, float attack, float decay, float sustain, float release)
        : PDSynthSignal_32(audio), attack(attack), decay(decay), sustain(sustain), release(release) {}

PDSynth_32::PDSynth_32(Audio *audio) : SoundSource_32(audio), envelope(audio) {}

ControlSignal_32::ControlSignal_32(Audio *audio) : PDSynthSignal_32(audio) {}

PDSynthInstrument_32::PDSynthInstrument_32(Audio *audio) : SoundSource_32(audio) {}

Audio::Audio(Emulator *emulator) : emulator(emulator), heap(emulator->heap), mainChannel(heap.construct<SoundChannel_32>(this)) {}

void Audio::sampleAudio(int16_t *samples, int len) {
    // Todo
}

void Audio::reset() {
    sampleTime = 0;
    lastError = 0;
}

SoundChannel_32 *Audio::allocateChannel() {
    auto channel = heap.construct<SoundChannel_32>(this);
    soundChannels.emplace(channel);
    return channel;
}

void Audio::freeChannel(SoundChannel_32 *channel) {
    // Todo: Remove from system
    soundChannels.erase(channel);
    heap.destruct(channel);
}

template<class T, typename ...Args>
T *Audio::allocateSoundSource(Args... args) {
    T *source = heap.construct<T>(this, args...);
    mainChannel->sources.push_back(source);
    soundSources.emplace(source);
    return source;
}

template FilePlayer_32 *Audio::allocateSoundSource();
template SamplePlayer_32 *Audio::allocateSoundSource();
template PDSynth_32 *Audio::allocateSoundSource();
template PDSynthInstrument_32 *Audio::allocateSoundSource();
template SequenceTrack_32 *Audio::allocateSoundSource();
template SoundSequence_32 *Audio::allocateSoundSource();
template DelayLineTap_32 *Audio::allocateSoundSource(DelayLine_32 *delayLine, int delay);

void Audio::freeSoundSource(SoundSource_32 *source) {
    // Todo: Remove from system
    soundSources.erase(source);
    heap.destruct(source);
}

template<class T, typename... Args>
T *Audio::allocateSoundEffect(Args... args) {
    auto effect = heap.construct<T>(this, args...);
    soundEffects.emplace(effect);
    return effect;
}

template TwoPoleFilter_32 *Audio::allocateSoundEffect();
template OnePoleFilter_32 *Audio::allocateSoundEffect();
template BitCrusher_32 *Audio::allocateSoundEffect();
template RingModulator_32 *Audio::allocateSoundEffect();
template DelayLine_32 *Audio::allocateSoundEffect(int length, bool stereo);
template Overdrive_32 *Audio::allocateSoundEffect();

void Audio::freeSoundEffect(SoundEffect_32 *effect) {
    // Todo: Remove from system
    soundEffects.erase(effect);
    heap.destruct(effect);
}

template<class T, typename... Args>
T *Audio::allocateSynthSignal(Args... args) {
    auto signal = heap.construct<T>(this, args...);
    synthSignals.emplace(signal);
    return signal;
}

template PDSynthLFO_32 *Audio::allocateSynthSignal(LFOType type);
template PDSynthEnvelope_32 *Audio::allocateSynthSignal(float attack, float decay, float sustain, float release);
template ControlSignal_32 *Audio::allocateSynthSignal();

void Audio::freeSynthSignal(PDSynthSignalValue_32 *signal) {
    // Todo: Remove from system
    synthSignals.erase(signal);
    heap.destruct(signal);
}

AudioSample_32 *Audio::allocateAudioSample(int size) {
    auto sample = heap.construct<AudioSample_32>(this, size);
    audioSamples.emplace(sample);
    return sample;
}

void Audio::freeAudioSample(AudioSample_32 *sample) {
    // Todo: Remove from system
    audioSamples.erase(sample);
    heap.destruct(sample);
}
