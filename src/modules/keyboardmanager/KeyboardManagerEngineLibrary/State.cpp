#include "pch.h"
#include "State.h"
#include <common/interop/shared_constants.h>
#include <keyboardmanager/common/KeyboardManagerConstants.h>
#include <optional>

// Function to get the iterator of a single key remap given the source key. Returns nullopt if it isn't remapped
std::optional<SingleKeyRemapTable::iterator> State::GetSingleKeyRemap(const DWORD& originalKey)
{
    auto it = singleKeyReMap.find(originalKey);
    if (it != singleKeyReMap.end())
    {
        return it;
    }

    return std::nullopt;
}

std::optional<std::wstring> State::GetSingleKeyToTextRemapEvent(const DWORD originalKey) const
{
    if (auto it = singleKeyToTextReMap.find(originalKey); it != end(singleKeyToTextReMap))
    {
        return std::get<std::wstring>(it->second);
    }
    else
    {
        return std::nullopt;
    }
}

bool State::CheckShortcutRemapInvoked(const std::optional<std::wstring>& appName)
{
    // Assumes appName exists in the app-specific remap table
    ShortcutRemapTable& currentRemapTable = appName ? appSpecificShortcutReMap[*appName] : osLevelShortcutReMap;
    for (auto& it : currentRemapTable)
    {
        if (it.second.isShortcutInvoked)
        {
            return true;
        }
    }

    return false;
}

// Function to get the source and target of a shortcut remap given the source shortcut. Returns nullopt if it isn't remapped
ShortcutRemapTable& State::GetShortcutRemapTable(const std::optional<std::wstring>& appName)
{
    if (appName)
    {
        auto itTable = appSpecificShortcutReMap.find(*appName);
        if (itTable != appSpecificShortcutReMap.end())
        {
            return itTable->second;
        }
    }

    return osLevelShortcutReMap;
}

std::vector<Shortcut>& State::GetSortedShortcutRemapVector(const std::optional<std::wstring>& appName)
{
    // Assumes appName exists in the app-specific remap table
    return appName ? appSpecificShortcutReMapSortedKeys[*appName] : osLevelShortcutReMapSortedKeys;
}

// Sets the activated target application in app-specific shortcut
void State::SetActivatedApp(const std::wstring& appName)
{
    activatedAppSpecificShortcutTarget = appName;
}

// Gets the activated target application in app-specific shortcut
std::wstring State::GetActivatedApp()
{
    return activatedAppSpecificShortcutTarget;
}

// Helper method to check if a key event is part of the Copilot sequence
bool State::IsCopilotSequenceKey(DWORD vkCode, bool isKeyDown)
{
    if (!isKeyDown)
    {
        return false;
    }

    if (vkCode == VK_F23)
    {
        return true;
    }

    if (vkCode != VK_LWIN && vkCode != VK_RWIN)
    {
        return false;
    }

    SHORT keyState = GetAsyncKeyState(vkCode);
    bool isPhysicallyPressed = (keyState & 0x8000) != 0;
    return !isPhysicallyPressed;
}

// Update the Copilot key state machine based on key events
void State::UpdateCopilotKeyState(DWORD vkCode, bool isKeyDown)
{
    std::lock_guard<std::mutex> lock(copilotState_mutex);

    // Safety: timeout only while detecting/releasing sequence. Active can be held for long periods.
    if (copilotState == CopilotKeyState::Detecting || copilotState == CopilotKeyState::Releasing)
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - copilotStateChangeTime).count();
        if (elapsed > KeyboardManagerConstants::CopilotKeySequenceTimeoutMs)
        {
            Logger::warn(L"Copilot state machine timeout after {}ms in state {}, force resetting", elapsed, static_cast<int>(copilotState));
            copilotState = CopilotKeyState::Idle;
            copilotWinSeen = false;
            copilotShiftSeen = false;
            copilotF23Seen = false;
        }
    }

    if (isKeyDown)
    {
        if (IsCopilotSequenceKey(vkCode, true))
        {
            if (vkCode == VK_LWIN || vkCode == VK_RWIN)
            {
                copilotWinSeen = true;
                if (copilotState == CopilotKeyState::Idle)
                {
                    copilotState = CopilotKeyState::Detecting;
                    copilotStateChangeTime = std::chrono::steady_clock::now();
                }
            }
            else if (vkCode == VK_F23)
            {
                // Safety: F23-down while already active means we missed F23-up, reset and start fresh
                if (copilotF23Seen && copilotState == CopilotKeyState::Active)
                {
                    Logger::warn(L"Duplicate F23-down detected, force resetting Copilot state");
                    copilotState = CopilotKeyState::Idle;
                    copilotWinSeen = false;
                    copilotShiftSeen = false;
                    copilotF23Seen = false;
                    return;
                }

                copilotF23Seen = true;
                if (copilotWinSeen)
                {
                    copilotState = CopilotKeyState::Active;
                    copilotStateChangeTime = std::chrono::steady_clock::now();
                }
            }
        }
    }
    else
    {
        // Safety: Key-up handling with robust cleanup
        if (vkCode == VK_F23 && copilotState == CopilotKeyState::Active)
        {
            copilotState = CopilotKeyState::Releasing;
            copilotStateChangeTime = std::chrono::steady_clock::now();
        }
        else if (vkCode == VK_F23 && (copilotState == CopilotKeyState::Detecting || copilotState == CopilotKeyState::Idle))
        {
            // F23-up without F23-down being tracked - just clean up
            copilotF23Seen = false;
        }
        else if ((vkCode == VK_LWIN || vkCode == VK_RWIN || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT) && copilotState == CopilotKeyState::Releasing)
        {
            if (vkCode == VK_LWIN || vkCode == VK_RWIN)
            {
                copilotWinSeen = false;
            }
            else if (vkCode == VK_LSHIFT || vkCode == VK_RSHIFT)
            {
                copilotShiftSeen = false;
            }

            if (!copilotWinSeen && !copilotShiftSeen)
            {
                copilotState = CopilotKeyState::Idle;
                copilotF23Seen = false;
            }
        }
        // Safety: Win/Shift-up while in Detecting state means sequence was abandoned
        else if ((vkCode == VK_LWIN || vkCode == VK_RWIN || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT) && copilotState == CopilotKeyState::Detecting)
        {
            Logger::trace(L"Copilot sequence abandoned during Detecting state (key-up 0x{:X}), resetting", vkCode);
            copilotState = CopilotKeyState::Idle;
            copilotWinSeen = false;
            copilotShiftSeen = false;
            copilotF23Seen = false;
        }
        // Safety: Unexpected key-up - clean up that key's flag to prevent stuck state
        else if (vkCode == VK_LWIN || vkCode == VK_RWIN)
        {
            copilotWinSeen = false;
        }
        else if (vkCode == VK_LSHIFT || vkCode == VK_RSHIFT)
        {
            copilotShiftSeen = false;
        }
    }
}

