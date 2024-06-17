#include "Audio.hpp"
#include "Cranked.hpp"

using namespace cranked;

SoundSource_32::SoundSource_32(Cranked &cranked) : NativeResource(cranked) {}

SoundSource_32::~SoundSource_32() {
    // Todo: Remove from channel
}

AudioSample_32::AudioSample_32(Cranked &cranked, int size) : NativeResource(cranked), data(vheap_vector<uint8_t>(size, cranked.heap.allocator<uint8_t>())) {}

SoundEffect_32::SoundEffect_32(Cranked &cranked) : NativeResource(cranked) {}

SoundEffect_32::~SoundEffect_32() {
// Todo: Remove from channel
}

SoundChannel_32::SoundChannel_32(Cranked &cranked) : NativeResource(cranked) {}

SoundChannel_32::~SoundChannel_32() {
    // Todo: Remove from system
}

SoundChannel_32::Signal::Signal(SoundChannel_32 *channel, bool wet) : PDSynthSignalValue_32(channel->cranked), channel(channel), wet(wet) {}

AudioPlayerBase::AudioPlayerBase(Cranked &cranked) : SoundSource_32(cranked) {}

FilePlayer_32::FilePlayer_32(Cranked &cranked) : AudioPlayerBase(cranked) {}

SamplePlayer_32::SamplePlayer_32(Cranked &cranked) : AudioPlayerBase(cranked) {}

PDSynthSignalValue_32::PDSynthSignalValue_32(Cranked &cranked) : NativeResource(cranked) {}

PDSynthSignalValue_32::~PDSynthSignalValue_32() {
    // Todo: Remove from synth
}

PDSynthSignal_32::PDSynthSignal_32(Cranked &cranked) : PDSynthSignalValue_32(cranked) {}

SequenceTrack_32::SequenceTrack_32(Cranked &cranked) : SoundSource_32(cranked) {}

SoundSequence_32::SoundSequence_32(Cranked &cranked) : SoundSource_32(cranked) {}

TwoPoleFilter_32::TwoPoleFilter_32(Cranked &cranked) : SoundEffect_32(cranked) {}

OnePoleFilter_32::OnePoleFilter_32(Cranked &cranked) : SoundEffect_32(cranked) {}

BitCrusher_32::BitCrusher_32(Cranked &cranked) : SoundEffect_32(cranked) {}

RingModulator_32::RingModulator_32(Cranked &cranked) : SoundEffect_32(cranked) {}

DelayLine_32::DelayLine_32(Cranked &cranked, int length, bool stereo) : SoundEffect_32(cranked), data((stereo ? 2 : 1) * length), stereo(stereo) {}

DelayLineTap_32::DelayLineTap_32(Cranked &cranked, DelayLine_32 *delayLine, int delay) : SoundSource_32(cranked), delayLine(delayLine), delayFrames(delay) {}

Overdrive_32::Overdrive_32(Cranked &cranked) : SoundEffect_32(cranked) {}

PDSynthLFO_32::PDSynthLFO_32(Cranked &cranked, LFOType type) : PDSynthSignal_32(cranked), type(type) {}

PDSynthEnvelope_32::PDSynthEnvelope_32(Cranked &cranked, float attack, float decay, float sustain, float release)
        : PDSynthSignal_32(cranked), attack(attack), decay(decay), sustain(sustain), release(release) {}

PDSynth_32::PDSynth_32(Cranked &cranked) : SoundSource_32(cranked), envelope(cranked) {}

ControlSignal_32::ControlSignal_32(Cranked &cranked) : PDSynthSignal_32(cranked) {}

PDSynthInstrument_32::PDSynthInstrument_32(Cranked &cranked) : SoundSource_32(cranked) {}

Audio::Audio(Cranked &cranked) : cranked(cranked), heap(cranked.heap) {}

void Audio::sampleAudio(int16_t *samples, int len) {
    // Todo
}

void Audio::reset() {
    sampleTime = 0;
    lastError = 0;
    mainChannel.reset();
    for (auto &active : activeChannels)
        active.reset();
    activeChannels.clear();
}

void Audio::init() {
    mainChannel = cranked.heap.construct<SoundChannel_32>(cranked);
}
