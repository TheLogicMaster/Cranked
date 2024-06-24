#pragma once

#include "gen/PlaydateAPI.hpp"
#include "NativeResource.hpp"

namespace cranked {

    class Cranked;

    struct SDFile_32 : NativeResource {
        explicit SDFile_32(Cranked &cranked);

        SDFile_32(const SDFile_32 &other) = delete;
        SDFile_32(SDFile_32 &&other) = delete;
        ~SDFile_32() override;

        FILE *file{};
        vector<uint8> *romData{};
        int romFilePosition{};
    };

    class Files {
    public:

        explicit Files(Cranked &cranked);

        void init();

        void reset();

        int listFiles(const char *path, bool showHidden, vector<string> &files);

        bool exists(const string &path);

        int mkdir(const string &path);

        int rename(const string &path, const string &newPath);

        int unlink(const string &path, bool recursive);

        [[nodiscard]] const char *getType(const string &path) const;

        int stat(const string &path, FileStat_32 &stat);

        File open(const char *path, FileOptions mode);

        int close(File file);

        int flush(File file);

        string readline(File file);

        int read(File file, void *buf, int len);

        int write(File file, void *buf, int len);

        int seek(File file, int pos, int whence);

        int tell(File file);

        bool isFileOpen(File file) {
            return openFiles.find(file) != openFiles.end();
        }

        Cranked &cranked;

        unordered_set<FileRef> openFiles{};
        filesystem::path appDataPath = "./appData/";
        cref_t lastError{};

    private:
        filesystem::path romDataPath;
    };

}
