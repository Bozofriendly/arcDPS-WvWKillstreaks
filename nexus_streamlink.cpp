///----------------------------------------------------------------------------------------------------
/// WvW Killstreak Tracker - Nexus Addon
///
/// Tracks personal kills in WvW and writes the killstreak count to a file for OBS integration.
///----------------------------------------------------------------------------------------------------

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <atomic>
#include <mutex>
#include <string>
#include <unordered_set>

#include "imgui/imgui.h"

#include "Nexus.h"
#include "ArcDPS.h"
#include "UnofficialExtras.h"

// Plugin info
#define ADDON_NAME "Nexus Streamlink"
// Negative signature for non-Raidcore hosted addons (cast to uint32_t)
#define ADDON_SIGNATURE static_cast<uint32_t>(-0xB020F1)

// MumbleLink structures for reading GW2 game state
struct MumbleContext
{
    unsigned char serverAddress[28];
    uint32_t      mapId;
    uint32_t      mapType;
    uint32_t      shardId;
    uint32_t      instance;
    uint32_t      buildId;
};

struct LinkedMem
{
    uint32_t uiVersion;
    uint32_t uiTick;
    float    fAvatarPosition[3];
    float    fAvatarFront[3];
    float    fAvatarTop[3];
    wchar_t  name[256];
    float    fCameraPosition[3];
    float    fCameraFront[3];
    float    fCameraTop[3];
    wchar_t  identity[256];
    uint32_t context_len;
    unsigned char context[256];
};

// State
static std::atomic<uint32_t> g_killCount{0};
static std::atomic<bool> g_inSquad{false};
static std::mutex g_fileMutex;
static std::mutex g_squadMutex;
static uintptr_t g_selfId = 0;
static std::unordered_set<std::string> g_squadMembers;
static const char* g_playerStatus = "alive";

// MumbleLink
static HANDLE g_mumbleHandle = nullptr;
static LinkedMem* g_mumbleLink = nullptr;

// Nexus API
static AddonAPI* g_api = nullptr;
static HMODULE g_hModule = nullptr;
static AddonDefinition g_addonDef = {};

// Settings
static char g_outputPath[512] = "addons/streamlink/killstreak.txt";
static char g_squadOutputPath[512] = "addons/streamlink/squad.txt";
static char g_playerStatusPath[512] = "addons/streamlink/playerstatus.txt";
static char g_settingsPath[512] = "";

// Forward declarations
static void AddonLoad(AddonAPI* aAPI);
static void AddonUnload();
static void AddonOptions();
static void OnCombatEvent(void* eventArgs);
static void OnSquadCombatEvent(void* eventArgs);
static void OnSquadUpdate(void* eventArgs);
static void WriteKillcountToFile();
static void WriteSquadStatusToFile();
static void WritePlayerStatusToFile();
static void LoadSettings();
static void SaveSettings();
static std::string ResolvePath(const char* aPath);

///----------------------------------------------------------------------------------------------------
/// IsInWvW - Check if player is in WvW via MumbleLink shared memory
///----------------------------------------------------------------------------------------------------
static bool IsInWvW()
{
    if (!g_mumbleLink || g_mumbleLink->uiTick == 0)
        return false;

    const MumbleContext* ctx = reinterpret_cast<const MumbleContext*>(g_mumbleLink->context);
    // WvW mapType values: 9=EB, 10=Blue BL, 11=Green BL, 12=Red BL, 14=Obsidian Sanctum, 15=EotM
    switch (ctx->mapType)
    {
        case 9:
        case 10:
        case 11:
        case 12:
        case 14:
        case 15:
            return true;
        default:
            break;
    }

    // Armistice Bastion (WvW lounge) has a different mapType but is WvW-adjacent
    if (ctx->mapId == 1315)
        return true;

    return false;
}

///----------------------------------------------------------------------------------------------------
/// DllMain
///----------------------------------------------------------------------------------------------------
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    (void)lpReserved;  // Unused parameter
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            g_hModule = hModule;
            DisableThreadLibraryCalls(hModule);
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

