#include "Debugger.hpp"
#include "Cranked.hpp"

#include <regex>

using namespace cranked;

Debugger::Debugger(Cranked &cranked) : cranked(cranked), nativeEngine(cranked.nativeEngine) {
    if (auto result = cs_open(CS_ARCH_ARM, CS_MODE_THUMB, &capstoneHandle); result != CS_ERR_OK)
        throw std::runtime_error(std::format("Failed to open capstone: {}", cs_strerror(result)));
}

Debugger::~Debugger() {
    if (disassembly)
        cs_free(disassembly, disassemblySize);
    cs_close(&capstoneHandle);
}

void Debugger::init() {
    if (cranked.config.debugPort > 0) {
        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), (unsigned short)cranked.config.debugPort);
        tcpAcceptor.open(endpoint.protocol());
        tcpAcceptor.set_option(asio::ip::tcp::socket::reuse_address(true));
        tcpAcceptor.bind(endpoint);
        tcpAcceptor.listen();
        startTcpAccept();
    }

    if (disassemblySize = (int)cs_disasm(capstoneHandle, nativeEngine.getCodeMemory().data() + CODE_OFFSET, nativeEngine.getCodeMemory().size() - CODE_OFFSET, CODE_ADDRESS, 0, &disassembly); disassemblySize == 0)
        throw std::runtime_error(std::format("Failed to disassemble program: {}", cs_strerror(cs_errno(capstoneHandle))));
}

void Debugger::reset() {
    if (tcpSocket.is_open())
        tcpSocket.close();
    if (tcpAcceptor.is_open())
        tcpAcceptor.close();
    tcpClientConnected = false;
    std::queue<std::string>().swap(tcpTransmitBuffers);
    packetDecodeBuffer.clear();
    breakpoints.clear();

    if (disassembly)
        cs_free(disassembly, disassemblySize);
    disassembly = nullptr;

    enabled = false;
    halted = false;
    singleStepping = false;

    wasAtBreakpoint = false;
    breakpointReplacements.clear();
}

void Debugger::update() {
    if (!enabled or cranked.state != Cranked::State::Running)
        halted = false;
    if (tcpClientConnected and !tcpSocket.is_open()) {
        tcpClientConnected = false;
        startTcpAccept();
    }
    ioContext.poll();
}

void Debugger::handleBreakpoint() {
    halted = true;
    wasAtBreakpoint = true;
    sendHaltResponse();
}

void Debugger::addBreakpoint(cref_t address) {
    if (address >= CODE_PAGE_ADDRESS and address < CODE_PAGE_ADDRESS + nativeEngine.getCodeMemory().size())
        breakpoints.emplace(address);
}

void Debugger::startTcpAccept() {
    if (!tcpAcceptor.is_open())
        return;
    tcpAcceptor.async_accept(tcpSocket, [&](const boost::system::error_code &error){
        if (error)
            startTcpAccept();
        else {
            tcpClientConnected = true;
            enabled = true;
            cranked.logMessage(LogLevel::Info, "GDB connected");
            startTcpRead();
        }
    });
}

void Debugger::startTcpRead() {
    tcpSocket.async_read_some(asio::buffer(tcpReceiveBuffer), [&](const boost::system::error_code& error, size_t length){
        if (error == asio::error::eof) {
            tcpClientConnected = false;
            tcpSocket.close();
            startTcpAccept();
            cranked.logMessage(LogLevel::Info, "GDB disconnected");
        } else if (error) {
            startTcpRead();
        } else {
            packetDecodeBuffer.append(tcpReceiveBuffer.begin(), tcpReceiveBuffer.begin() + length);
            processDecodeBuffer();
            startTcpRead();
        }
    });
}

