#pragma once

#include "gen/PlaydateAPI.hpp"

namespace cranked {

    class Cranked;

    struct SDFile_32 : NativeResource {
        explicit SDFile_32(Cranked &cranked);

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

        int listFiles(string path, bool showHidden, vector<string> &files);

        bool exists(string path);

        int mkdir(string path);

        int rename(string path, string newPath);

        int unlink(string path, bool recursive);

        [[nodiscard]] const char *getType(string path) const;

        int stat(string path, FileStat_32 &stat);

        File open(string path, FileOptions mode);

        int close(File file);

        int flush(File file);

        string readline(File file);

        int read(File file, void *buf, int len);

        int write(File file, void *buf, int len);

        int seek(File file, int pos, SeekWhence whence);

        int tell(File file);

        bool isFileOpen(File file) const {
            return openFiles.contains(file);
        }

        Cranked &cranked;

        unordered_resource_set<File> openFiles{};
        fs::path appDataPath = "./appData/";
        cref_t lastError{};

    private:
        fs::path romDataPath;
    };

}
