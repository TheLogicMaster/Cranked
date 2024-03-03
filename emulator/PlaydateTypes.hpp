#pragma once

#include "Constants.hpp"

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>

typedef uint32_t cref_t;

enum class ArgType {
    uint8_t,
    int8_t,
    uint16_t,
    int16_t,
    uint32_t,
    int32_t,
    uint64_t,
    int64_t,
    float_t,
    double_t,
    void_t,
    ptr_t, // Translation from virtual to native addresses is done
    varargs_t, // Requires format string parameter proceeding it
    // Structs of N 32-bit integers (For parameters, it's passed in consecutive registers then overflows into stack, extra param for pointer as return value)
    // Smaller types work fine so long as they are padded to be 32-bit (Two consecutive char types would be an issue)
    struct2_t,
    struct4_t,
};

struct NativeFunctionMetadata {
    const char *name;
    void *func;
    const std::vector<ArgType> argTypes;
    const ArgType returnType;
};

enum class PDSystemEvent : int32_t {
    Init,
    InitLua,
    Lock,
    Unlock,
    Pause,
    Resume,
    Terminate,
    KeyPressed,
    KeyReleased,
    LowPower,
};

enum class PDButtons : int32_t {
    Left = 1 << 0,
    Right = 1 << 1,
    Up = 1 << 2,
    Down = 1 << 3,
    B = 1 << 4,
    A = 1 << 5,
    Lock = 1 << 6, // Internal-only from here
    Menu = 1 << 7,
};

enum class PDLanguage : int32_t {
    English,
    Japanese,
    Unknown,
};

enum class PDPeripherals : int32_t {
    None,
    Accelerometer,
    All = 0xFFFF,
};

enum class JsonValueType : int32_t {
    Null,
    True,
    False,
    Integer,
    Float,
    String,
    Array,
    Table,
};

enum class FileOptions : int32_t {
    Read = 1 << 0, // Only read ROM
    ReadData = 1 << 1, // Only read data
    Write = 1 << 2,
    Append = 1 << 3,
    ReadDataFallback = Read | ReadData, // Read data then fallback to ROM
};

enum class DitherType : int32_t {
    None,
    DiagonalLine,
    VerticalLine,
    HorizontalLine,
    Screen,
    Bayer2x2,
    Bayer4x4,
    Bayer8x8,
    FloydSteinberg,
    Burkes,
    Atkinson,
};

enum class LCDBitmapDrawMode : int32_t {
    Copy,
    WhiteTransparent,
    BlackTransparent,
    FillWhite,
    FillBlack,
    XOR,
    NXOR,
    Inverted,
};

enum class LCDBitmapFlip : int32_t {
    Unflipped,
    FlippedX,
    FlippedY,
    FlippedXY,
};

enum class LCDSolidColor : int32_t {
    Black,
    White,
    Clear,
    XOR,
};

enum class LCDLineCapStyle : int32_t {
    Butt,
    Square,
    Round,
};

enum class LCDFontLanguage : int32_t {
    English,
    Japanese,
    Unknown,
};

enum class PDStringEncoding : int32_t {
    ASCII,
    UFT8,
    LE16Bit,
};

enum class LCDPolygonFillRule : int32_t {
    NonZero,
    EvenOdd,
};

enum class StrokeLocation : int32_t {
    Centered,
    Outside,
    Inside,
};

enum class SoundFormat : int32_t {
    Mono8bit,
    Stereo8bit,
    Mono16bit,
    Stereo16bit,
    MonoADPCM,
    StereoADPCM,
};

enum class LFOType : int32_t {
    Square,
    Triangle,
    Sine,
    SampleAndHold,
    SawtoothUp,
    SawtoothDown,
    Arpeggiator,
    Function,
};

enum class SoundWaveform : int32_t {
    Square,
    Triangle,
    Sine,
    Noise,
    Sawtooth,
    POPhase,
    PODigital,
    POVosim,
};

enum class TwoPoleFilterType : int32_t {
    LowPass,
    HighPass,
    BandPass,
    Notch,
    PEQ,
    LowShelf,
    HighShelf,
};

struct LCDVideoPlayer_32 {

};

struct LCDBitmap_32;

struct LCDBitmapTable_32;

struct LCDFont_32;

struct LCDFontPage_32;

struct LCDFontData_32 {}; // Presumably this is just a plain data buffer

struct LCDFontGlyph_32;

struct PDMenuItem_32;

struct SDFile_32;

struct LuaUDObject_32 {
    // Todo: Possibly void* equivalent, maybe holds either a stack index or a global table key
};

struct LCDSprite_32;

struct SoundSource_32 {

};

struct FilePlayer_32 {

};

struct AudioSample_32 {

};

struct SamplePlayer_32 {

};

struct PDSynthSignal_32 {

};

struct PDSynthLFO_32 {

};

struct PDSynthEnvelope_32 {

};

struct PDSynth_32 {

};

struct ControlSignal_32 {

};

struct PDSynthInstrument_32 {

};

struct SequenceTrack_32 {

};

struct SoundSequence_32 {

};

struct TwoPoleFilter_32 {

};

struct PDSynthSignalValue_32 {

};

struct OnePoleFilter_32 {

};

struct BitCrusher_32 {

};

struct RingModulator_32 {

};

struct DelayLine_32 {

};

struct DelayLineTap_32 {

};

struct Overdrive_32 {

};

struct SoundEffect_32 {

};

struct SoundChannel_32 {

};
