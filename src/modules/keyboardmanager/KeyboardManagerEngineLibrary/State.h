#pragma once
#include <keyboardmanager/common/MappingConfiguration.h>
#include <mutex>
#include <chrono>

// Copilot key state machine states
enum class CopilotKeyState
{
    Idle,       // No Copilot sequence detected
    Detecting,  // Win or Shift seen (but not both yet)
    Active,     // Full Win+Shift+F23 sequence confirmed
    Releasing   // F23 released, waiting for Win/Shift to release
};

class State : public MappingConfiguration
{
private:
    // Stores the activated target application in app-specific shortcut
    std::wstring activatedAppSpecificShortcutTarget;

    // Copilot key state machine
    CopilotKeyState copilotState = CopilotKeyState::Idle;
    bool copilotWinSeen = false;
    bool copilotShiftSeen = false;
    bool copilotF23Seen = false;
    std::chrono::steady_clock::time_point copilotStateChangeTime;
    std::mutex copilotState_mutex;

public:
    // Function to get the iterator of a single key remap given the source key. Returns nullopt if it isn't remapped
    std::optional<SingleKeyRemapTable::iterator> GetSingleKeyRemap(const DWORD& originalKey);

    // Function to get a unicode string remap given the source key. Returns nullopt if it isn't remapped
    std::optional<std::wstring> GetSingleKeyToTextRemapEvent(const DWORD originalKey) const;

    bool CheckShortcutRemapInvoked(const std::optional<std::wstring>& appName);

    // Function to get the source and target of a shortcut remap given the source shortcut. Returns nullopt if it isn't remapped
    ShortcutRemapTable& GetShortcutRemapTable(const std::optional<std::wstring>& appName);

    std::vector<Shortcut>& GetSortedShortcutRemapVector(const std::optional<std::wstring>& appName);

    // Sets the activated target application in app-specific shortcut
    void SetActivatedApp(const std::wstring& appName);

    // Gets the activated target application in app-specific shortcut
    std::wstring GetActivatedApp();

    // Copilot key state machine methods
    void UpdateCopilotKeyState(DWORD vkCode, bool isKeyDown);
    void ResetCopilotKeyState();
    bool IsCopilotKeyActive();
    bool IsCopilotKeyReleasing();
    bool IsCopilotSequenceKey(DWORD vkCode, bool isKeyDown);
    bool ShouldSuppressCopilotSequenceKey(DWORD vkCode, bool isKeyDown, bool isInjected);
    void CleanupCopilotKeyStateOnStartup();
};