// Reset the Copilot key state machine
void State::ResetCopilotKeyState()
{
    std::lock_guard<std::mutex> lock(copilotState_mutex);
    copilotState = CopilotKeyState::Idle;
    copilotWinSeen = false;
    copilotShiftSeen = false;
    copilotF23Seen = false;
}

// Check if Copilot key is currently active
bool State::IsCopilotKeyActive()
{
    std::lock_guard<std::mutex> lock(copilotState_mutex);

    return copilotState == CopilotKeyState::Active;
}

// Check if Copilot key is in the releasing state (F23 was released, waiting for Win/Shift cleanup)
bool State::IsCopilotKeyReleasing()
{
    std::lock_guard<std::mutex> lock(copilotState_mutex);
    return copilotState == CopilotKeyState::Releasing;
}

// Safety method: Should we suppress a Copilot sequence key?
// Returns true only if the key is part of sequence AND we're confident it's safe to suppress
bool State::ShouldSuppressCopilotSequenceKey(DWORD vkCode, bool isKeyDown, bool isInjected)
{
    std::lock_guard<std::mutex> lock(copilotState_mutex);

    // Only consider Win/Shift sequence side-events.
    if (vkCode != VK_LWIN && vkCode != VK_RWIN && vkCode != VK_LSHIFT && vkCode != VK_RSHIFT)
    {
        return false;
    }

    // Only while Copilot sequence is in progress.
    if (copilotState != CopilotKeyState::Detecting && copilotState != CopilotKeyState::Active && copilotState != CopilotKeyState::Releasing)
    {
        return false;
    }

    // Do not suppress key-up events.
    if (!isKeyDown)
    {
        return false;
    }

    SHORT keyState = GetAsyncKeyState(vkCode);
    bool isPhysicallyPressed = (keyState & 0x8000) != 0;

    // During Detecting, suppress synthetic Copilot side-events.
    if (copilotState == CopilotKeyState::Detecting)
    {
        if (vkCode == VK_LSHIFT)
        {
            return isInjected || !isPhysicallyPressed;
        }

        return isInjected || !isPhysicallyPressed;
    }

    // During Active/Releasing, suppress Win side-effects and injected Shift side-events.
    if (copilotState == CopilotKeyState::Active || copilotState == CopilotKeyState::Releasing)
    {
        if (vkCode == VK_LWIN)
        {
            return isInjected || !isPhysicallyPressed;
        }

        if (vkCode == VK_LSHIFT)
        {
            return isInjected;
        }
    }

    return false;
}

// Cleanup method to run at startup - clears stuck keyboard state from previous crash
void State::CleanupCopilotKeyStateOnStartup()
{
    Logger::info(L"Performing Copilot key state cleanup on startup");

    // Reset our state machine
    ResetCopilotKeyState();

    // Send key-up events for common modifiers to clear any stuck state in Windows
    // Use raw SendInput to avoid going through our hooks
    std::vector<INPUT> cleanupEvents;

    const auto addKeyUp = [&cleanupEvents](WORD vk) {
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = vk;
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        input.ki.dwExtraInfo = CommonSharedConstants::KEYBOARDMANAGER_INJECTED_FLAG;
        cleanupEvents.push_back(input);
    };

    addKeyUp(VK_LWIN);
    addKeyUp(VK_RWIN);
    addKeyUp(VK_LSHIFT);
    addKeyUp(VK_RSHIFT);
    addKeyUp(VK_LCONTROL);
    addKeyUp(VK_RCONTROL);

    // Also send key-up for the Copilot remap target to unstick it if PowerToys crashed mid-sequence
    auto copilotRemap = GetSingleKeyRemap(CommonSharedConstants::VK_COPILOT);
    if (copilotRemap)
    {
        auto it = copilotRemap.value();
        if (it->second.index() == 0)
        {
            DWORD targetKey = std::get<DWORD>(it->second);
            if (targetKey != 0 && targetKey != CommonSharedConstants::VK_DISABLED)
            {
                Logger::info(L"Sending key-up for Copilot remap target 0x{:X} to clear stuck key state", targetKey);
                addKeyUp(static_cast<WORD>(targetKey));
            }
        }
    }
    
    UINT result = SendInput(static_cast<UINT>(cleanupEvents.size()), cleanupEvents.data(), sizeof(INPUT));
    Logger::info(L"Sent {} cleanup key-up events to clear stuck Windows keyboard state", result);
}
