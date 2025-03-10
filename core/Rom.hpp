#pragma once

#include "PlaydateTypes.hpp"

namespace cranked {

    class Cranked;

    // Todo: Validate ROM files to avoid unsafe memory accesses
    // Todo: Should probably have image formats stored compactly since that's how they are used anyway

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
            map<int, int8> kerningTable;
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

        vector<uint8> readRomFile(string name, const string &extension = "", const vector<string> &replacedExtensions = {});

        File *findRomFile(string name);

        vector<string> listRomFiles(string base, bool recursive); // Doesn't include PDZ contents

        const char *getManifestString(const string &name) {
            if (auto it = manifest.find(name); it != manifest.end())
                return it->second.c_str();
            return nullptr;
        }

        const char * getName() {
            return getManifestString("name");
        }

        const char *getAuthor() {
            return getManifestString("author");
        }

        const char *getDescription() {
            return getManifestString("description");
        }

        const char *getBundleID() {
            return getManifestString("bundleID");
        }

        const char *getVersion() {
            return getManifestString("version");
        }

        int getBuildNumber() {
            auto buildNumber = getManifestString("buildNumber");
            if (not buildNumber)
                return -1;
            try {
                return stoi(buildNumber);
            } catch (exception &) {
                return -1;
            }
        }

        const char *getImagePath() {
            return getManifestString("imagePath");
        }

        const char *getLaunchSoundPath() {
            return getManifestString("launchSoundPath");
        }

        const char *getContentWarning() {
            return getManifestString("contentWarning");
        }

        const char *getContentWarning2() {
            return getManifestString("contentWarning2");
        }

        Version getPdxVersion() {
            auto version = getManifestString("pdxversion");
            if (not version)
                return {};
            try {
                return Version(stoi(version));
            } catch (exception &) {
                return {};
            }
        }

        int getBuildTime() {
            auto buildTime = getManifestString("buildTime");
            if (not buildTime)
                return -1;
            try {
                return stoi(buildTime);
            } catch (exception &) {
                return -1;
            }
        }

        Image getImage(const string &name) {
            auto data = readRomFile(name, ".pdi", { ".gif", ".png" });
            return readImage(data.data(), data.size());
        }

        ImageTable getImageTable(const string &name) {
            auto data = readRomFile(name, ".pdt");
            return readImageTable(data.data(), data.size());
        }

        StringTable getStringTable(const string &name) {
            auto data = readRomFile(name, ".pds", { ".strings" });
            return readStringTable(data.data(), data.size());
        }

        map<PDLanguage, StringTable> getStrings() {
            map<PDLanguage, StringTable> mapping;

            auto loadLanguage = [&](PDLanguage language, const string &name) {
                auto &table = mapping[language];
                try {
                    if (findRomFile(name))
                        table = getStringTable(name);
                } catch (exception &) {}
            };

            loadLanguage(PDLanguage::English, "en");
            loadLanguage(PDLanguage::Japanese, "jp");

            return mapping;
        }

        Font getFont(const string &name) {
            auto data = readRomFile(name, ".pft", { ".fnt" });
            return readFont(data.data(), data.size());
        }

        Audio getAudio(const string &name) {
            auto data = readRomFile(name, ".pda", { ".wav", ".aiff" });
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

        static vector<uint8> writeImage(Bitmap image);

        static File *findSystemFile(string path);

        static Font readSystemFont(const string &path) {
            auto file = findSystemFile(fs::path("System/Fonts") / path);
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

        static void bufferAddStr(vector<uint8> &buffer, const char *str) {
            while (*str)
                buffer.push_back(*str++);
        }

        template<typename T>
        static void bufferAddVal(vector<uint8> &buffer, T val) {
            for (auto i = 0; i < sizeof(T); i++)
                buffer.push_back(((uint8 *)&val)[i]);
        }

        static ImageCell readImageCell(const uint8 *start);

        static pair<string, File> makeSystemFilePair(string path, FileType type, const uint8 *data, size_t size) {
            path = fs::path(normalizePath(path)).replace_extension().string();
            return pair{ path, File{ path, type, false, (uint32)size, 0, vector(data, data + size), 0 } };
        }

        static unordered_map<string, File> systemFiles;
    };

}