///----------------------------------------------------------------------------------------------------
/// GetAddonDef - Nexus entry point
///----------------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) AddonDefinition* GetAddonDef()
{
    g_addonDef.Signature = ADDON_SIGNATURE;
    g_addonDef.APIVersion = NEXUS_API_VERSION;
    g_addonDef.Name = ADDON_NAME;
    g_addonDef.Version.Major = 2;
    g_addonDef.Version.Minor = 6;
    g_addonDef.Version.Build = 0;
    g_addonDef.Version.Revision = 0;
    g_addonDef.Author = "Bozo";
    g_addonDef.Description = "Tracks WvW killstreaks and writes to file for OBS integration.";
    g_addonDef.Load = AddonLoad;
    g_addonDef.Unload = AddonUnload;
    g_addonDef.Flags = EAddonFlags_None;
    g_addonDef.Provider = EUpdateProvider_GitHub;
    g_addonDef.UpdateLink = "https://github.com/Bozofriendly/nexus-streamlink";

    return &g_addonDef;
}

///----------------------------------------------------------------------------------------------------
/// IsAbsolutePath - True for drive-letter (C:\...), UNC (\\server\...) and root-relative paths
///----------------------------------------------------------------------------------------------------
static bool IsAbsolutePath(const char* aPath)
{
    if (!aPath || !aPath[0]) return false;
    if (aPath[0] == '\\' || aPath[0] == '/') return true;
    bool isLetter = (aPath[0] >= 'A' && aPath[0] <= 'Z') || (aPath[0] >= 'a' && aPath[0] <= 'z');
    return isLetter && aPath[1] == ':';
}

///----------------------------------------------------------------------------------------------------
/// ResolvePath - Returns aPath as-is if absolute, otherwise resolves against the game directory
///----------------------------------------------------------------------------------------------------
static std::string ResolvePath(const char* aPath)
{
    if (IsAbsolutePath(aPath) || !g_api) return aPath;

    const char* gameDir = g_api->Paths_GetGameDirectory();
    if (!gameDir) return aPath;

    std::string fullPath = gameDir;
    if (!fullPath.empty() && fullPath.back() != '\\' && fullPath.back() != '/')
    {
        fullPath += "\\";
    }
    fullPath += aPath;
    return fullPath;
}

///----------------------------------------------------------------------------------------------------
/// GetSettingsPath - Returns the path to the settings file
///----------------------------------------------------------------------------------------------------
static std::string GetSettingsPath()
{
    if (!g_api) return "";

    const char* addonDir = g_api->Paths_GetAddonDirectory("streamlink");
    if (!addonDir) return "";

    std::string path = addonDir;
    if (!path.empty() && path.back() != '\\' && path.back() != '/')
    {
        path += "\\";
    }
    path += "settings.txt";
    return path;
}

///----------------------------------------------------------------------------------------------------
/// LoadSettings - Load settings from file (key=value lines; a bare path is the legacy format)
///----------------------------------------------------------------------------------------------------
static void LoadSettings()
{
    std::string path = GetSettingsPath();
    if (path.empty()) return;

    FILE* f = nullptr;
    if (fopen_s(&f, path.c_str(), "r") != 0 || !f) return;

    char buffer[600];
    while (fgets(buffer, sizeof(buffer), f))
    {
        // Trim trailing newline/carriage return
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r'))
            buffer[--len] = '\0';
        if (len == 0) continue;

        char* eq = strchr(buffer, '=');
        if (!eq)
        {
            // Legacy format: the whole line is the killstreak output path
            strncpy_s(g_outputPath, buffer, sizeof(g_outputPath) - 1);
            continue;
        }

        *eq = '\0';
        const char* key = buffer;
        const char* value = eq + 1;
        if (*value == '\0') continue;

        if (strcmp(key, "killstreak_path") == 0)
            strncpy_s(g_outputPath, value, sizeof(g_outputPath) - 1);
        else if (strcmp(key, "squad_path") == 0)
            strncpy_s(g_squadOutputPath, value, sizeof(g_squadOutputPath) - 1);
        else if (strcmp(key, "playerstatus_path") == 0)
            strncpy_s(g_playerStatusPath, value, sizeof(g_playerStatusPath) - 1);
    }
    fclose(f);
}

