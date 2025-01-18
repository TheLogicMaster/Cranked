#include "Rom.hpp"
#include "Cranked.hpp"
#include "gen/AshevilleSans14Light.hpp"
#include "gen/AshevilleSans14Bold.hpp"
#include "gen/AshevilleSans14LightOblique.hpp"
#include "gen/AshevilleSans24Light.hpp"

using namespace cranked;

unordered_map<string, Rom::File> Rom::systemFiles {
    makeSystemFilePair("Fonts/Asheville-Sans-14-Light.pft", FileType::PFT, ASHEVILLE_SANS_14_LIGHT, ASHEVILLE_SANS_14_LIGHT_SIZE),
    makeSystemFilePair("Fonts/Asheville-Sans-14-Bold.pft", FileType::PFT, ASHEVILLE_SANS_14_BOLD, ASHEVILLE_SANS_14_BOLD_SIZE),
    makeSystemFilePair("Fonts/Asheville-Sans-14-Light-Oblique.pft", FileType::PFT, ASHEVILLE_SANS_14_LIGHT_OBLIQUE, ASHEVILLE_SANS_14_LIGHT_OBLIQUE_SIZE),
    makeSystemFilePair("Fonts/Asheville-Sans-24-Light.pft", FileType::PFT, ASHEVILLE_SANS_24_LIGHT, ASHEVILLE_SANS_24_LIGHT_SIZE),
};

Rom::Rom(const string &path, Cranked *cranked) : cranked(cranked), path(path) {
    if (!fs::is_directory(path)) {
        zip = make_unique<libzippp::ZipArchive>(path);
        zip->open(libzippp::ZipArchive::ReadOnly);
    }
    try {
        loadManifest();
    } catch (exception &) {}
    sdkVersion = getPdxVersion();
    if (!sdkVersion.isValid())
        logMessage(LogLevel::Warning, "ROM SDK version missing, using latest");
}

Rom::~Rom() {
    if (zip)
        zip->close();
}

void Rom::loadManifest() {
    auto data = readRomFile("pdxinfo");
    istringstream input((const char *) data.data());
    string line;
    while (getline(input, line)) {
        auto index = line.find('=');
        if (index == string::npos)
            continue;
        manifest[line.substr(0, index)] = line.substr(index + 1);
    }
}