void Debugger::processDecodeBuffer() {
    while (!packetDecodeBuffer.empty()) {
        if (packetDecodeBuffer.starts_with('\03')) {
            cranked.logMessage(LogLevel::Info, "GDB interrupt received");
            packetDecodeBuffer.erase(0, 1);
            halted = true;
            sendHaltResponse();
            continue;
        }
        if (packetDecodeBuffer.starts_with('+')) { // Ignore acknowledgements
            packetDecodeBuffer.erase(0, 1);
            continue;
        }
        if (!packetDecodeBuffer.starts_with('$')) {
            if (auto start = packetDecodeBuffer.find('$'); start != std::string::npos) {
                cranked.logMessage(LogLevel::Warning, "GDB invalid packet received: `%.*s`", (int)start, packetDecodeBuffer.c_str());
                packetDecodeBuffer.erase(0, start);
            } else
                return;
        }
        auto checksumStart = packetDecodeBuffer.find('#');
        if (checksumStart == std::string::npos and packetDecodeBuffer.size() > MESSAGE_MAX_LENGTH + 4) {
            packetDecodeBuffer.clear();
            return;
        }
        if (checksumStart == std::string::npos or checksumStart + 2 >= packetDecodeBuffer.size())
            return;
        sendData("+");
        processPacket({packetDecodeBuffer.begin() + 1, packetDecodeBuffer.begin() + (int) checksumStart});
        packetDecodeBuffer.erase(0, checksumStart + 3);
    }
}

void Debugger::processPacket(std::string_view data) {
    static std::regex expression( // Stolen from: https://github.com/not-chciken/TLMBoy/blob/main/src/gdb_server.cpp
            R"(^(\?)|(D)|(g))"
            R"(|(c)([0-9]*))"
            R"(|(C)([0-9]*)(;[0-9]*)?)"
            R"(|(G)([0-9A-Fa-f]+))"
            R"(|(M)([0-9A-Fa-f]+),([0-9A-Fa-f]+):([0-9A-Fa-f]+))"
            R"(|(m)([0-9A-Fa-f]+),([0-9A-Fa-f]+))"
            R"(|([zZ])([0-1]),([0-9A-Fa-f]+),([0-9]))"
            R"(|(qAttached)$)"
            R"(|(qSupported):(.+))");
    std::vector<std::string> results;
    std::match_results<std::string_view::const_iterator> match;
    std::regex_match(data.begin(), data.end(), match, expression);
    for (uint i = 1; i < (int)match.size(); i++) {
        if (!match[i].str().empty())
            results.push_back(match[i].str());
    }
    if (results.empty()) {
        cranked.logMessage(LogLevel::Verbose, "GDB Unknown Packet: `%.*s`", (int)data.length(), data.begin());
        sendResponse(MESSAGE_EMPTY);
        return;
    }
    cranked.logMessage(LogLevel::Verbose, "GDB Packet: `%.*s`", (int)data.length(), data.begin());
    (this->*packetHandlers.at(results[0]))(results);
}

void Debugger::sendData(std::string data) {
    tcpSocket.async_write_some(asio::buffer(tcpTransmitBuffers.emplace(std::move(data))), [&](const boost::system::error_code& error, size_t){
        tcpTransmitBuffers.pop();
    });
}

void Debugger::sendResponse(const std::string &data) {
    if (!tcpSocket.is_open())
        return;
    std::string response = "$" + data + "#" + std::format("{:02x}", computePacketChecksum(data));;
    cranked.logMessage(LogLevel::Verbose, "GDB Response: `%s`", response.c_str());
    sendData(response);
}

uint8_t Debugger::computePacketChecksum(std::string_view data) {
    uint8_t sum = 0;
    for (char c : data)
        sum += bit_cast<uint8_t>(c);
    return sum;
}

void Debugger::handleCmdHaltReason([[maybe_unused]] const std::vector<std::string> &args) {
    halted = true;
    sendHaltResponse();
}

void Debugger::handleCmdReadReg([[maybe_unused]] const std::vector<std::string> &args) {
    if (!nativeEngine.isLoaded()) {
        sendErrorResponse(SIGSEGV);
        return;
    }
    std::string message;
    for (int i = 0; i < 13; i++)
        message += formatHexStringLE(nativeEngine.readRegister(UC_ARM_REG_R0 + i));
    message += formatHexStringLE(nativeEngine.readRegister(UC_ARM_REG_SP));
    message += formatHexStringLE(nativeEngine.readRegister(UC_ARM_REG_LR));
    message += formatHexStringLE(nativeEngine.readRegister(UC_ARM_REG_PC));
    message += formatHexStringLE(nativeEngine.readRegister(UC_ARM_REG_XPSR));
    for (int i = 0; i < 16; i++)
        message += formatHexStringLE(nativeEngine.readRegister64(UC_ARM_REG_D0 + i));
    message += formatHexStringLE(nativeEngine.readRegister(UC_ARM_REG_FPSCR));
    sendResponse(message);
}

