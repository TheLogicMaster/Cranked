#pragma once

#include "gen/PlaydateAPI.hpp"
#include "NativeResource.hpp"

#include <filesystem>
#include <vector>

namespace cranked {

    class Cranked;

    struct SDFile_32 : NativeResource {
        explicit SDFile_32(Cranked &cranked);

        SDFile_32(const SDFile_32 &other) = delete;
        SDFile_32(SDFile_32 &&other) = delete;
        ~SDFile_32() override;

        FILE *file{};
        std::vector<uint8_t> *romData{};
        int romFilePosition{};
    };

    class File {
    public:

        explicit File(Cranked &cranked);

        void init();

        void reset();

        int listFiles(const char *path, bool showHidden, std::vector<std::string> &files);

        bool exists(const std::string &path);

        int mkdir(const std::string &path);

        int rename(const std::string &path, const std::string &newPath);

        int unlink(const std::string &path, bool recursive);

        [[nodiscard]] const char *getType(const std::string &path) const;

        int stat(const std::string &path, FileStat_32 &stat);

        SDFile_32 *open(const char *path, FileOptions mode);

        int close(SDFile_32 *file);

        int flush(SDFile_32 *file);

        std::string readline(SDFile_32 *file);

        int read(SDFile_32 *file, void *buf, int len);

        int write(SDFile_32 *file, void *buf, int len);

        int seek(SDFile_32 *file, int pos, int whence);

        int tell(SDFile_32 *file);

        bool isFileOpen(SDFile_32 *file) {
            return std::find_if(openFiles.begin(), openFiles.end(), [&](auto &val){ return val.get() == file; }) != openFiles.end();
        }

        Cranked &cranked;

        std::vector<ResourcePtr<SDFile_32>> openFiles{};
        std::filesystem::path appDataPath = "./appData/";
        cref_t lastError{};

    private:
        std::filesystem::path romDataPath;
    };

}