void Rom::load() {
    if (loaded)
        return;

    if (zip)
        zip->open();

    vector<uint8> pdzData;
    try {
        pdzData = readRomFile("main.pdz");  // Todo: Don't use exceptions if not needed here
    } catch (exception &) {}
    if (!pdzData.empty())
        pdzFiles = loadPDZ(pdzData);

    for (auto &file : pdzFiles)
        if (file.type == FileType::LUAC) {
            hasLua = true;
            break;
        }

    vector<uint8> pdexData;
    try {
        pdexData = readRomFile("pdex.bin");
    } catch (exception &) {}
    if (!pdexData.empty()) {
        // Format is 16 byte magic, 16 byte MD5, decompressed size, Code and data size, relative address of shim function, relocation count
        // Last N 4-byte ints of compressed data are relocation offsets
        const int HEADER_SIZE = 0x30; // Magic + 16 byte MD5 + 4 32-bit integers
        if (strcmp((char *) pdexData.data(), PDX_MAGIC) != 0)
            throw CrankedError("Invalid PDX magic");
        auto decompressedSize = *(uint32 *) &pdexData[0x20];
        combinedProgramSize = *(uint32 *) &pdexData[0x24];
        auto shimOffset = *(uint32 *) &pdexData[0x28];
        auto relocationCount = *(uint32 *) &pdexData[0x2C];

        // Todo: Detect uncompressed binary gracefully (Test with older version of SDK)
        try {
            binary = decompressData(pdexData.data() + HEADER_SIZE, pdexData.size() - HEADER_SIZE, decompressedSize);
        } catch (exception &ex) {
            binary = pdexData;
        }

        // Relocations
        for (int i = 0; i < relocationCount; i++) {
            auto offset = *(uint32 *) (binary.data() + binary.size() - 4 * (i + 1));
            *(uint32 *) &binary[offset] += CODE_ADDRESS;
        }
        binary.resize(binary.size() - 4 * relocationCount);
        binarySize = binary.size();

        eventHandlerAddress = CODE_ADDRESS + shimOffset;
    }

    if (zip) {
        for (auto &entry : zip->getEntries()) { // Preload zip entry data, to avoid having to abstract into common file-type API
            if (!entry.isFile()) continue;
            vector<uint8> data(entry.getSize());
            auto zipData = entry.readAsBinary();
            memcpy(data.data(), zipData, data.size());
            delete[] (char *) zipData;
            outerFiles.emplace_back(File{
                    .name = entry.getName(),
                    .type = getFileType(data.data()),
                    .isDir = entry.isDirectory(),
                    .size = (uint32) entry.getSize(),
                    .extra = 0,
                    .data = data,
                    .modTime = entry.getDate(),
            });
        }
    } else {
        for (auto &entry : fs::recursive_directory_iterator(path)) {
            auto type = FileType::UNKNOWN;
            if (!entry.is_directory())
                try {
                    ifstream input(path / entry.path(), ios::binary);
                    char buffer[13]{}; // Null terminator is not strictly needed, but doesn't hurt
                    input.read(buffer, 12);
                    type = getFileType((uint8 *) buffer);
                } catch (exception &) {}
            outerFiles.emplace_back(File{
                    .name = entry.path().lexically_relative(path),
                    .type = type,
                    .isDir = entry.is_directory(),
                    .size = entry.is_directory() ? 0 : (uint32) entry.file_size(),
                    .extra = 0,
                    .data = {},
                    .modTime = to_time_t(entry.last_write_time()),
            });
        }
    }

    loaded = true;
}

void Rom::unload() {
    if (zip)
        zip->close();
    pdzFiles.clear();
    outerFiles.clear();
    binary.clear();
    loaded = false;
}

vector<uint8> Rom::readRomFile(string name, const string &extension, const vector<string> &replacedExtensions) {
    for (auto &replaced : replacedExtensions) {
        if (name.ends_with(replaced)) {
            name = name.substr(0, name.size() - replaced.size());
            break;
        }
    }
    if (!extension.empty() and not name.ends_with(extension))
        name += extension;
    if (name.starts_with("/System/")) {
        auto file = findSystemFile(name.substr(8));
        if (!file)
            throw CrankedError("No such file: " + name);
        return file->data;
    }
    for (auto &file : pdzFiles)
        if (file.name == name)
            return file.data;
    vector<uint8> data;
    if (zip) {
        auto entry = zip->getEntry(name);
        if (entry.isNull())
            throw CrankedError("No such file: " + name);
        data.resize(entry.getSize());
        auto zipData = entry.readAsBinary();
        memcpy(data.data(), zipData, data.size());
        delete[] (char *) zipData;
    } else
        data = readFileData(fs::path(path) / name);
    return data;
}

Rom::File *Rom::findRomFile(const string &name) {
    if (name.starts_with("/System/"))
        return findSystemFile(name.substr(8));
    auto normalize = [](const fs::path &p){
        return p; // p.lexically_normal(); // Todo: This surely doesn't work on Windows
    };
    for (auto &file : pdzFiles) // Todo: Should PDZ contents be included?
        if (normalize(file.name) == normalize(name))
            return &file;
    for (auto &file : outerFiles)
        if (normalize(file.name) == normalize(name))
            return &file;
    return nullptr;
}

vector<string> Rom::listRomFiles(string base, bool recursive) {
    vector<string> files;
    if (base == "/" or base == ".") // Todo: Normalize paths better with fs::lexically_normal
        base = "";
    vector<string> collectedDirs;
    for (auto &file : outerFiles) {
        if (!file.name.starts_with(base))
            continue;
        bool nested = false;
        if (!recursive)
            for (auto &dir : collectedDirs)
                if (file.name.starts_with(dir)) { // Todo: Improve with std::filesystem
                    nested = true;
                    break;
                }
        if (nested)
            continue;
        files.emplace_back(file.name + (file.isDir and !file.name.ends_with("/") ? "/" : ""));
        if (file.isDir)
            collectedDirs.emplace_back(file.name);
    }
    return files;
}

