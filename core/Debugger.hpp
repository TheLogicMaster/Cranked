#pragma once

#include "PlaydateTypes.hpp"
#include "Utils.hpp"

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

        const unordered_set<cref_t> &getBreakpoints() const {
            return breakpoints;
        }

#if USE_CAPSTONE
        const cs_insn *getDisassembly() {
            return disassembly;
        }
#endif

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
            return halted and enabled;
        }

        void step() {
            if (!enabled or !halted)
                return;
            halted = false;
            singleStepping = true;
        }

        bool handleStep() {
            bool stepping = singleStepping and enabled;
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
        static constexpr uint16 BREAKPOINT_INSTRUCTION = 0xBEBE;

        void startTcpAccept();
        void startTcpRead();
        void processDecodeBuffer();
        void processPacket(string_view data);

        void sendData(string data);

        void sendResponse(const string &data);

        static uint8 computePacketChecksum(string_view data);

        void sendErrorResponse(int error) {
            sendResponse(format("E{:02x}", error));
        }

        void sendHaltResponse() {
            sendResponse(format("S{:02x}", SIGTRAP));
        }

        void handleCmdHaltReason(const vector<string> &args);
        void handleCmdReadReg(const vector<string> &args);
        void handleCmdWriteReg(const vector<string> &args);
        void handleCmdDetach(const vector<string> &args);
        void handleCmdReadMem(const vector<string> &args);
        void handleCmdWriteMem(const vector<string> &args);
        void handleCmdContinue(const vector<string> &args);
        void handleCmdContinueWithSignal(const vector<string> &args);
        void handleCmdStep(const vector<string> &args);
        void handleCmdStepWithSignal(const vector<string> &args);
        void handleCmdInsertBreakpoint(const vector<string> &args);
        void handleCmdRemoveBreakpoint(const vector<string> &args);
        void handleCmdSupported(const vector<string> &args);
        void handleCmdAttached(const vector<string> &args);

        const unordered_map<string, void(Debugger::*)(const vector<string> &args)> packetHandlers {
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
#if USE_CAPSTONE
        csh capstoneHandle{};
        cs_insn *disassembly{};
#endif
        int disassemblySize{};
        asio::io_context ioContext;
        asio::ip::tcp::acceptor tcpAcceptor {ioContext};
        asio::ip::tcp::socket tcpSocket {ioContext};
        array<char, MESSAGE_MAX_LENGTH> tcpReceiveBuffer{};
        string packetDecodeBuffer;
        queue<string> tcpTransmitBuffers;
        bool tcpClientConnected{};
        unordered_set<cref_t> breakpoints;
        bool halted{};
        bool singleStepping{};
        bool enabled{};
        unordered_map<cref_t, uint16> breakpointReplacements;
        bool wasAtBreakpoint{};
    };

}
