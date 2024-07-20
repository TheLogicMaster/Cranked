#pragma once

#include "NativeResource.hpp"

namespace cranked {

    typedef uint32 cref_t;
    typedef float MIDINote;

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
        va_list_t, // Requires format string parameter proceeding it
        // Structs of N 32-bit integers or floats (For parameters, it's passed in consecutive registers then overflows into stack, extra param for pointer as return value)
        // Smaller types work fine so long as they are padded to be 32-bit (Two consecutive char types would be an issue)
        struct2_t,
        struct4_t,
        struct4f_t,
    };

    struct NativeFunctionMetadata {
        const char *name;
        void *func;
        const std::vector<ArgType> argTypes;
        const ArgType returnType;
    };

    enum class PDSystemEvent : int32 {
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

    enum class PDButtons : int32 {
        Left = 1 << 0,
        Right = 1 << 1,
        Up = 1 << 2,
        Down = 1 << 3,
        B = 1 << 4,
        A = 1 << 5,
        Lock = 1 << 6, // Internal-only from here
        Menu = 1 << 7,
    };

    enum class PDLanguage : int32 {
        English,
        Japanese,
        Unknown,
    };

    enum class PDFontVariant : int32 {
        Normal,
        Bold,
        Italic,
    };

    enum class PDPeripherals : int32 {
        None,
        Accelerometer,
        All = 0xFFFF,
    };

    enum class JsonValueType : int32 {
        Null,
        True,
        False,
        Integer,
        Float,
        String,
        Array,
        Table,
    };

    enum class FileOptions : int32 {
        Read = 1 << 0, // Only read ROM
        ReadData = 1 << 1, // Only read data
        Write = 1 << 2,
        Append = 1 << 3,
        ReadDataFallback = Read | ReadData, // Read data then fallback to ROM
    };

    enum class LuaType : int32 {
        Nil,
        Bool,
        Int,
        Float,
        String,
        Table,
        Function,
        Thread,
        Object
    };

    enum class DitherType : int32 {
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

    enum class LCDBitmapDrawMode : int32 {
        Copy,
        WhiteTransparent,
        BlackTransparent,
        FillWhite,
        FillBlack,
        XOR,
        NXOR,
        Inverted,
    };

    typedef LCDBitmapDrawMode BitmapDrawMode;

    enum class GraphicsFlip : int32 {
        Unflipped,
        FlippedX,
        FlippedY,
        FlippedXY,
    };

    typedef GraphicsFlip LCDBitmapFlip;

    enum class LCDSolidColor : int32 {
        Black,
        White,
        Clear,
        XOR,
    };

    enum class LCDLineCapStyle : int32 {
        Butt,
        Square,
        Round,
    };

    enum class PDStringEncoding : int32 {
        ASCII,
        UFT8,
        LE16Bit,
    };

    enum class LCDPolygonFillRule : int32 {
        NonZero,
        EvenOdd,
    };

    enum class StrokeLocation : int32 {
        Centered,
        Outside,
        Inside,
    };

    enum class SoundFormat : int32 {
        Mono8bit,
        Stereo8bit,
        Mono16bit,
        Stereo16bit,
        MonoADPCM,
        StereoADPCM,
    };

    enum class LFOType : int32 {
        Square,
        Triangle,
        Sine,
        SampleAndHold,
        SawtoothUp,
        SawtoothDown,
        Arpeggiator,
        Function,
    };

    enum class SoundWaveform : int32 {
        Square,
        Triangle,
        Sine,
        Noise,
        Sawtooth,
        POPhase,
        PODigital,
        POVosim,
    };

    enum class TwoPoleFilterType : int32 {
        LowPass,
        HighPass,
        BandPass,
        Notch,
        PEQ,
        LowShelf,
        HighShelf,
    };

    enum class MicSource : int32 {
        Autodetect,
        Internal,
        Headset
    };

    struct PDNodeGraph_32;
    typedef PDNodeGraph_32 *NodeGraph;
    typedef ResourceRef<PDNodeGraph_32> NodeGraphRef;

    struct PDGraphNode_32;
    typedef PDGraphNode_32 *GraphNode;
    typedef ResourceRef<PDGraphNode_32> GraphNodeRef;

    struct LCDVideoPlayer_32;
    typedef LCDVideoPlayer_32 *VideoPlayer;
    typedef ResourceRef<LCDVideoPlayer_32> VideoPlayerRef;

    struct LCDBitmap_32;
    typedef LCDBitmap_32 *Bitmap;
    typedef ResourceRef<LCDBitmap_32> BitmapRef;

    struct LCDBitmapTable_32;
    typedef LCDBitmapTable_32 *BitmapTable;
    typedef ResourceRef<LCDBitmapTable_32> BitmapTableRef;

    struct LCDTileMap_32;
    typedef LCDTileMap_32 *TileMap;
    typedef ResourceRef<LCDTileMap_32> TileMapRef;

    struct LCDFont_32;
    typedef LCDFont_32 *Font;
    typedef ResourceRef<LCDFont_32> FontRef;

    struct LCDFontPage_32;
    typedef LCDFontPage_32 *FontPage;
    typedef ResourceRef<LCDFontPage_32> FontPageRef;

    struct LCDFontData_32 {}; // Presumably this is just a plain data buffer

    struct LCDFontGlyph_32;
    typedef LCDFontGlyph_32 *FontGlyph;
    typedef ResourceRef<LCDFontGlyph_32> FontGlyphRef;

    struct PDMenuItem_32;
    typedef PDMenuItem_32 *MenuItem;
    typedef ResourceRef<PDMenuItem_32> MenuItemRef;

    struct SDFile_32;
    typedef SDFile_32 *File;
    typedef ResourceRef<SDFile_32> FileRef;

    struct LuaUDObject_32; // Opaque pointer, just store stack offset int in pointer relative to HEAP_ADDRESS

    struct LCDSprite_32;
    typedef LCDSprite_32 *Sprite;
    typedef ResourceRef<LCDSprite_32> SpriteRef;

    struct SoundSource_32;
    typedef SoundSource_32 *SoundSource;
    typedef ResourceRef<SoundSource_32> SoundSourceRef;

    struct FilePlayer_32;
    typedef FilePlayer_32 *FilePlayer;
    typedef ResourceRef<FilePlayer_32> FilePlayerRef;

    struct AudioSample_32;
    typedef AudioSample_32 *AudioSample;
    typedef ResourceRef<AudioSample_32> AudioSampleRef;

    struct SamplePlayer_32;
    typedef SamplePlayer_32 *SamplePlayer;
    typedef ResourceRef<SamplePlayer_32> SamplePlayerRef;

    struct PDSynthSignal_32;
    typedef PDSynthSignal_32 *SynthSignal;
    typedef ResourceRef<PDSynthSignal_32> SynthSignalRef;

    struct PDSynthLFO_32;
    typedef PDSynthLFO_32 *SynthLFO;
    typedef ResourceRef<PDSynthLFO_32> SynthLFORef;

    struct PDSynthEnvelope_32;
    typedef PDSynthEnvelope_32 *SynthEnvelope;
    typedef ResourceRef<PDSynthEnvelope_32> SynthEnvelopeRef;

    struct PDSynth_32;
    typedef PDSynth_32 *Synth;
    typedef ResourceRef<PDSynth_32> SynthRef;

    struct ControlSignal_32;
    typedef ControlSignal_32 *ControlSignal;
    typedef ResourceRef<ControlSignal_32> ControlSignalRef;

    struct PDSynthInstrument_32;
    typedef PDSynthInstrument_32 *SynthInstrument;
    typedef ResourceRef<PDSynthInstrument_32> SynthInstrumentRef;

    struct SequenceTrack_32;
    typedef SequenceTrack_32 *SequenceTrack;
    typedef ResourceRef<SequenceTrack_32> SequenceTrackRef;

    struct SoundSequence_32;
    typedef SoundSequence_32 *SoundSequence;
    typedef ResourceRef<SoundSequence_32> SoundSequenceRef;

    struct TwoPoleFilter_32;
    typedef TwoPoleFilter_32 *TwoPoleFilter;
    typedef ResourceRef<TwoPoleFilter_32> TwoPoleFilterRef;

    struct PDSynthSignalValue_32;
    typedef PDSynthSignalValue_32 *SynthSignalValue;
    typedef ResourceRef<PDSynthSignalValue_32> SynthSignalValueRef;

    struct OnePoleFilter_32;
    typedef OnePoleFilter_32 *OnePoleFilter;
    typedef ResourceRef<OnePoleFilter_32> OnePoleFilterRef;

    struct BitCrusher_32;
    typedef BitCrusher_32 *BitCrusher;
    typedef ResourceRef<BitCrusher_32> BitCrusherRef;

    struct RingModulator_32;
    typedef RingModulator_32 *RingModulator;
    typedef ResourceRef<RingModulator_32> RingModularRef;

    struct DelayLine_32;
    typedef DelayLine_32 *DelayLine;
    typedef ResourceRef<DelayLine_32> DelayLineRef;

    struct DelayLineTap_32;
    typedef DelayLineTap_32 *DelayLineTap;
    typedef ResourceRef<DelayLineTap_32> DelayLineTapRef;

    struct Overdrive_32;
    typedef Overdrive_32 *Overdrive;
    typedef ResourceRef<Overdrive_32> OverdriveRef;

    struct SoundEffect_32;
    typedef SoundEffect_32 *SoundEffect;
    typedef ResourceRef<SoundEffect_32> SoundEffectRef;

    struct SoundChannel_32;
    typedef SoundChannel_32 *SoundChannel;
    typedef ResourceRef<SoundChannel_32> SoundChannelRef;

}