///----------------------------------------------------------------------------------------------------
/// SaveSettings - Write settings to file
///----------------------------------------------------------------------------------------------------
static void SaveSettings()
{
    std::string path = GetSettingsPath();
    if (path.empty()) return;

    std::lock_guard<std::mutex> lock(g_fileMutex);

    std::string dirPath = path.substr(0, path.find_last_of("\\/"));
    CreateDirectoryA(dirPath.c_str(), nullptr);

    FILE* f = nullptr;
    if (fopen_s(&f, path.c_str(), "w") == 0 && f)
    {
        fprintf(f, "killstreak_path=%s\n", g_outputPath);
        fprintf(f, "squad_path=%s\n", g_squadOutputPath);
        fprintf(f, "playerstatus_path=%s\n", g_playerStatusPath);
        fclose(f);
    }
}

///----------------------------------------------------------------------------------------------------
/// WriteKillcountToFile - Write current killstreak to output file
///----------------------------------------------------------------------------------------------------
static void WriteKillcountToFile()
{
    std::lock_guard<std::mutex> lock(g_fileMutex);

    std::string fullPath = ResolvePath(g_outputPath);

    // Ensure directory exists
    std::string dirPath = fullPath.substr(0, fullPath.find_last_of("\\/"));
    CreateDirectoryA(dirPath.c_str(), nullptr);

    FILE* f = nullptr;
    if (fopen_s(&f, fullPath.c_str(), "w") == 0 && f)
    {
        fprintf(f, "%u", g_killCount.load());
        fclose(f);
    }
}

///----------------------------------------------------------------------------------------------------
/// WriteSquadStatusToFile - Write current squad status (0 or 1) to output file
///----------------------------------------------------------------------------------------------------
static void WriteSquadStatusToFile()
{
    std::lock_guard<std::mutex> lock(g_fileMutex);

    std::string fullPath = ResolvePath(g_squadOutputPath);

    // Ensure directory exists
    std::string dirPath = fullPath.substr(0, fullPath.find_last_of("\\/"));
    CreateDirectoryA(dirPath.c_str(), nullptr);

    FILE* f = nullptr;
    if (fopen_s(&f, fullPath.c_str(), "w") == 0 && f)
    {
        fprintf(f, "%u", g_inSquad.load() ? 1 : 0);
        fclose(f);
    }
}

///----------------------------------------------------------------------------------------------------
/// WritePlayerStatusToFile - Write current player status (alive/downed/dead) to output file
///----------------------------------------------------------------------------------------------------
static void WritePlayerStatusToFile()
{
    std::lock_guard<std::mutex> lock(g_fileMutex);

    std::string fullPath = ResolvePath(g_playerStatusPath);

    // Ensure directory exists
    std::string dirPath = fullPath.substr(0, fullPath.find_last_of("\\/"));
    CreateDirectoryA(dirPath.c_str(), nullptr);

    FILE* f = nullptr;
    if (fopen_s(&f, fullPath.c_str(), "w") == 0 && f)
    {
        fprintf(f, "%s", g_playerStatus);
        fclose(f);
    }
}

///----------------------------------------------------------------------------------------------------
/// OnSquadUpdate - Handle Unofficial Extras squad update events via Nexus
///----------------------------------------------------------------------------------------------------
static void OnSquadUpdate(void* eventArgs)
{
    if (!eventArgs) return;

    EvSquadUpdate* data = static_cast<EvSquadUpdate*>(eventArgs);
    if (!data->UpdatedUsers || data->UpdatedUsersCount == 0) return;

    std::lock_guard<std::mutex> lock(g_squadMutex);

    for (uint64_t i = 0; i < data->UpdatedUsersCount; i++)
    {
        const UnofficialExtras::UserInfo& user = data->UpdatedUsers[i];
        if (!user.AccountName) continue;

        std::string accountName(user.AccountName);

        if (user.Role != UnofficialExtras::UserRole::None &&
            user.Role != UnofficialExtras::UserRole::Invalid)
        {
            g_squadMembers.insert(accountName);
        }
        else
        {
            g_squadMembers.erase(accountName);
        }
    }

    bool wasInSquad = g_inSquad.load();
    bool nowInSquad = !g_squadMembers.empty();

    if (wasInSquad != nowInSquad)
    {
        g_inSquad.store(nowInSquad);
        WriteSquadStatusToFile();
    }
}

