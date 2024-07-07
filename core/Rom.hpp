#pragma once

#include "PlaydateTypes.hpp"

namespace cranked {

    class Cranked;

    // Todo: Validate ROM files to avoid unsafe memory accesses
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

        constexpr static const char *FILE_TYPE_NAMES[]{ // Todo: Verify names
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
            string name;
            FileType type{};
            bool isDir{};
            uint32 size{};
            uint32 extra{}; // Used for audio
            vector<uint8> data; // Populated for PDZ contents
            time_t modTime;
        };

        // Stored with one byte per pixel
        struct ImageCell {
            int width{};
            int height{};
            vector<uint8> data;
            vector<uint8> mask;
        };

        struct Image {
            int width{};
            int height{};
            uint32 extra{}; // Probably unused
            ImageCell cell;
        };

        struct ImageTable {
            int cellsPerRow{};
            vector<ImageCell> cells;
        };

        struct StringTable {
            map<string, string> strings;
        };

        struct FontGlyph {
            int advance{};
            map<int, int8> shortKerningTable; // Page 0 entries
            map<int, int8> longKerningTable;
            ImageCell cell;
        };

        struct Font {
            int tracking{};
            int glyphWidth{};
            int glyphHeight{};
            unordered_map<int, unordered_map<int, FontGlyph>> glyphs; // Bottom 8 bits of codepoint is page number, upper 8? bits is page index
        };

        struct Audio {
            SoundFormat soundFormat;
            bool stereo;
            uint32 sampleRate;
            vector<int16> samples; // Not sure if this is even useful, since it needs to support playing raw data, anyway
            vector<uint8> data;
        };

        struct Video {
            float framerate;
            int width;
            int height;
            vector<vector<uint8>> frames;
        };

        explicit Rom(const string &path, Cranked *cranked = nullptr);

        ~Rom();

        void load();

        void unload();

        vector<uint8> readRomFile(const string &name, const string &extension = "");

        File *findRomFile(const string &name);

        vector<string> listRomFiles(string base, bool recursive); // Doesn't include PDZ contents

        string getName() {
            return manifest["name"];
        }

        string getAuthor() {
            return manifest["author"];
        }

        string getDescription() {
            return manifest["description"];
        }

        string getBundleID() {
            return manifest["bundleID"];
        }

        string getVersion() {
            return manifest["version"];
        }

        int getBuildNumber() {
            try {
                return stoi(manifest["buildNumber"]);
            } catch (exception &ex) {
                return -1;
            }
        }

        string getImagePath() {
            return manifest["imagePath"];
        }

        string getLaunchSoundPath() {
            return manifest["launchSoundPath"];
        }

        string getContentWarning() {
            return manifest["contentWarning"];
        }

        string getContentWarning2() {
            return manifest["contentWarning2"];
        }

        Version getPdxVersion() {
            try {
                return Version(stoi(manifest["pdxversion"]));
            } catch (exception &ex) {
                return Version(0);
            }
        }

        int getBuildTime() {
            try {
                return stoi(manifest["buildtime"]);
            } catch (exception &ex) {
                return -1;
            }
        }

        Image getImage(const string &name) {
            auto data = readRomFile(name, ".pdi");
            return readImage(data.data(), data.size());
        }

        ImageTable getImageTable(const string &name) {
            auto data = readRomFile(name, ".pdt");
            return readImageTable(data.data(), data.size());
        }

        StringTable getStringTable(const string &name) {
            auto data = readRomFile(name, ".pds");
            return readStringTable(data.data(), data.size());
        }

        Font getFont(const string &name) {
            auto data = readRomFile(name, ".pft");
            return readFont(data.data(), data.size());
        }

        Audio getAudio(const string &name) {
            auto data = readRomFile(name, ".pda");
            return readAudio(data.data(), data.size());
        }

        Video getVideo(const string &name) {
            auto data = readRomFile(name, ".pdv");
            return readVideo(data.data(), data.size());
        }

        void logMessage(LogLevel level, const char *fmt, ...) const;

        static vector<File> loadPDZ(const vector<uint8> &data);

        static StringTable readStringTable(const uint8 *data, size_t dataSize);

        static Font readFont(const uint8 *data, size_t dataSize);

        static Font readFontData(const uint8 *data, bool wide);

        static Image readImage(const uint8 *data, size_t dataSize);

        static ImageTable readImageTable(const uint8 *data, size_t dataSize);

        static Audio readAudio(const uint8 *data, size_t dataSize);

        static Video readVideo(const uint8 *data, size_t dataSize);

        static FileType getFileType(const uint8 *header);

        static File *findSystemFile(const string &path);

        static Font readSystemFont(const string &path) {
            auto file = findSystemFile(fs::path("Fonts") / path);
            if (!file)
                throw CrankedError("No such system file: " + path);
            return readFont(file->data.data(), file->data.size());
        }

        Cranked *cranked; // Only present for logging, so optional
        bool loaded{};
        unique_ptr<libzippp::ZipArchive> zip;
        fs::path path;
        map<string, string> manifest{};
        vector<File> outerFiles;
        vector<File> pdzFiles; // Todo: Do later loaded PDZ files get added?
        vector<uint8> binary;
        uint32 binarySize{};
        uint32 combinedProgramSize{};
        uint32 eventHandlerAddress{};
        bool hasLua{};
        Version sdkVersion{};

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

        static ImageCell readImageCell(const uint8 *start);

        static pair<string, File> makeSystemFilePair(const string &path, FileType type, const uint8 *data, size_t size) {
            return pair{ path, File{ path, type, false, (uint32)size, 0, vector(data, data + size), 0 } };
        }

        static unordered_map<string, File> systemFiles;
    };

}