vector<Rom::File> Rom::loadPDZ(const vector<uint8> &data) {
    vector<File> files;
    if (data.size() <= strlen(PDZ_MAGIC) or strcmp(PDZ_MAGIC, (char *) data.data()) != 0)
        throw CrankedError("Invalid PDZ magic");
    auto flags = readUint32LE(&data[12]);
    if (flags & 0x40000000)
        throw CrankedError("Encrypted PDZ");
    size_t index = 0x10;

    while (index < data.size()) {
        auto entryHeader = readUint32LE(&data[index]);
        index += 4;
        auto entryFlags = entryHeader & 0xFF;
        auto entryLength = (entryHeader >> 8) & 0xFFFFFF;
        bool isCompressed = entryFlags & 0x80;
        auto fileType = entryFlags & 0xF;
        auto fileName = string((const char *) data.data() + index);
        index += (int) fileName.length() + 1;
        index = (index + 0x3) & ~0x3; // Align to 4 bytes
        uint32 extra = 0;
        if (FileType(fileType) == FileType::PDA) {
            extra = readUint32LE(&data[index]);
            index += 4;
            entryLength -= 4;
        }

        vector<uint8> fileContents;
        if (isCompressed) {
            auto fileSize = readUint32LE(&data[index]);
            fileContents = decompressData(data.data() + index + 4, entryLength - 4, fileSize);
            index += entryLength;
        } else {
            fileContents.resize(entryLength);
            memcpy(fileContents.data(), data.data() + index, entryLength);
            index += entryLength;
        }

        files.emplace_back(File{fileName, FileType(fileType), false, (uint32) fileContents.size(), extra, fileContents});
    }

    return files;
}

Rom::Font Rom::readFont(const uint8 *data, size_t dataSize) {
    if (strcmp(FONT_MAGIC, (char *) data) != 0)
        throw CrankedError("Bad font magic");
    auto flags = readUint32LE(&data[12]);
    bool compressed = flags & 0x80000000;
    bool wide = flags & 0x00000001; // Contains characters above U+1FFFF // Todo: Is this needed?
    const uint8 *fontData;
    vector<uint8> decompressedData;
    if (compressed) {
        auto size = readUint32LE(&data[0x10]);
//        auto maxGlyphWidth = (int) readUint32LE(&data[0x14]); // Not needed
//        auto maxGlyphHeight = (int) readUint32LE(&data[0x18]); // Not needed
//        auto unknown = (int) readUint32LE(&data[0x1C]); // Unknown/padding
        decompressedData = decompressData(&data[0x20], dataSize - 0x20, size);
        fontData = decompressedData.data();
    } else
        fontData = data + 0x10;

    return readFontData(fontData, wide);
}