///----------------------------------------------------------------------------------------------------
/// OnCombatEvent - Handle ArcDPS combat events via Nexus
///----------------------------------------------------------------------------------------------------
static void OnCombatEvent(void* eventArgs)
{
    if (!eventArgs) return;

    EvCombatData* data = static_cast<EvCombatData*>(eventArgs);
    ArcDPS::CombatEvent* ev = data->ev;
    ArcDPS::AgentShort* src = data->src;
    ArcDPS::AgentShort* dst = data->dst;

    // Handle null event (agent tracking)
    if (!ev)
    {
        if (src && src->IsSelf)
        {
            g_selfId = src->ID;
        }
        return;
    }

    // State change events don't come through LOCAL_RAW, handled in OnSquadCombatEvent
    if (ev->IsStatechange)
        return;

    // Check for killing blow (WvW only via MumbleLink)
    if (ev->Result == ArcDPS::CBTR_KILLINGBLOW)
    {
        if (g_api && g_mumbleLink)
        {
            const MumbleContext* ctx = reinterpret_cast<const MumbleContext*>(g_mumbleLink->context);
            char logMsg[128];
            snprintf(logMsg, sizeof(logMsg), "KILLINGBLOW: mapType=%u, mapId=%u, isWvW=%s",
                     ctx->mapType, ctx->mapId, IsInWvW() ? "true" : "false");
            g_api->Log(ELogLevel_DEBUG, ADDON_NAME, logMsg);
        }
    }
    if (ev->Result == ArcDPS::CBTR_KILLINGBLOW && IsInWvW())
    {
        // Check if WE dealt the killing blow
        bool isSelfKill = false;
        if (src)
        {
            if (src->IsSelf)
            {
                isSelfKill = true;
            }
            else if (g_selfId != 0 && src->ID == g_selfId)
            {
                isSelfKill = true;
            }
        }

        if (isSelfKill)
        {
            // Only count kills against enemy players, not NPCs
            // In arcDPS, Profession is 1-9 for players, species ID (>9) for NPCs
            bool dstIsPlayer = (dst && dst->Profession >= 1 && dst->Profession <= 9);
            if (dstIsPlayer)
            {
                uint32_t newCount = g_killCount.fetch_add(1) + 1;
                WriteKillcountToFile();

                // Send alert for milestones
                if (g_api && (newCount == 5 || newCount == 10 || newCount == 25 || newCount == 50 || newCount == 100))
                {
                    char alertMsg[64];
                    snprintf(alertMsg, sizeof(alertMsg), "Killstreak: %u!", newCount);
                    g_api->GUI_SendAlert(alertMsg);
                }
            }
        }

        // Check if WE were killed (we are the target of a killing blow)
        // Note: Stomp deaths don't trigger KILLINGBLOW, only direct deaths do
        bool isSelfDeath = false;
        if (dst)
        {
            if (dst->IsSelf)
            {
                isSelfDeath = true;
            }
            else if (g_selfId != 0 && dst->ID == g_selfId)
            {
                isSelfDeath = true;
            }
        }

        if (isSelfDeath)
        {
            g_killCount.store(0);
            WriteKillcountToFile();
        }
    }
}

