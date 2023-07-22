#pragma once

#include "PlaydateTypes.hpp"

#include "libzippp.h"

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <filesystem>

class Rom {
public:
    enum class FileType {
        UNKNOWN = 0, // Unknown
        LUAC = 1, // Lua bytecode
        PDI = 2, // Static image
        PDT = 3, // Animation image
        PDV = 4, // Video
        PDA = 5, // Audio
        PDS = 6, // Text strings
        PFT = 7, // Font
    };

    constexpr static const char *FILE_TYPE_NAMES[] { // Todo: Verify names
            "unknown",
            "lua",
            "image",
            "animation",
            "video",
            "audio",
            "text",
            "font"
    };

    struct File {
        std::string name;
        FileType type{};
        bool isDir{};
        uint32_t size{};
        uint32_t extra{}; // Used for audio
        std::vector<uint8_t> data; // Populated for PDZ contents
        std::chrono::time_point<std::chrono::system_clock> modTime;
    };

    struct ImageCell {
        int width{};
        int height{};
        std::vector<uint8_t> data;
        std::vector<uint8_t> mask;
    };

    struct Image {
        int width{};
        int height{};
        uint32_t extra{}; // Probably unused
        ImageCell cell;
    };

    struct ImageTable {
        int cellsPerRow{};
        std::vector<ImageCell> cells;
    };

    struct StringTable {
        std::map<std::string, std::string> strings;
    };

    struct FontGlyph {
        int advance{};
        std::map<int, int8_t> shortKerningTable; // Page 0 entries
        std::map<int, int8_t> longKerningTable;
        ImageCell cell;
    };

    struct Font {
        int tracking{};
        int glyphWidth{};
        int glyphHeight{};
        std::unordered_map<int, std::unordered_map<int, FontGlyph>> glyphs; // Bottom 8 bits of codepoint is page number, upper 8? bits is page index
    };

    struct Audio {
        SoundFormat format;
        bool stereo;
        uint32_t sampleRate;
        std::vector<int16_t> samples; // Not sure if this is even useful, since it needs to support playing raw data, anyway
        std::vector<uint8_t> data;
    };

    struct Video {
        float framerate;
        int width;
        int height;
        std::vector<std::vector<uint8_t>> frames;
    };

    explicit Rom(const std::string &path);
    ~Rom();
    void load();
    void unload();
    std::vector<uint8_t> readRomFile(const std::string &name, const std::string &extension = "");
    File *findRomFile(const std::string &name);
    std::vector<std::string> listRomFiles(std::string base, bool recursive); // Doesn't include PDZ contents

    inline std::string getName() {
        return manifest["name"];
    }

    inline std::string getAuthor() {
        return manifest["author"];
    }

    inline std::string getDescription() {
        return manifest["description"];
    }

    inline std::string getBundleID() {
        return manifest["bundleID"];
    }

    inline std::string getVersion() {
        return manifest["version"];
    }

    inline int getBuildNumber() {
        try {
            return std::stoi(manifest["buildNumber"]);
        } catch (std::exception &ex) {
            return -1;
        }
    }

    inline std::string getImagePath() {
        return manifest["imagePath"];
    }

    inline std::string getLaunchSoundPath() {
        return manifest["launchSoundPath"];
    }

    inline std::string getContentWarning() {
        return manifest["contentWarning"];
    }

    inline std::string getContentWarning2() {
        return manifest["contentWarning2"];
    }

    inline int getPdxVersion() {
        try {
            return std::stoi(manifest["pdxversion"]);
        } catch (std::exception &ex) {
            return -1;
        }
    }

    inline int getBuildTime() {
        try {
            return std::stoi(manifest["buildtime"]);
        } catch (std::exception &ex) {
            return -1;
        }
    }

    inline Image getImage(const std::string &name) {
        auto data = readRomFile(name, ".pdi");
        return readImage(data.data(), data.size());
    }

    inline ImageTable getImageTable(const std::string &name) {
        auto data = readRomFile(name, ".pdt");
        return readImageTable(data.data(), data.size());
    }

    inline StringTable getStringTable(const std::string &name) {
        auto data = readRomFile(name, ".pds");
        return readStringTable(data.data(), data.size());
    }

    inline Font getFont(const std::string &name) {
        auto data = readRomFile(name, ".pft");
        return readFont(data.data(), data.size());
    }

    inline Audio getAudio(const std::string &name) {
        auto data = readRomFile(name, ".pda");
        return readAudio(data.data(), data.size());
    }

    inline Video getVideo(const std::string &name) {
        auto data = readRomFile(name, ".pdv");
        return readVideo(data.data(), data.size());
    }

    static std::vector<File> loadPDZ(const std::vector<uint8_t> &data);
    static StringTable readStringTable(const uint8_t *data, size_t dataSize);
    static Font readFont(const uint8_t *data, size_t dataSize);
    static Font readFontData(const uint8_t *data, bool wide);
    static Image readImage(const uint8_t *data, size_t dataSize);
    static ImageTable readImageTable(const uint8_t *data, size_t dataSize);
    static Audio readAudio(const uint8_t *data, size_t dataSize);
    static Video readVideo(const uint8_t *data, size_t dataSize);
    static FileType getFileType(const uint8_t *header);

    bool loaded{};
    std::unique_ptr<libzippp::ZipArchive> zip;
    std::filesystem::path path;
    std::map<std::string, std::string> manifest;
    std::vector<File> outerFiles;
    std::vector<File> pdzFiles; // Todo: Do later loaded PDZ files get added?
    std::vector<uint8_t> binary;
    uint32_t binarySize{};
    uint32_t combinedProgramSize{};
    uint32_t eventHandlerAddress{};
    bool hasLua{};

private:
    constexpr static auto LUA_MAGIC = "\27Lua";
    constexpr static auto PDZ_MAGIC = "Playdate PDZ";
    constexpr static auto PDX_MAGIC = "Playdate PDX";
    constexpr static auto FONT_MAGIC = "Playdate FNT";
    constexpr static auto STRING_TABLE_MAGIC = "Playdate STR";
    constexpr static auto IMAGE_MAGIC = "Playdate IMG";
    constexpr static auto IMAGE_TABLE_MAGIC = "Playdate IMT";
    constexpr static auto AUDIO_MAGIC = "Playdate AUD";
    constexpr static auto VIDEO_MAGIC = "Playdate VID";

    void loadManifest();

    static ImageCell readCell(const uint8_t *data);
};