Rom::Font Rom::readFontData(const uint8 *data, bool wide) {
    // Read page list header
    Font font{};
    font.glyphWidth = data[0];
    font.glyphHeight = data[1];
    font.tracking = readUint16LE(data + 2);
    vector<uint8> pageUsageFlags(64);
    memcpy(pageUsageFlags.data(), data + 4, 64);
    int pageCount = 0;
    for (int i = 0; i < 64; i++)
        pageCount += popcount(pageUsageFlags[i]);
    vector<int> pageIndices(pageCount);
    int pageIndex = 0;
    for (int i = 0; i < 64; i++)
        for (int j = 0; j < 8; j++) {
            bool hasPage = pageUsageFlags[i] & (1 << j);
            if (hasPage)
                pageIndices[pageIndex++] = i * 64 + j;
        }
    vector<uint32> pageOffsets(pageCount + 1); // First offset is zero, ignore last offset which points to end
    for (int i = 0; i < pageCount; i++)
        pageOffsets[i + 1] = readUint32LE(data + 68 + i * 4);
    auto pageListHeaderEnd = data + 68 + pageCount * 4;

    // Read pages
    for (int page = 0; page < pageCount; page++) {
        const uint8 *pageData = pageListHeaderEnd + pageOffsets[page];

        // Read page header
        pageData += 3; // First 3 bytes are padding
        int glyphCount = *(pageData++);
        vector<uint8> glyphUsageFlags(32);
        memcpy(glyphUsageFlags.data(), pageData, 32);
        pageData += 32;
        vector<uint16> glyphOffsets(glyphCount + 1); // First offset is zero, ignore last offset which points to end
        for (int i = 0; i < glyphCount; i++) {
            glyphOffsets[i + 1] = readUint16LE(pageData);
            pageData += 2;
        }

        // Alignment padding
        if ((glyphCount % 2) == 1)
            pageData += 2;

        auto pageHeaderEnd = pageData;

        // Read page glyphs
        auto &pageGlyphs = font.glyphs[pageIndices[page]];
        int glyphIndex = 0;
        for (int i = 0; i < 256; i++) {
            auto word = glyphUsageFlags[i / 8];
            bool present = word & (1 << (i % 8));
            if (!present)
                continue;
            auto &glyph = pageGlyphs[i];
            auto glyphData = pageHeaderEnd + glyphOffsets[glyphIndex++];

            // Read glyph header
            glyph.advance = *(glyphData++);
            int shortKerningEntries = *(glyphData++);
            int longKerningEntries = readUint16LE(glyphData);
            glyphData += 2;

            // Read short kerning table
            for (int j = 0; j < shortKerningEntries; j++) {
                glyph.shortKerningTable[glyphData[0]] = *(int8 *) &glyphData[1];
                glyphData += 2;
            }

            // Alignment padding
            if ((shortKerningEntries % 2) == 1)
                glyphData += 2;

            // Read long kerning table
            for (int j = 0; j < longKerningEntries; j++) {
                auto entry = readUint32LE(glyphData);
                glyph.longKerningTable[int(entry >> 8)] = *(int8 *) &entry;
                glyphData += 4;
            }

            glyph.cell = readImageCell(glyphData);
        }
    }

    return font;
}


Rom::StringTable Rom::readStringTable(const uint8 *data, size_t dataSize) {
    if (strcmp(STRING_TABLE_MAGIC, (char *) data) != 0)
        throw CrankedError("Bad string table magic");
    auto flags = readUint32LE(&data[12]);
    bool compressed = flags & 0x80000000;
    const uint8 *stringData;
    vector<uint8> decompressedData;
    if (compressed) {
        auto size = readUint32LE(&data[0x10]);
        // Next 12 bytes are unused
        decompressedData = decompressData(&data[0x20], dataSize - 0x20, size);
        stringData = decompressedData.data();
    } else
        stringData = data + 0x10;
    auto stringCount = readUint32LE(stringData);
    stringData += 4;
    vector<uint32> offsets(stringCount);
    for (int i = 0; i < stringCount - 1; i++) {
        offsets[i + 1] = readUint32LE(stringData);
        stringData += 4;
    }
    stringData += 4;
    StringTable table;
    for (int i = 0; i < stringCount; i++) {
        auto start = stringData + offsets[i];
        auto keyLength = strlen((const char *) start);
        table.strings[(const char *) start] = (const char *) start + keyLength + 1;
    }
    return table;
}

