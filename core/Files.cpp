#include "Files.hpp"
#include "Cranked.hpp"

#include <regex>
#include <chrono>

using namespace cranked;

SDFile_32::SDFile_32(Cranked &cranked) : NativeResource(cranked, ResourceType::File, this) {}

SDFile_32::~SDFile_32() {
    if (file)
        fclose(file);
}

Files::Files(Cranked &cranked) : cranked(cranked) {}

void Files::init() {
    auto id = cranked.rom->getBundleID();
    romDataPath = appDataPath / (id ? id : "unknown");
    create_directory(appDataPath);
    create_directory(romDataPath);
}

void Files::reset() {
    for (FileRef &file : vector(openFiles.begin(), openFiles.end())) {
        try {
            close(file);
        } catch (exception &ex) {
            cranked.logMessage(LogLevel::Error, "Exception closing file: %s", ex.what());
        }
        file.reset();
    }
    openFiles.clear();
    lastError = 0;
}

int Files::listFiles(string path, bool showHidden, vector<string> &files) {
    path = normalizePath(path);

    bool exists = false;

    auto dataPath = path.empty() ? romDataPath : romDataPath / path;
    if (fs::exists(dataPath) and is_directory(dataPath)) {
        try {
            for (auto &entry: fs::directory_iterator(dataPath))
                if (showHidden or !entry.path().filename().string().starts_with("."))
                    files.emplace_back(string(fs::path(entry).lexically_relative(dataPath)) + (entry.is_directory() ? "/" : ""));
        } catch (exception &ex) {
            lastError = cranked.getEmulatedStringLiteral(ex.what());
            return -1;
        }
        exists = true;
    }

    if (path.empty() or cranked.rom->findRomFile(path)) {
        auto romFiles = cranked.rom->listRomFiles(path, false);
        for (auto &file : romFiles) {
            if (!showHidden and fs::path(file).filename().string().starts_with('.'))
                 continue;
            files.emplace_back(file);
        }
        exists = true;
    }

    return exists ? 0 : -1;
}

bool Files::exists(string path) {
    path = normalizePath(path);
    if (fs::exists(romDataPath / path))
        return true;
    return cranked.rom->findRomFile(path) != nullptr;
}

int Files::mkdir(string path) {
    // Documentation lies, both Lua and C API create intermediate directories
    path = normalizePath(path);
    try {
        create_directories(romDataPath / normalizePath(path));
    } catch (exception &ex) {
        lastError = cranked.getEmulatedStringLiteral(ex.what());
        return -1;
    }
    return 0;
}

int Files::rename(string path, string newPath) {
    path = normalizePath(path);
    newPath = normalizePath(newPath);
    try {
        fs::rename(romDataPath / path, romDataPath / newPath);
    } catch (exception &ex) {
        lastError = cranked.getEmulatedStringLiteral(ex.what());
        return -1;
    }
    return 0;
}

int Files::unlink(string path, bool recursive) {
    path = normalizePath(path);
    try {
        if (recursive)
            remove_all(romDataPath / path);
        else
            fs::remove(romDataPath / path);
    } catch (exception &ex) {
        lastError = cranked.getEmulatedStringLiteral(ex.what());
        return -1;
    }
    return 0;
}

const char *Files::getType(string path) const {
    path = normalizePath(path);
    auto romFile = cranked.rom->findRomFile(path);
    return romFile ? Rom::FILE_TYPE_NAMES[(int) romFile->type] : nullptr;
}

int Files::stat(string path, FileStat_32 &stat) {
    path = normalizePath(path);
    auto dataPath = romDataPath / path;
    auto romFile = cranked.rom->findRomFile(path);
    auto setTime = [&](const time_t &modTime){
        // Todo: Respect timezone offsets
        auto modTimePoint = chrono::system_clock::from_time_t(modTime);
        auto ymd = chrono::year_month_day{chrono::floor<chrono::days>(modTimePoint)};
        auto timeOffset = modTimePoint - chrono::floor<chrono::days>(modTimePoint);
        chrono::hh_mm_ss time{timeOffset};
        stat.m_year = (int) ymd.year();
        stat.m_month = (int) (uint) ymd.month();
        stat.m_day = (int) (uint) ymd.day();
        stat.m_hour = (int) time.hours().count();
        stat.m_minute = (int) time.minutes().count();
        stat.m_second = (int) time.seconds().count();
    };
    if (fs::exists(dataPath)) {
        stat.isdir = is_directory(dataPath);
        stat.size = (uint32)file_size(dataPath);
        setTime(to_time_t(last_write_time(dataPath)));
        return 0;
    }
    if (romFile) {
        stat.isdir = false;
        stat.size = romFile->size;
        setTime(romFile->modTime);
        return 0;
    }
    return -1;
}