void Debugger::handleCmdWriteReg(const std::vector<std::string> &args) {
    auto &data = args[1];
    const char *start = data.c_str();
    try {
        if ((int)data.size() / 2 != 200)
            throw std::runtime_error("Wrong number of registers");
        for (int i = 0; i < 13; i++) {
            cranked.nativeEngine.writeRegister(UC_ARM_REG_R0 + i, decodeHexStringLE<uint32_t>(std::string_view(start, start + 4 * 2)));
            start += 4 * 2;
        }
        cranked.nativeEngine.writeRegister(UC_ARM_REG_SP, decodeHexStringLE<uint32_t>(std::string_view(start, start + 4 * 2)));
        start += 4 * 2;
        cranked.nativeEngine.writeRegister(UC_ARM_REG_LR, decodeHexStringLE<uint32_t>(std::string_view(start, start + 4 * 2)));
        start += 4 * 2;
        cranked.nativeEngine.writeRegister(UC_ARM_REG_PC, decodeHexStringLE<uint32_t>(std::string_view(start, start + 4 * 2)));
        start += 4 * 2;
        cranked.nativeEngine.writeRegister(UC_ARM_REG_XPSR, decodeHexStringLE<uint32_t>(std::string_view(start, start + 4 * 2)));
        start += 4 * 2;
        for (int i = 0; i < 16; i++) {
            cranked.nativeEngine.writeRegister(UC_ARM_REG_D0 + i, decodeHexStringLE<uint64_t>(std::string_view(start, start + 8 * 2)));
            start += 8 * 2;
        }
        cranked.nativeEngine.writeRegister(UC_ARM_REG_FPSCR, decodeHexStringLE<uint32_t>(std::string_view(start, start + 4 * 2)));
        sendResponse(MESSAGE_OK);
        return;
    } catch (std::exception &ex) {
        cranked.logMessage(LogLevel::Error, "GDB WriteReg exception: %s", ex.what());
    }
    sendErrorResponse(SIGSEGV);
}

void Debugger::handleCmdDetach([[maybe_unused]] const std::vector<std::string> &args) {
    sendResponse(MESSAGE_OK);
    halted = false;
    enabled = false;
}

void Debugger::handleCmdReadMem(const std::vector<std::string> &args) {
    if (!nativeEngine.isLoaded()) {
        sendErrorResponse(SIGSEGV);
        return;
    }
    try {
        auto address = (uint32_t)std::stol(args[1], nullptr, 16);
        int length = std::stoi(args[2], nullptr, 16);
        std::string response;
        for (int i = 0; i < length; i++) {
            auto addr = address + i;
            if (breakpointReplacements.contains(addr) and length == 2)
                response += std::format("{:02x}", breakpointReplacements[addr]);
            else if (!nativeEngine.fromVirtualAddress(addr, false))
                goto fail;
            else
                response += std::format("{:02x}", nativeEngine.isLoaded() ? nativeEngine.virtualRead<uint8_t>(addr) : 0);
        }
        sendResponse(response);
        return;
    } catch (std::exception &ex) {
        cranked.logMessage(LogLevel::Error, "GDB ReadMem exception: %s", ex.what());
    }
    fail:
    sendErrorResponse(SIGSEGV);
}

void Debugger::handleCmdWriteMem(const std::vector<std::string> &args) {
    try {
        auto address = (uint32_t)std::stol(args[1], nullptr, 16);
        int length = std::stoi(args[2], nullptr, 16);
        const std::string &data = args[3];
        if ((int)data.size() == length and nativeEngine.isLoaded()) {
            for (int i = 0; i < length; i++) {
                uint8_t byte{};
                auto result = std::from_chars(data.c_str() + i * 2, data.c_str() + i * 2 + 2, byte, 16);
                if (!nativeEngine.fromVirtualAddress(address + i, false))
                    goto fail;
                nativeEngine.virtualWrite(address + i, result.ec == std::errc{} ? byte : (uint8_t) 0);
            }
            sendResponse(MESSAGE_OK);
            return;
        }
    } catch (std::exception &ex) {
        cranked.logMessage(LogLevel::Error, "GDB WriteMem exception: %s", ex.what());
    }
    fail:
    sendErrorResponse(SIGSEGV);
}