///----------------------------------------------------------------------------------------------------
/// OnSquadCombatEvent - Handle ArcDPS squad combat events (state changes come through here)
///----------------------------------------------------------------------------------------------------
static void OnSquadCombatEvent(void* eventArgs)
{
    if (!eventArgs) return;

    EvCombatData* data = static_cast<EvCombatData*>(eventArgs);
    ArcDPS::CombatEvent* ev = data->ev;
    ArcDPS::AgentShort* src = data->src;

    if (!ev)
    {
        if (g_api)
            g_api->Log(ELogLevel_DEBUG, ADDON_NAME, "SQUAD_RAW: null ev (agent tracking)");
        return;
    }

    if (g_api)
    {
        char logMsg[128];
        snprintf(logMsg, sizeof(logMsg), "SQUAD_RAW: statechange=%u result=%u",
                 (unsigned)ev->IsStatechange, (unsigned)ev->Result);
        g_api->Log(ELogLevel_DEBUG, ADDON_NAME, logMsg);
    }

    // Only handle state changes here
    if (!ev->IsStatechange) return;

    switch (ev->IsStatechange)
    {
        case ArcDPS::CBTS_CHANGEUP:
        case ArcDPS::CBTS_CHANGEDEAD:
        case ArcDPS::CBTS_CHANGEDOWN:
        {
            bool isSelf = src && (src->IsSelf || (g_selfId != 0 && src->ID == g_selfId));
            if (!isSelf) break;

            if (ev->IsStatechange == ArcDPS::CBTS_CHANGEUP)
                g_playerStatus = "alive";
            else if (ev->IsStatechange == ArcDPS::CBTS_CHANGEDOWN)
                g_playerStatus = "downed";
            else if (ev->IsStatechange == ArcDPS::CBTS_CHANGEDEAD)
            {
                g_playerStatus = "dead";
                if (IsInWvW())
                {
                    g_killCount.store(0);
                    WriteKillcountToFile();
                }
            }
            WritePlayerStatusToFile();

            if (g_api)
            {
                char logMsg[128];
                snprintf(logMsg, sizeof(logMsg), "Player status changed to: %s", g_playerStatus);
                g_api->Log(ELogLevel_INFO, ADDON_NAME, logMsg);
            }
            break;
        }
    }
}

///----------------------------------------------------------------------------------------------------
/// AddonOptions - Render settings UI in the Nexus options window
///----------------------------------------------------------------------------------------------------
static void AddonOptions()
{
    ImGui::TextDisabled("Status");
    ImGui::Separator();
    ImGui::Text("Current killstreak: %u", g_killCount.load());
    ImGui::Text("In squad: %s", g_inSquad.load() ? "yes" : "no");
    ImGui::Text("Player status: %s", g_playerStatus);
    ImGui::Text("In WvW: %s", IsInWvW() ? "yes" : "no");

    bool resetClicked = ImGui::Button("Reset Killstreak");

    ImGui::Spacing();
    ImGui::TextDisabled("Output Files");
    ImGui::Separator();
    ImGui::TextWrapped("Full paths to the output files (e.g. C:\\stream\\killstreak.txt). "
                       "Changes are saved when a field loses focus.");

    bool pathsEdited = false;
    {
        // Hold the file mutex while editing so writers never see a half-typed path
        std::lock_guard<std::mutex> lock(g_fileMutex);

        ImGui::InputText("Killstreak file", g_outputPath, sizeof(g_outputPath));
        pathsEdited |= ImGui::IsItemDeactivatedAfterEdit();

        ImGui::InputText("Squad status file", g_squadOutputPath, sizeof(g_squadOutputPath));
        pathsEdited |= ImGui::IsItemDeactivatedAfterEdit();

        ImGui::InputText("Player status file", g_playerStatusPath, sizeof(g_playerStatusPath));
        pathsEdited |= ImGui::IsItemDeactivatedAfterEdit();
    }

    // Act outside the locked scope: these helpers take g_fileMutex themselves
    if (resetClicked)
    {
        g_killCount.store(0);
        WriteKillcountToFile();
    }

    if (pathsEdited)
    {
        SaveSettings();
        WriteKillcountToFile();
        WriteSquadStatusToFile();
        WritePlayerStatusToFile();
    }
}

