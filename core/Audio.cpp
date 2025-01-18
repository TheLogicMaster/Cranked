#include "Audio.hpp"
#include "Cranked.hpp"

using namespace cranked;

SoundSource_32::SoundSource_32(Cranked &cranked, ResourceType type, void *address) : NativeResource(cranked, type, address) {
    cranked.audio.sourceMappings[address] = this;
}

SoundSource_32::SoundSource_32(const SoundSource_32 &other, ResourceType type, void *address) : SoundSource_32(other.cranked, type, address) {
    leftVolume = other.leftVolume;
    rightVolume = other.rightVolume;
    playing = other.playing;
}

SoundSource_32::~SoundSource_32() {
    cranked.audio.signalMappings.erase(address);
}

AudioSample_32::AudioSample_32(Cranked &cranked, int size)
    : NativeResource(cranked, ResourceType::AudioSample, this), data(vheap_vector(size, cranked.heap.allocator<uint8>())) {}

SoundEffect_32::SoundEffect_32(Cranked &cranked, ResourceType type, void *address) : NativeResource(cranked, type, address) {
    cranked.audio.effectMappings[address] = this;
}

SoundEffect_32::~SoundEffect_32() {
    cranked.audio.effectMappings.erase(address);
}

SoundChannel_32::SoundChannel_32(Cranked &cranked)
    : NativeResource(cranked, ResourceType::Channel, this), wetSignal(cranked.heap.construct<SoundChannelSignal_32>(this, true)), drySignal(cranked.heap.construct<SoundChannelSignal_32>(this, false)) {}

SoundChannelSignal_32::SoundChannelSignal_32(SoundChannel channel, bool wet)
    : PDSynthSignalValue_32(channel->cranked, ResourceType::ChannelSignal, this), channel(channel), wet(wet) {}

AudioPlayerBase::AudioPlayerBase(Cranked &cranked, ResourceType type, void *address) : SoundSource_32(cranked, type, address) {}

AudioPlayerBase::AudioPlayerBase(const AudioPlayerBase &other, ResourceType type, void *address) : AudioPlayerBase(other.cranked, type, address) {
    rate = other.rate;
}

FilePlayer_32::FilePlayer_32(Cranked &cranked) : AudioPlayerBase(cranked, ResourceType::FilePlayer, this) {}

SamplePlayer_32::SamplePlayer_32(Cranked &cranked) : AudioPlayerBase(cranked, ResourceType::SamplePlayer, this) {}

SamplePlayer SamplePlayer_32::copy() {
    auto player = cranked.audio.allocateSource<SamplePlayer_32>();
    player->sample = sample;
    player->leftVolume = leftVolume;
    player->rightVolume = rightVolume;
    player->rate = rate;
    return player;
}

PDSynthSignalValue_32::PDSynthSignalValue_32(Cranked &cranked, ResourceType type, void *address) : NativeResource(cranked, type, address) {
    cranked.audio.signalMappings[address] = this;
}

PDSynthSignalValue_32::~PDSynthSignalValue_32() {
    cranked.audio.signalMappings.erase(address);
}

PDSynthSignal_32::PDSynthSignal_32(Cranked &cranked, ResourceType type, void *address) : PDSynthSignalValue_32(cranked, type, address) {
    cranked.audio.signalMappings[this] = dynamic_cast<SynthSignalValue>(this);
}

PDSynthSignal_32::~PDSynthSignal_32() {
    cranked.audio.signalMappings.erase(this);
}

SequenceTrack_32::SequenceTrack_32(Cranked &cranked) : SoundSource_32(cranked, ResourceType::Track, this) {}

SoundSequence_32::SoundSequence_32(Cranked &cranked) : SoundSource_32(cranked, ResourceType::Sequence, this) {}

TwoPoleFilter_32::TwoPoleFilter_32(Cranked &cranked) : SoundEffect_32(cranked, ResourceType::TwoPoleFilter, this) {}

OnePoleFilter_32::OnePoleFilter_32(Cranked &cranked) : SoundEffect_32(cranked, ResourceType::OnePoleFilter, this) {}