File Files::open(string path, FileOptions mode) {
    path = normalizePath(path);

    auto dataPath = romDataPath / path;
    if (mode == FileOptions::Write or mode == FileOptions::Append) {
        auto fd = fopen(dataPath.c_str(), mode == FileOptions::Append ? "ab" : "wb");
        if (!fd) {
            lastError = cranked.getEmulatedStringLiteral("Failed to open file");
            return nullptr;
        }
        auto file = cranked.heap.construct<SDFile_32>(cranked);
        file->file = fd;
        openFiles.emplace(file);
        return file;
    }

    if (mode == FileOptions::ReadData or mode == FileOptions::ReadDataFallback) {
        if (fs::exists(dataPath) and !is_directory(dataPath)) {
            auto fd = fopen(dataPath.c_str(), "rb");
            if (!fd) {
                lastError = cranked.getEmulatedStringLiteral("Failed to open file");
                return nullptr;
            }
            auto file = cranked.heap.construct<SDFile_32>(cranked);
            file->file = fd;
            openFiles.emplace(file);
            return file;
        }
        if (mode == FileOptions::ReadData) {
            lastError = cranked.getEmulatedStringLiteral("File not found");
            return nullptr;
        }
    }

    auto romFile = cranked.rom->findRomFile(path);
    if (romFile) {
        File file;
        if (romFile->data.empty()) {
            auto fd = fopen((cranked.rom->path / romFile->name).c_str(), "rb");
            if (!fd) {
                lastError = cranked.getEmulatedStringLiteral("Failed to open file");
                return nullptr;
            }
            file = cranked.heap.construct<SDFile_32>(cranked);
            file->file = fd;
        } else {
            file = cranked.heap.construct<SDFile_32>(cranked);
            file->romData = &romFile->data;
            file->romFilePosition = 0;
        }
        openFiles.emplace(file);
        return file;
    }

    lastError = cranked.getEmulatedStringLiteral("File not found");
    return nullptr;
}

int Files::close(File file) {
    if (!isFileOpen(file)) {
        lastError = cranked.getEmulatedStringLiteral("File closed");
        return -1;
    }
    if (file->file)
        fclose(file->file);
    file->file = nullptr;
    eraseByEquivalentKey(openFiles, file);
    return 0;
}

int Files::flush(File file) {
    if (!isFileOpen(file)) {
        lastError = cranked.getEmulatedStringLiteral("File closed");
        return -1;
    }
    if (file->file)
        fflush(file->file);
    return 0;
}

string Files::readline(File file) {
    if (!isFileOpen(file)) {
        lastError = cranked.getEmulatedStringLiteral("File closed");
        return "";
    }
    string line;
    line.reserve(64);
    while (true) {
        char value;
        auto result = read(file, &value, 1);
        if (result < 0)
            return "";
        if (result == 0)
            break;
        if (value == '\n')
            break;
        if (value == '\r') {
            read(file, &value, 1); // Read presumed following '\n', ignoring all errors
        } else
            line += value;
    }
    return line;
}

int Files::read(File file, void *buf, int len) {
    if (!isFileOpen(file)) {
        lastError = cranked.getEmulatedStringLiteral("File closed");
        return -1;
    }
    if (file->file) {
        for (int i = 0; i < len; i++) {
            auto c = fgetc(file->file);
            if (c < 0) {
                if (ferror(file->file)) {
                    lastError = cranked.getEmulatedStringLiteral("Error reading file");
                    return -1;
                }
                return i;
            }
            ((char *) buf)[i] = (char) c;
        }
        return len;
    }
    auto romData = file->romData;
    auto length = min(len, (int) romData->size() - file->romFilePosition);
    if (length <= 0)
        return 0;
    memcpy(buf, romData->data() + file->romFilePosition, length);
    file->romFilePosition += length;
    return length;
}

int Files::write(File file, void *buf, int len) {
    if (!isFileOpen(file)) {
        lastError = cranked.getEmulatedStringLiteral("File closed");
        return -1;
    }
    int result = (int) fwrite(buf, 1, len, file->file);
    if (result < 0) {
        lastError = cranked.getEmulatedStringLiteral("Error writing to file");
        return -1;
    }
    return result;
}

int Files::seek(File file, int pos, SeekWhence whence) {
    if (!isFileOpen(file)) {
        lastError = cranked.getEmulatedStringLiteral("File closed");
        return -1;
    }
    if (file->file)
        return fseek(file->file, pos, (int)whence) ? -1 : 0;
    auto romData = file->romData;
    int target;
    if (whence == SeekWhence::End)
        target = (int) romData->size() + pos;
    else if (whence == SeekWhence::Current)
        target = file->romFilePosition + pos;
    else if (whence == SeekWhence::Set)
        target = pos;
    else {
        lastError = cranked.getEmulatedStringLiteral("Invalid seek whence");
        return -1;
    }
    if (target > romData->size()) {
        lastError = cranked.getEmulatedStringLiteral("Invalid seek offset");
        return -1;
    }
    file->romFilePosition = target;
    return 0;
}

int Files::tell(File file) {
    if (!isFileOpen(file)) {
        lastError = cranked.getEmulatedStringLiteral("File closed");
        return -1;
    }
    if (file->file)
        return (int) ftell(file->file);
    return file->romFilePosition;
}
