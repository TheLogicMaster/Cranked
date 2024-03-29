#include "File.hpp"
#include "Cranked.hpp"

#include <regex>
#include <chrono>

using namespace cranked;

void File::init() {
    romDataPath = appDataPath / cranked->rom->getBundleID();
    std::filesystem::create_directory(appDataPath);
    std::filesystem::create_directory(romDataPath);
}

void File::reset() {
    for (auto file : openFiles)
        close(file);
    openFiles.clear();
    lastError = 0;
}

static std::string sanitizePath(const std::string &path) {
    return std::regex_replace(path, std::regex("\\.\\."), ""); // This is pretty crude, but should hopefully protect against inadvertent directory traversal
}

int File::listFiles(const char *path, bool showHidden, std::vector<std::string> &files) {
    auto base = std::filesystem::path(path ? path : "").lexically_normal();

    bool exists = false;

    auto dataPath = base.empty() ? romDataPath : romDataPath / base;
    if (std::filesystem::exists(dataPath) and std::filesystem::is_directory(dataPath)) {
        try {
            for (auto &entry: std::filesystem::directory_iterator(dataPath))
                if (showHidden or !entry.path().filename().string().starts_with("."))
                    files.emplace_back(std::string(std::filesystem::path(entry).lexically_relative(dataPath)) + (entry.is_directory() ? "/" : ""));
        } catch (std::exception &ex) {
            lastError = cranked->getEmulatedStringLiteral(ex.what());
            return -1;
        }
        exists = true;
    }

    if (base.empty() or cranked->rom->findRomFile(base)) {
        auto romFiles = cranked->rom->listRomFiles(base, false);
        for (auto &file : romFiles) {
            if (!showHidden and std::filesystem::path(file).filename().string().starts_with('.'))
                 continue;
            files.emplace_back(file);
        }
        exists = true;
    }

    return exists ? 0 : -1;
}

bool File::exists(const std::string &path) {
    if (std::filesystem::exists(romDataPath / path))
        return true;
    return cranked->rom->findRomFile(path) != nullptr;
}

int File::mkdir(const std::string &path) {
    // Documentation lies, both Lua and C API create intermediate directories
    try {
        std::filesystem::create_directories(romDataPath / sanitizePath(path));
    } catch (std::exception &ex) {
        lastError = cranked->getEmulatedStringLiteral(ex.what());
        return -1;
    }
    return 0;
}

int File::rename(const std::string &path, const std::string &newPath) {
    try {
        std::filesystem::rename(romDataPath / sanitizePath(path), romDataPath / sanitizePath(newPath));
    } catch (std::exception &ex) {
        lastError = cranked->getEmulatedStringLiteral(ex.what());
        return -1;
    }
    return 0;
}

int File::unlink(const std::string &path, bool recursive) {
    auto sanitized = sanitizePath(path);
    try {
        if (recursive)
            std::filesystem::remove_all(romDataPath / sanitized);
        else
            std::filesystem::remove(romDataPath / sanitized);
    } catch (std::exception &ex) {
        lastError = cranked->getEmulatedStringLiteral(ex.what());
        return -1;
    }
    return 0;
}

const char *File::getType(const std::string &path) {
    auto romFile = cranked->rom->findRomFile(path);
    return romFile ? Rom::FILE_TYPE_NAMES[(int) romFile->type] : nullptr;
}

int File::stat(const std::string &path, FileStat_32 &stat) {
    auto dataPath = romDataPath / sanitizePath(path);
    auto romFile = cranked->rom->findRomFile(path);
    auto setTime = [&](const std::time_t &modTime){
        // Todo: Respect timezone offsets
        auto modTimePoint = std::chrono::system_clock::from_time_t(modTime);
        auto ymd = std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(modTimePoint)};
        auto timeOffset = modTimePoint - std::chrono::floor<std::chrono::days>(modTimePoint);
        std::chrono::hh_mm_ss time{timeOffset};
        stat.m_year = (int) ymd.year();
        stat.m_month = (int) (uint) ymd.month();
        stat.m_day = (int) (uint) ymd.day();
        stat.m_hour = (int) time.hours().count();
        stat.m_minute = (int) time.minutes().count();
        stat.m_second = (int) time.seconds().count();
    };
    if (std::filesystem::exists(dataPath)) {
        stat.isdir = std::filesystem::is_directory(dataPath);
        stat.size = (uint32_t) std::filesystem::file_size(dataPath);
        setTime(to_time_t(std::filesystem::last_write_time(dataPath)));
        return 0;
    } else if (romFile) {
        stat.isdir = false;
        stat.size = (uint32_t) romFile->data.size();
        setTime(romFile->modTime);
        return 0;
    }
    return -1;
}