BitCrusher_32::BitCrusher_32(Cranked &cranked) : SoundEffect_32(cranked, ResourceType::BitCrusher, this) {}

RingModulator_32::RingModulator_32(Cranked &cranked) : SoundEffect_32(cranked, ResourceType::RingModulator, this) {}

DelayLine_32::DelayLine_32(Cranked &cranked, int length, bool stereo)
    : SoundEffect_32(cranked, ResourceType::DelayLine, this), stereo(stereo), data((stereo ? 2 : 1) * length) {}

DelayLineTap_32::DelayLineTap_32(Cranked &cranked, DelayLine_32 *delayLine, int delay)
    : SoundSource_32(cranked, ResourceType::DelayLineTap, this), delayLine(delayLine), delayFrames(delay) {}

Overdrive_32::Overdrive_32(Cranked &cranked) : SoundEffect_32(cranked, ResourceType::Overdrive, this) {}

CustomSoundSource_32::CustomSoundSource_32(Cranked &cranked, cref_t func, void *userdata, bool stereo)
    : SoundSource_32(cranked, ResourceType::CustomSource, this), func(func), userdata(userdata), stereo(stereo) {}

CustomSoundEffect_32::CustomSoundEffect_32(Cranked &cranked, cref_t func, void *userdata)
    : SoundEffect_32(cranked, ResourceType::CustomEffect, this), func(func), userdata(userdata) {}

CustomSoundSignal_32::CustomSoundSignal_32(Cranked &cranked, cref_t stepFunc, cref_t noteOnFunc, cref_t noteOffFunc, cref_t deallocFunc, void *userdata)
    : PDSynthSignal_32(cranked, ResourceType::CustomSource, this), stepFunc(stepFunc), noteOnFunc(noteOnFunc), noteOffFunc(noteOffFunc), deallocFunc(deallocFunc), userdata(userdata) {}

PDSynthLFO_32::PDSynthLFO_32(Cranked &cranked, LFOType type) : PDSynthSignal_32(cranked, ResourceType::LFO, this), type(type) {}

PDSynthEnvelope_32::PDSynthEnvelope_32(Cranked &cranked, float attack, float decay, float sustain, float release)
        : PDSynthSignal_32(cranked, ResourceType::Envelope, this), attack(attack), decay(decay), sustain(sustain), release(release) {}

PDSynth_32::PDSynth_32(Cranked &cranked) : SoundSource_32(cranked, ResourceType::Synth, this), envelope(cranked.heap.construct<PDSynthEnvelope_32>(cranked)) {}

ControlSignal_32::ControlSignal_32(Cranked &cranked) : PDSynthSignal_32(cranked, ResourceType::ControlSignal, this) {}

PDSynthInstrument_32::PDSynthInstrument_32(Cranked &cranked) : SoundSource_32(cranked, ResourceType::Instrument, this) {}

Audio::Audio(Cranked &cranked) : cranked(cranked), heap(cranked.heap) {}

AudioSample Audio::loadSample(const string &path) {
    auto audio = cranked.rom->getAudio(path);
    auto sample = heap.construct<AudioSample_32>(cranked, audio.data.size());
    memcpy(sample->data.data(), audio.data.data(), audio.data.size());
    sample->soundFormat = audio.soundFormat;
    sample->sampleRate = audio.sampleRate;
    return sample;
}

void Audio::sampleAudio(int16 *samples, int len) {
    // Todo: TEMP
    // static int frames;
    // for (int i = 0; i < len; i++) {
    //     auto val = (int16)(sin(frames++ / 44100.0 * 2 * numbers::pi * 500) * 4000);
    //     samples[i * 2] = val;
    //     samples[i * 2 + 1] = val;
    // }
}

void Audio::reset() {
    sampleTime = 0;
    lastError = 0;
    mainChannel.reset();
    for (SoundChannelRef &active : vector(channels.begin(), channels.end()))
        active.reset();
    channels.clear();
}

void Audio::init() {
    mainChannel = cranked.heap.construct<SoundChannel_32>(cranked);
}
