#include <motif/wm/SessionManager.h>
#include <motif/wm/WindowManager.h>

#include <X11/SM/SMlib.h>
#include <X11/ICE/ICElib.h>

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>

namespace motif::wm {

SessionManager::SessionManager(WindowManager& wm) : wm_(wm) {}

SessionManager::~SessionManager() {
    disconnect();
}

bool SessionManager::connect() {
    char errorBuf[256] = {};
    SmcCallbacks callbacks;

    callbacks.save_yourself.callback = saveYourselfCB;
    callbacks.save_yourself.client_data = this;
    callbacks.die.callback = dieCB;
    callbacks.die.client_data = this;
    callbacks.save_complete.callback = saveCompleteCB;
    callbacks.save_complete.client_data = this;
    callbacks.shutdown_cancelled.callback = shutdownCancelledCB;
    callbacks.shutdown_cancelled.client_data = this;

    // Try to get previous client ID
    char* prevId = nullptr;
    const char* envId = std::getenv("SESSION_MANAGER_CLIENT_ID");

    smConn_ = SmcOpenConnection(
        nullptr,           // network IDs (use SESSION_MANAGER env)
        nullptr,           // context
        SmProtoMajor,
        SmProtoMinor,
        SmcSaveYourselfProcMask | SmcDieProcMask |
        SmcSaveCompleteProcMask | SmcShutdownCancelledProcMask,
        &callbacks,
        prevId,
        &prevId,
        sizeof(errorBuf),
        errorBuf
    );

    if (!smConn_) {
        return false;
    }

    if (prevId) {
        clientId_ = prevId;
        free(prevId);
    }

    iceConn_ = SmcGetIceConnection(smConn_);
    setSmProperties();

    return true;
}

void SessionManager::disconnect() {
    if (smConn_) {
        SmcCloseConnection(smConn_, 0, nullptr);
        smConn_ = nullptr;
        iceConn_ = nullptr;
    }
}

int SessionManager::iceFd() const {
    if (!iceConn_) return -1;
    return IceConnectionNumber(iceConn_);
}

void SessionManager::processIceMessages() {
    if (!iceConn_) return;
    IceProcessMessages(iceConn_, nullptr, nullptr);
}

void SessionManager::setSmProperties() {
    if (!smConn_) return;

    // Program name
    SmPropValue progVal;
    const char* progName = "motif-wm";
    progVal.length = static_cast<int>(strlen(progName));
    progVal.value = const_cast<char*>(progName);

    SmProp progProp;
    progProp.name = const_cast<char*>(SmProgram);
    progProp.type = const_cast<char*>(SmARRAY8);
    progProp.num_vals = 1;
    progProp.vals = &progVal;

    // Restart command
    SmPropValue restartVal;
    restartVal.length = static_cast<int>(strlen(progName));
    restartVal.value = const_cast<char*>(progName);

    SmProp restartProp;
    restartProp.name = const_cast<char*>(SmRestartCommand);
    restartProp.type = const_cast<char*>(SmLISTofARRAY8);
    restartProp.num_vals = 1;
    restartProp.vals = &restartVal;

    // Clone command
    SmProp cloneProp;
    cloneProp.name = const_cast<char*>(SmCloneCommand);
    cloneProp.type = const_cast<char*>(SmLISTofARRAY8);
    cloneProp.num_vals = 1;
    cloneProp.vals = &restartVal;

    // User ID
    const char* user = std::getenv("USER");
    if (!user) user = "unknown";
    SmPropValue userVal;
    userVal.length = static_cast<int>(strlen(user));
    userVal.value = const_cast<char*>(user);

    SmProp userProp;
    userProp.name = const_cast<char*>(SmUserID);
    userProp.type = const_cast<char*>(SmARRAY8);
    userProp.num_vals = 1;
    userProp.vals = &userVal;

    // Restart style: RestartIfRunning
    char restartStyle = SmRestartIfRunning;
    SmPropValue styleVal;
    styleVal.length = 1;
    styleVal.value = &restartStyle;

    SmProp styleProp;
    styleProp.name = const_cast<char*>(SmRestartStyleHint);
    styleProp.type = const_cast<char*>(SmCARD8);
    styleProp.num_vals = 1;
    styleProp.vals = &styleVal;

    SmProp* props[] = {&progProp, &restartProp, &cloneProp, &userProp, &styleProp};
    SmcSetProperties(smConn_, 5, props);
}

// ── XSMP callbacks ──────────────────────────────────────────────

void SessionManager::saveYourselfCB(SmcConn conn, void* clientData,
                                      int saveType, bool shutdown,
                                      int interactStyle, bool fast) {
    auto* self = static_cast<SessionManager*>(clientData);
    self->handleSaveYourself(saveType, shutdown, interactStyle, fast);
}

void SessionManager::dieCB(SmcConn /*conn*/, void* clientData) {
    auto* self = static_cast<SessionManager*>(clientData);
    self->handleDie();
}

void SessionManager::saveCompleteCB(SmcConn /*conn*/, void* /*clientData*/) {
    // Nothing to do
}

void SessionManager::shutdownCancelledCB(SmcConn /*conn*/, void* /*clientData*/) {
    // Nothing to do
}

void SessionManager::handleSaveYourself(int /*saveType*/, bool /*shutdown*/,
                                          int /*interactStyle*/, bool /*fast*/) {
    // Save session state to file
    saveStateToFile();

    // Notify SM that save is complete
    SmcSaveYourselfDone(smConn_, True);
}

void SessionManager::handleDie() {
    disconnect();
    wm_.quit();
}

// ── Session state ───────────────────────────────────────────────

SessionManager::SessionState SessionManager::saveState() const {
    SessionState state;
    state.currentDesktop = wm_.currentDesktop();
    state.numDesktops = wm_.numDesktops();

    for (auto& [w, client] : wm_.clients()) {
        SessionState::ClientState cs;
        cs.wmClass = client->title; // simplified
        cs.desktop = client->desktop;
        cs.x = client->x;
        cs.y = client->y;
        cs.width = client->width;
        cs.height = client->height;
        cs.iconified = client->iconified;
        cs.maximized = client->maximized;
        state.clients.push_back(cs);
    }

    return state;
}

std::string SessionManager::defaultStateFile() {
    const char* home = std::getenv("HOME");
    if (home) return std::string(home) + "/.motif-wm-session";
    return "/tmp/.motif-wm-session";
}

bool SessionManager::saveStateToFile(const std::string& path) const {
    std::string filePath = path.empty() ? defaultStateFile() : path;
    std::ofstream file(filePath);
    if (!file.is_open()) return false;

    auto state = saveState();

    file << "# motif-wm session state\n";
    file << "numDesktops=" << state.numDesktops << "\n";
    file << "currentDesktop=" << state.currentDesktop << "\n";

    for (const auto& cs : state.clients) {
        file << "client {"
             << " class=" << cs.wmClass
             << " desktop=" << cs.desktop
             << " x=" << cs.x
             << " y=" << cs.y
             << " w=" << cs.width
             << " h=" << cs.height
             << " iconified=" << (cs.iconified ? 1 : 0)
             << " maximized=" << (cs.maximized ? 1 : 0)
             << " }\n";
    }

    return true;
}

bool SessionManager::loadStateFromFile(const std::string& path) {
    std::string filePath = path.empty() ? defaultStateFile() : path;
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    SessionState state;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line.find("numDesktops=") == 0) {
            state.numDesktops = std::stoi(line.substr(12));
        } else if (line.find("currentDesktop=") == 0) {
            state.currentDesktop = std::stoi(line.substr(15));
        }
        // Client restoration is handled by XSMP restart mechanism
    }

    restoreState(state);
    return true;
}

void SessionManager::restoreState(const SessionState& state) {
    wm_.setNumDesktops(state.numDesktops);
    wm_.switchDesktop(state.currentDesktop);
}

} // namespace motif::wm