void Debugger::handleCmdContinue([[maybe_unused]] const std::vector<std::string> &args) {
    halted = false;
}

void Debugger::handleCmdContinueWithSignal([[maybe_unused]] const std::vector<std::string> &args) {
    halted = false;
}

void Debugger::handleCmdStep([[maybe_unused]] const std::vector<std::string> &args) {
    halted = false;
    singleStepping = true;
    sendHaltResponse();
}

void Debugger::handleCmdStepWithSignal([[maybe_unused]] const std::vector<std::string> &args) {
    halted = false;
    singleStepping = true;
    sendHaltResponse();
}

void Debugger::handleCmdInsertBreakpoint(const std::vector<std::string> &args) {
    if (args[1] == HARDWARE_BREAKPOINT_STR) {
        try {
            int address = std::stoi(args[2], nullptr, 16);
            if (address >= CODE_PAGE_ADDRESS and address < CODE_PAGE_ADDRESS + nativeEngine.getCodeMemory().size()) {
                breakpoints.emplace(address);
                sendResponse(MESSAGE_OK);
            } else
                sendResponse(MESSAGE_EMPTY);
        } catch (std::range_error &) {
            cranked.logMessage(LogLevel::Error, "GDB invalid breakpoint: %s", args[2].c_str());
            sendResponse(MESSAGE_EMPTY);
        }
    } else
        sendResponse(MESSAGE_EMPTY); // Watchpoints not supported
}

void Debugger::handleCmdRemoveBreakpoint(const std::vector<std::string> &args) {
    if (args[1] == HARDWARE_BREAKPOINT_STR) {
        try {
            breakpoints.erase(std::stoi(args[2], nullptr, 16));
            sendResponse(MESSAGE_OK);
        } catch (std::range_error &) {
            cranked.logMessage(LogLevel::Error, "GDB invalid breakpoint: %s", args[2].c_str());
            sendResponse(MESSAGE_EMPTY);
        }
    } else
        sendResponse(MESSAGE_EMPTY); // Watchpoints not supported
}

void Debugger::handleCmdSupported(const std::vector<std::string> &args) {
    std::string response;
    if (args[1].find(SUPPORTS_HARDWARE_BREAKPOINTS) != std::string::npos) // Only support hardware breakpoints
        response += std::string(SUPPORTS_HARDWARE_BREAKPOINTS) + ";";
    sendResponse(response);
}

void Debugger::handleCmdAttached([[maybe_unused]] const std::vector<std::string> &args) {
    sendResponse("1");
}

bool Debugger::handleInvocationDebugging() {
    while (isHalted()) {
        cranked.updateInternals();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }

    for (auto it = breakpointReplacements.begin(); it != breakpointReplacements.end();) {
        if (!isEnabled() or !breakpoints.contains(it->first)) {
            nativeEngine.virtualWrite(it->first, it->second);
            nativeEngine.flushCaches();
            it = breakpointReplacements.erase(it);
        }
        else
            it++;
    }
    if (isEnabled()) {
        for (auto breakpoint: breakpoints) {
            if (!breakpointReplacements.contains(breakpoint)) {
                breakpointReplacements[breakpoint] = nativeEngine.virtualRead<uint16_t>(breakpoint);
                nativeEngine.virtualWrite(breakpoint, BREAKPOINT_INSTRUCTION);
                nativeEngine.flushCaches();
            }
        }
    }

    bool shouldStep = false;
    cref_t address = cranked.nativeEngine.readRegister(UC_ARM_REG_PC) & ~0x1;
    if (wasAtBreakpoint and breakpointReplacements.contains(address)) {
        nativeEngine.virtualWrite(address, breakpointReplacements[address]);
        breakpointReplacements.erase(address);
        nativeEngine.flushCaches();
        shouldStep = true;
        wasAtBreakpoint = false;
    }
    if (handleStep()) {
        shouldStep = true;
        halt();
    }
    return shouldStep;
}
