#pragma once

#include "PlaydateTypes.hpp"
#include "Utils.hpp"

#include <boost/asio.hpp>
#include <string>
#include <queue>
#include <map>
#include <unordered_set>
#include <regex>
#include <format>
#include <capstone/capstone.h>

namespace cranked {

    namespace asio = boost::asio;

    class Cranked;
    class NativeEngine;

    class Debugger {
    public:
        explicit Debugger(Cranked &cranked);
        ~Debugger();
        Debugger(const Debugger &other) = delete;
        Debugger(Debugger &&other) = delete;

        void init();
        void reset();
        void update();

        void handleBreakpoint();

        bool handleInvocationDebugging();

        void clearBreakpoints() {
            breakpoints.clear();
        }

        bool hasBreakpoint(cref_t address) {
            return breakpoints.contains(address);
        }

        void addBreakpoint(cref_t address);

        void removeBreakpoint(cref_t address) {
            breakpoints.erase(address);
        }

        const std::unordered_set<cref_t> &getBreakpoints() const {
            return breakpoints;
        }

        const cs_insn *getDisassembly() {
            return disassembly;
        }

        int getDisassemblySize() const {
            return disassemblySize;
        }

        void halt() {
            if (!enabled)
                return;
            halted = true;
        }

        void resume() {
            halted = false;
        }

        bool isHalted() const {
            return halted && enabled;
        }

        void step() {
            if (!enabled or !halted)
                return;
            halted = false;
            singleStepping = true;
        }

        bool handleStep() {
            bool stepping = singleStepping && enabled;
            singleStepping = false;
            return stepping;
        }

        bool &isEnabled() {
            return enabled;
        }

    private:
        static constexpr const char *MESSAGE_EMPTY = "";
        static constexpr const char *MESSAGE_OK = "OK";
        static constexpr const char *SUPPORTS_HARDWARE_BREAKPOINTS = "hwbreak+";
        static constexpr int MESSAGE_MAX_LENGTH = 4096;
        static constexpr const char *HARDWARE_BREAKPOINT_STR = "1";
        static constexpr uint16_t BREAKPOINT_INSTRUCTION = 0xBEBE;

        void startTcpAccept();
        void startTcpRead();
        void processDecodeBuffer();
        void processPacket(std::string_view data);

        void sendData(std::string data);

        void sendResponse(const std::string &data);

        static uint8_t computePacketChecksum(std::string_view data);

        void sendErrorResponse(int error) {
            sendResponse(std::format("E{:02x}", error));
        }

        void sendHaltResponse() {
            sendResponse(std::format("S{:02x}", SIGTRAP));
        }

        void handleCmdHaltReason(const std::vector<std::string> &args);
        void handleCmdReadReg(const std::vector<std::string> &args);
        void handleCmdWriteReg(const std::vector<std::string> &args);
        void handleCmdDetach(const std::vector<std::string> &args);
        void handleCmdReadMem(const std::vector<std::string> &args);
        void handleCmdWriteMem(const std::vector<std::string> &args);
        void handleCmdContinue(const std::vector<std::string> &args);
        void handleCmdContinueWithSignal(const std::vector<std::string> &args);
        void handleCmdStep(const std::vector<std::string> &args);
        void handleCmdStepWithSignal(const std::vector<std::string> &args);
        void handleCmdInsertBreakpoint(const std::vector<std::string> &args);
        void handleCmdRemoveBreakpoint(const std::vector<std::string> &args);
        void handleCmdSupported(const std::vector<std::string> &args);
        void handleCmdAttached(const std::vector<std::string> &args);

        const std::map<std::string, void(Debugger::*)(const std::vector<std::string> &args)> packetHandlers {
                {"?", &Debugger::handleCmdHaltReason},
                {"g", &Debugger::handleCmdReadReg},
                {"G", &Debugger::handleCmdWriteReg},
                {"D", &Debugger::handleCmdDetach},
                {"m", &Debugger::handleCmdReadMem},
                {"M", &Debugger::handleCmdWriteMem},
                {"c", &Debugger::handleCmdContinue},
                {"C", &Debugger::handleCmdContinueWithSignal},
                {"s", &Debugger::handleCmdStep},
                {"S", &Debugger::handleCmdStepWithSignal},
                {"Z", &Debugger::handleCmdInsertBreakpoint},
                {"z", &Debugger::handleCmdRemoveBreakpoint},
                {"qSupported", &Debugger::handleCmdSupported},
                {"qAttached", &Debugger::handleCmdAttached},
        };

        Cranked &cranked;
        NativeEngine &nativeEngine;
        csh capstoneHandle{};
        cs_insn *disassembly{};
        int disassemblySize{};
        asio::io_context ioContext;
        asio::ip::tcp::acceptor tcpAcceptor {ioContext};
        asio::ip::tcp::socket tcpSocket {ioContext};
        std::array<char, MESSAGE_MAX_LENGTH> tcpReceiveBuffer{};
        std::string packetDecodeBuffer;
        std::queue<std::string> tcpTransmitBuffers;
        bool tcpClientConnected{};
        std::unordered_set<cref_t> breakpoints;
        bool halted{};
        bool singleStepping{};
        bool enabled{};
        std::unordered_map<cref_t, uint16_t> breakpointReplacements;
        bool wasAtBreakpoint{};
    };

}
