#pragma once

#include <string>
#include <vector>
#include <functional>

// X Session Management Protocol (XSMP / ICE)
typedef struct _SmcConn* SmcConn;
typedef struct _IceConn* IceConn;

namespace motif::wm {

class WindowManager;

/// X Session Management Protocol (XSMP) client.
/// Allows the window manager to participate in session save/restore,
/// shutdown, and checkpoint operations.
class SessionManager {
public:
    explicit SessionManager(WindowManager& wm);
    ~SessionManager();

    /// Connect to the session manager. Returns false if no SM is available.
    bool connect();

    /// Disconnect from the session manager.
    void disconnect();

    bool isConnected() const { return smConn_ != nullptr; }

    /// Get the ICE file descriptor for select() integration
    int iceFd() const;

    /// Process ICE messages (call when iceFd is readable)
    void processIceMessages();

    /// Session state that can be saved/restored
    struct SessionState {
        struct ClientState {
            std::string wmClass;
            std::string wmCommand;
            int desktop = 0;
            int x = 0, y = 0;
            int width = 0, height = 0;
            bool iconified = false;
            bool maximized = false;
        };

        int currentDesktop = 0;
        int numDesktops = 4;
        std::vector<ClientState> clients;
    };

    /// Save current session state
    SessionState saveState() const;

    /// Restore session state (call after initialize)
    void restoreState(const SessionState& state);

    /// Save state to file (~/.motif-wm-session)
    bool saveStateToFile(const std::string& path = {}) const;

    /// Load state from file
    bool loadStateFromFile(const std::string& path = {});

private:
    // XSMP callbacks
    static void saveYourselfCB(SmcConn conn, void* clientData,
                                int saveType, bool shutdown,
                                int interactStyle, bool fast);
    static void dieCB(SmcConn conn, void* clientData);
    static void saveCompleteCB(SmcConn conn, void* clientData);
    static void shutdownCancelledCB(SmcConn conn, void* clientData);

    void handleSaveYourself(int saveType, bool shutdown, int interactStyle, bool fast);
    void handleDie();

    void setSmProperties();
    static std::string defaultStateFile();

    WindowManager& wm_;
    SmcConn smConn_ = nullptr;
    IceConn iceConn_ = nullptr;
    std::string clientId_;
};

} // namespace motif::wm