Rom::Audio Rom::readAudio(const uint8 *data, size_t dataSize) {
    if (strcmp(AUDIO_MAGIC, (char *) data) != 0)
        throw CrankedError("Bad audio magic");
    auto headerWord = readUint32LE(data + 12);
    auto sampleRate = headerWord >> 8;
    auto soundFormat = SoundFormat(headerWord & 0xFF);
    bool stereo = soundFormat == SoundFormat::Stereo8bit or soundFormat == SoundFormat::Stereo16bit or soundFormat == SoundFormat::StereoADPCM;
    auto audioDataSize = dataSize - 16;
    Audio audio {.soundFormat = soundFormat, .sampleRate = sampleRate};
    audio.data.resize(audioDataSize);
    memcpy(audio.data.data(), data + 16, audioDataSize);
    if (soundFormat == SoundFormat::Mono8bit or soundFormat == SoundFormat::Stereo8bit) {
        auto samples = audioDataSize;
        audio.samples.resize(samples);
        for (int i = 0; i < samples; i++)
            audio.samples[i] = (int16) (((int) bit_cast<int8>(data[16 + i]) - 0x80) << 8); // Todo: Verify conversion
    } else if (soundFormat == SoundFormat::Mono16bit or soundFormat == SoundFormat::Stereo16bit) {
        auto samples = audioDataSize / 2;
        audio.samples.resize(samples);
        for (int i = 0; i < samples; i++)
            audio.samples[i] = bit_cast<int16>(readUint16LE(data + 16 + i * 2));
    } else { // 4-bit IMA ADPCM
        static constexpr int IMA_INDEX_TABLE[]{
                -1, -1, -1, -1, 2, 4, 6, 8,
                -1, -1, -1, -1, 2, 4, 6, 8,
        };
        static constexpr int IMA_STEP_TABLE[]{
                7, 8, 9, 10, 11, 12, 13, 14,
                16, 17, 19, 21, 23, 25, 28, 31,
                34, 37, 41, 45, 50, 55, 60, 66,
                73, 80, 88, 97, 107, 118, 130, 143,
                157, 173, 190, 209, 230, 253, 279, 307,
                337, 371, 408, 449, 494, 544, 598, 658,
                724, 796, 876, 963, 1060, 1166, 1282, 1411,
                1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
                3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
                7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
                15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
                32767,
        };
        auto blockSize = readUint16LE(data + 16);
        auto blocks = audioDataSize / blockSize;
        auto samplesPerBlock = (blockSize - (stereo ? 8 : 4)) * 2; // 4 byte header per-channel, 4-bits per sample
        auto samples = blocks * samplesPerBlock;
        audio.samples.resize(samples);
        for (int i = 0; i < blocks; i++) {
            int predictorLeft{}, predictorRight{};
            int stepIndexLeft{}, stepIndexRight{};
            int stepLeft{}, stepRight{};
            auto blockData = data + 16 + blockSize * i;
            predictorLeft = readUint16LE(blockData);
            blockData += 2;
            stepIndexLeft = blockData[0];
            blockData += 2; // Last byte is zero
            if (stereo) {
                predictorRight = readUint16LE(blockData);
                blockData += 2;
                stepIndexRight = blockData[0];
                blockData += 2;
            }
            for (int j = 0; j < samplesPerBlock / 2; j++) {
                auto decodeSample = [](uint8 sample, int &predictor, int &stepIndex, int &step){
                    int difference = 0;
                    if (sample & 0x4)
                        difference += step;
                    if (sample & 0x2)
                        difference += step >> 1;
                    if (sample & 0x1)
                        difference += step >> 2;
                    difference += step >> 3;
                    if (sample & 0x8)
                        difference = -difference;
                    predictor += difference;
                    predictor = max(min(predictor, 32767), -32767);
                    stepIndex += IMA_INDEX_TABLE[sample];
                    stepIndex = max(min(stepIndex, 88), 0);
                    step = IMA_STEP_TABLE[stepIndex];
                    return (int16) predictor;
                };
                auto sample = *(blockData++);
                auto first = sample >> 4;
                auto second = sample & 0xF;
                audio.samples[samplesPerBlock * i + j * 2] = decodeSample(first, predictorLeft, stepIndexLeft, stepLeft);
                audio.samples[samplesPerBlock * i + j * 2 + 1] = stereo ? decodeSample(second, predictorRight, stepIndexRight, stepRight) : decodeSample(second, predictorLeft, stepIndexLeft, stepLeft);
            }
        }
    }
    return audio;
}

