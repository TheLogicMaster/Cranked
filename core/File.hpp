#pragma once

#include "gen/PlaydateAPI.hpp"

#include <filesystem>
#include <unordered_set>
#include <vector>

namespace cranked {

    class Cranked;

    struct SDFile_32 {
        FILE *file;
        std::vector<uint8_t> *romData;
        int romFilePosition;
    };

    class File {
    public:

        inline explicit File(Cranked *cranked) : cranked(cranked) {}

        void init();

        void reset();

        int listFiles(const char *path, bool showHidden, std::vector<std::string> &files);

        bool exists(const std::string &path);

        int mkdir(const std::string &path);

        int rename(const std::string &path, const std::string &newPath);

        int unlink(const std::string &path, bool recursive);

        const char *getType(const std::string &path);

        int stat(const std::string &path, FileStat_32 &stat);

        SDFile_32 *open(const char *path, FileOptions mode);

        int close(SDFile_32 *file);

        int flush(SDFile_32 *file);

        std::string readline(SDFile_32 *file);

        int read(SDFile_32 *file, void *buf, int len);

        int write(SDFile_32 *file, void *buf, int len);

        int seek(SDFile_32 *file, int pos, int whence);

        int tell(SDFile_32 *file);

        Cranked *cranked;

        std::unordered_set<SDFile_32 *> openFiles;
        std::filesystem::path appDataPath = "./appData/";
        cref_t lastError{};

    private:
        std::filesystem::path romDataPath;
    };

}