SDFile_32 *File::open(const char *path, FileOptions mode) {
    if (!path) {
        lastError = cranked->getEmulatedStringLiteral("Invalid path");
        return nullptr;
    }
    auto normalized = std::filesystem::path(path).lexically_normal();

    auto dataPath = romDataPath / normalized;
    if (mode == FileOptions::Write or mode == FileOptions::Append) {
        auto fd = fopen(dataPath.c_str(), mode == FileOptions::Append ? "ab" : "wb");
        if (!fd) {
            lastError = cranked->getEmulatedStringLiteral("Failed to open file");
            return nullptr;
        }
        auto file = cranked->heap.construct<SDFile_32>();
        file->file = fd;
        openFiles.emplace(file);
        return file;
    }

    if (mode == FileOptions::ReadData or mode == FileOptions::ReadDataFallback) {
        if (std::filesystem::exists(dataPath) and !std::filesystem::is_directory(dataPath)) {
            auto fd = fopen(dataPath.c_str(), "rb");
            if (!fd) {
                lastError = cranked->getEmulatedStringLiteral("Failed to open file");
                return nullptr;
            }
            auto file = cranked->heap.construct<SDFile_32>();
            file->file = fd;
            openFiles.emplace(file);
            return file;
        }
        if (mode == FileOptions::ReadData) {
            lastError = cranked->getEmulatedStringLiteral("File not found");
            return nullptr;
        }
    }

    auto romFile = cranked->rom->findRomFile(normalized);
    if (romFile) {
        SDFile_32 *file;
        if (romFile->data.empty()) {
            auto fd = fopen((cranked->rom->path / romFile->name).c_str(), "rb");
            if (!fd) {
                lastError = cranked->getEmulatedStringLiteral("Failed to open file");
                return nullptr;
            }
            file = cranked->heap.construct<SDFile_32>();
            file->file = fd;
        } else {
            file = cranked->heap.construct<SDFile_32>();
            file->romData = &romFile->data;
            file->romFilePosition = 0;
        }
        openFiles.emplace(file);
        return file;
    }

    lastError = cranked->getEmulatedStringLiteral("File not found");
    return nullptr;
}

int File::close(SDFile_32 *file) {
    if (!openFiles.contains(file)) {
        lastError = cranked->getEmulatedStringLiteral("File closed");
        return -1;
    }
    if (file->file)
        fclose(file->file);
    openFiles.erase(file);
    cranked->heap.destruct(file);
    return 0;
}

int File::flush(SDFile_32 *file) {
    if (!openFiles.contains(file)) {
        lastError = cranked->getEmulatedStringLiteral("File closed");
        return -1;
    }
    if (file->file)
        fflush(file->file);
    return 0;
}

std::string File::readline(SDFile_32 *file) {
    if (!openFiles.contains(file)) {
        lastError = cranked->getEmulatedStringLiteral("File closed");
        return "";
    }
    std::string line;
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
        else if (value == '\r') {
            read(file, &value, 1); // Read presumed following '\n', ignoring all errors
        } else
            line += value;
    }
    return line;
}

int File::read(SDFile_32 *file, void *buf, int len) {
    if (!openFiles.contains(file)) {
        lastError = cranked->getEmulatedStringLiteral("File closed");
        return -1;
    }
    if (file->file) {
        for (int i = 0; i < len; i++) {
            auto c = fgetc(file->file);
            if (c < 0) {
                if (ferror(file->file)) {
                    lastError = cranked->getEmulatedStringLiteral("Error reading file");
                    return -1;
                }
                return i;
            }
            ((char *) buf)[i] = (char) c;
        }
        return len;
    } else {
        auto romData = file->romData;
        auto length = std::min(len, (int) romData->size() - file->romFilePosition);
        if (length <= 0)
            return 0;
        memcpy(buf, romData->data() + file->romFilePosition, length);
        file->romFilePosition += length;
        return length;
    }
}

int File::write(SDFile_32 *file, void *buf, int len) {
    if (!openFiles.contains(file)) {
        lastError = cranked->getEmulatedStringLiteral("File closed");
        return -1;
    }
    int result = (int) fwrite(buf, 1, len, file->file);
    if (result < 0) {
        lastError = cranked->getEmulatedStringLiteral("Error writing to file");
        return -1;
    }
    return result;
}

int File::seek(SDFile_32 *file, int pos, int whence) {
    if (!openFiles.contains(file)) {
        lastError = cranked->getEmulatedStringLiteral("File closed");
        return -1;
    }
    if (file->file)
        return fseek(file->file, pos, whence) ? -1 : 0;
    else {
        auto romData = file->romData;
        int target;
        if (whence == SEEK_END)
            target = (int) romData->size() + pos;
        else if (whence == SEEK_CUR)
            target = file->romFilePosition + pos;
        else if (whence == SEEK_SET)
            target = pos;
        else {
            lastError = cranked->getEmulatedStringLiteral("Invalid seek whence");
            return -1;
        }
        if (target > romData->size()) {
            lastError = cranked->getEmulatedStringLiteral("Invalid seek offset");
            return -1;
        }
        file->romFilePosition = target;
        return 0;
    }
}

int File::tell(SDFile_32 *file) {
    if (!openFiles.contains(file)) {
        lastError = cranked->getEmulatedStringLiteral("File closed");
        return -1;
    }
    if (file->file)
        return (int) ftell(file->file);
    else
        return file->romFilePosition;
}