Rom::Image Rom::readImage(const uint8 *data, size_t dataSize) {
    if (strcmp(IMAGE_MAGIC, (char *) data) != 0)
        throw CrankedError("Bad image magic");
    auto flags = readUint32LE(&data[12]);
    bool compressed = flags & 0x80000000;
    const uint8 *imageData;
    vector<uint8> decompressedData;
    Image image{};
    if (compressed) {
        auto size = readUint32LE(&data[0x10]);
//        image.width = (int) readUint32LE(&data[0x14]); // Redundant
//        image.height = (int) readUint32LE(&data[0x18]); // Redundant
//        image.extra = readUint32LE(&data[0x1C]); // Unused
        decompressedData = decompressData(&data[0x20], dataSize - 0x20, size);
        imageData = decompressedData.data();
    } else
        imageData = data + 0x10;
    image.cell = readImageCell(imageData);
    image.width = image.cell.width;
    image.height = image.cell.height;
    return image;
}

Rom::ImageTable Rom::readImageTable(const uint8 *data, size_t dataSize) {
    if (strcmp(IMAGE_TABLE_MAGIC, (char *) data) != 0)
        throw CrankedError("Bad image table magic");
    auto flags = readUint32LE(&data[12]);
    bool compressed = flags & 0x80000000;
    const uint8 *tableData;
    vector<uint8> decompressedData;
    if (compressed) {
        auto size = readUint32LE(&data[0x10]);
        // auto firstImageWidth = (int) readUint32LE(&data[0x14]); // Redundant
        // auto firstImageHeight = (int) readUint32LE(&data[0x18]); // Redundant
        // auto cellCount = readUint32LE(&data[0x1C]); // Redundant
        decompressedData = decompressData(&data[0x20], dataSize - 0x20, size);
        tableData = decompressedData.data();
    } else
        tableData = data + 0x10;
    auto cellCount = readUint16LE(tableData);
    auto cellsPerRow = readUint16LE(tableData + 2);
    tableData += 4;
    vector<uint32> offsets(cellCount);
    for (int i = 0; i < cellCount - 1; i++) {
        offsets[i + 1] = readUint32LE(tableData);
        tableData += 4;
    }
    tableData += 4;
    ImageTable table;
    table.cellsPerRow = cellsPerRow;
    for (int i = 0; i < cellCount; i++)
        table.cells.emplace_back(readImageCell(tableData + offsets[i]));
    return table;
}

Rom::Video Rom::readVideo(const uint8 *data, size_t dataSize) {
    if (strcmp(VIDEO_MAGIC, (char *) data) != 0)
        throw CrankedError("Bad video magic");
    data += 16; // Previous 32-bit word is reserved
    auto frameCount = readUint16LE(data);
    data += 4; // Previous 16-bit word is reserved
    auto framerate = bit_cast<float>(readUint32LE(data));
    auto width = readUint16LE(data + 4);
    auto height = readUint16LE(data + 6);
    data += 8;
    enum class FrameType { Empty, I, P, Combined };
    struct FrameHeader {
        uint32 offset;
        FrameType type;
    };
    vector<FrameHeader> headers(frameCount);
    for (int i = 0; i < frameCount; i++) {
        auto value = readUint32LE(data);
        headers[i].offset = value >> 2;
        headers[i].type = FrameType(value & 0x3);
        data += 4;
    }
    Video video {.framerate = framerate, .width = width, .height = height};
    for (int i = 0; i < frameCount; i++) {
        auto frameData = data + headers[i].offset;
        auto type = headers[i].type;
        auto readFrame = [&](const uint8 *data){
            vector<uint8> frame(width * height);
            auto stride = (int) ceil(width / 8.0);
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++)
                    frame[y * width + x] = (uint8) (bool) (data[y * stride + x / 8] & (1 << (7 - x % 8)));
            return frame;
        };
        if (type == FrameType::I) {
            video.frames[i] = readFrame(frameData);
        } else if (type == FrameType::P) {
            auto &previous = video.frames[i - 1];
            auto frame = readFrame(frameData);
            for (int j = 0; j < frame.size(); i++)
                frame[j] ^= previous[j];
            video.frames[i] = frame;
        } else if (type == FrameType::Combined) {
            auto iFrameLength = readUint16LE(frameData);
            auto iFrame = readFrame(frameData + 2);
            auto pFrame = readFrame(frameData + 2 + iFrameLength);
            for (int j = 0; j < iFrame.size(); i++)
                iFrame[j] ^= pFrame[j];
            video.frames[i] = iFrame;
        }
    }
    return video;
}