///----------------------------------------------------------------------------------------------------
/// AddonLoad - Called when addon is loaded
///----------------------------------------------------------------------------------------------------
static void AddonLoad(AddonAPI* aAPI)
{
    g_api = aAPI;

    // Bind to the ImGui context provided by Nexus so our UI renders in its frame
    ImGui::SetCurrentContext(static_cast<ImGuiContext*>(aAPI->ImguiContext));
    ImGui::SetAllocatorFunctions(
        reinterpret_cast<void* (*)(size_t, void*)>(aAPI->ImguiMalloc),
        reinterpret_cast<void (*)(void*, void*)>(aAPI->ImguiFree));

    // Open MumbleLink shared memory for WvW detection
    g_mumbleHandle = OpenFileMappingW(FILE_MAP_READ, FALSE, L"MumbleLink");
    if (g_mumbleHandle)
    {
        g_mumbleLink = static_cast<LinkedMem*>(MapViewOfFile(g_mumbleHandle, FILE_MAP_READ, 0, 0, sizeof(LinkedMem)));
        if (g_mumbleLink)
        {
            const MumbleContext* ctx = reinterpret_cast<const MumbleContext*>(g_mumbleLink->context);
            char logMsg[128];
            snprintf(logMsg, sizeof(logMsg), "MumbleLink connected. mapType=%u, mapId=%u, isWvW=%s",
                     ctx->mapType, ctx->mapId, IsInWvW() ? "true" : "false");
            aAPI->Log(ELogLevel_INFO, ADDON_NAME, logMsg);
        }
        else
        {
            CloseHandle(g_mumbleHandle);
            g_mumbleHandle = nullptr;
            aAPI->Log(ELogLevel_WARNING, ADDON_NAME, "MumbleLink: MapViewOfFile failed.");
        }
    }
    else
    {
        aAPI->Log(ELogLevel_WARNING, ADDON_NAME, "MumbleLink: shared memory not found.");
    }

    // Load settings, then expand any relative paths (defaults or legacy settings) to full paths
    LoadSettings();
    strncpy_s(g_outputPath, ResolvePath(g_outputPath).c_str(), sizeof(g_outputPath) - 1);
    strncpy_s(g_squadOutputPath, ResolvePath(g_squadOutputPath).c_str(), sizeof(g_squadOutputPath) - 1);
    strncpy_s(g_playerStatusPath, ResolvePath(g_playerStatusPath).c_str(), sizeof(g_playerStatusPath) - 1);

    // Subscribe to ArcDPS combat events
    aAPI->Events_Subscribe(EV_ARCDPS_COMBATEVENT_LOCAL_RAW, OnCombatEvent);
    aAPI->Events_Subscribe(EV_ARCDPS_COMBATEVENT_SQUAD_RAW, OnSquadCombatEvent);

    // Subscribe to Unofficial Extras squad events (requires ArcdpsIntegration addon)
    aAPI->Events_Subscribe(EV_UNOFFICIAL_EXTRAS_SQUAD_UPDATE, OnSquadUpdate);

    // Register the options panel shown in the Nexus addon settings
    aAPI->GUI_Register(RT_OptionsRender, AddonOptions);

    // Initialize output files
    g_killCount.store(0);
    g_inSquad.store(false);
    g_playerStatus = "alive";
    WriteKillcountToFile();
    WriteSquadStatusToFile();
    WritePlayerStatusToFile();

    aAPI->Log(ELogLevel_INFO, ADDON_NAME, "Addon loaded successfully.");
}

///----------------------------------------------------------------------------------------------------
/// AddonUnload - Called when addon is unloaded
///----------------------------------------------------------------------------------------------------
static void AddonUnload()
{
    if (g_api)
    {
        g_api->GUI_Deregister(AddonOptions);

        // Unsubscribe from events
        g_api->Events_Unsubscribe(EV_ARCDPS_COMBATEVENT_LOCAL_RAW, OnCombatEvent);
        g_api->Events_Unsubscribe(EV_ARCDPS_COMBATEVENT_SQUAD_RAW, OnSquadCombatEvent);
        g_api->Events_Unsubscribe(EV_UNOFFICIAL_EXTRAS_SQUAD_UPDATE, OnSquadUpdate);

        g_api->Log(ELogLevel_INFO, ADDON_NAME, "Addon unloaded.");
    }

    // Clean up MumbleLink
    if (g_mumbleLink)
    {
        UnmapViewOfFile(g_mumbleLink);
        g_mumbleLink = nullptr;
    }
    if (g_mumbleHandle)
    {
        CloseHandle(g_mumbleHandle);
        g_mumbleHandle = nullptr;
    }

    // Final file writes
    WriteKillcountToFile();
    WriteSquadStatusToFile();
    WritePlayerStatusToFile();
    SaveSettings();

    // Clear squad members
    {
        std::lock_guard<std::mutex> lock(g_squadMutex);
        g_squadMembers.clear();
    }

    g_api = nullptr;
}