Rom::ImageCell Rom::readImageCell(const uint8 *data) {
    int clipWidth = readUint16LE(data + 0);
    int clipHeight = readUint16LE(data + 2);
    int srcStride = readUint16LE(data + 4);
    int clipLeft = readUint16LE(data + 6);
    int clipRight = readUint16LE(data + 8);
    int clipTop = readUint16LE(data + 10);
    int clipBottom = readUint16LE(data + 12);
    int flags = readUint16LE(data + 14);
    bool transparency = flags & 0x3;
    ImageCell cell;
    cell.width = clipLeft + clipWidth + clipRight;
    cell.height = clipTop + clipHeight + clipBottom;
    cell.data.resize(cell.width * cell.height);
    cell.mask.resize(cell.data.size());
    auto read = [&](int start, uint8 *out){
        for (int y = 0; y < clipHeight; y++)
            for (int x = 0; x < clipWidth; x++)
                out[(clipTop + y) * cell.width + clipLeft + x] = (bool) (data[start + y * srcStride + x / 8] & (1 << (7 - x % 8)));
    };
    read(16, cell.data.data());
    if (transparency)
        read(16 + srcStride * clipHeight, cell.mask.data());
    else {
        for (int y = 0; y < clipHeight; y++)
            for (int x = 0; x < clipWidth; x++)
                cell.mask[(clipTop + y) * cell.width + clipLeft + x] = 1;
    }
    return cell;
}

Rom::FileType Rom::getFileType(const uint8 *header) {
    if (!strcmp((const char *) header, LUA_MAGIC))
        return FileType::LUAC;
    string magic(header, header + 12);
    if (magic == IMAGE_MAGIC)
        return FileType::PDI;
    if (magic == IMAGE_TABLE_MAGIC)
        return FileType::PDT;
    if (magic == VIDEO_MAGIC)
        return FileType::PDV;
    if (magic == AUDIO_MAGIC)
        return FileType::PDA;
    if (magic == STRING_TABLE_MAGIC)
        return FileType::PDS;
    if (magic == FONT_MAGIC)
        return FileType::PFT;
    return FileType::UNKNOWN;
}

vector<uint8> Rom::writeImage(Bitmap image) {
    vector<uint8> buffer;
    bufferAddStr(buffer, IMAGE_MAGIC);
    bufferAddVal<uint32>(buffer, 0); // Flags
    bufferAddVal<uint16>(buffer, image->width); // Clip width
    bufferAddVal<uint16>(buffer, image->height); // Clip height
    bufferAddVal<uint16>(buffer, (int) ceil((float) image->width / 8)); // Stride
    bufferAddVal<uint16>(buffer, 0); // Clip left
    bufferAddVal<uint16>(buffer, 0); // Clip right
    bufferAddVal<uint16>(buffer, 0); // Clip top
    bufferAddVal<uint16>(buffer, 0); // Clip bottom
    bufferAddVal<uint16>(buffer, (bool)image->mask * 0b1); // Cell flags
    buffer.insert(buffer.end(), image->data.begin(), image->data.end());
    if (image->mask)
        buffer.insert(buffer.end(), image->mask->data.begin(), image->mask->data.end());
    return buffer;
}

Rom::File *Rom::findSystemFile(const string &path) {
    // Todo: Path should be normalized a bit
    if (auto it = systemFiles.find(path); it != systemFiles.end())
        return &it->second;
    throw CrankedError("No such system file: " + path);
}

void Rom::logMessage(LogLevel level, const char *fmt, ...) const {
    if (!cranked)
        return;
    va_list args;
    va_start(args, fmt);
    cranked->logMessageVA(level, fmt, args);
    va_end(args);
}
