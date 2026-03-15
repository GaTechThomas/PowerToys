# Copilot Key Remapping - Safety Measures

## Problem: Keyboard Lockup Risk

When remapping the Copilot key, we suppress the Win+Shift sequence to prevent Copilot from launching. However, if the state machine gets stuck (missed key events, crash, etc.), **all future Win/Shift presses get permanently suppressed**, locking the keyboard.

## Implemented Safety Measures

### 1. **500ms Timeout Protection**
- **Where**: `UpdateCopilotKeyState()`, `IsCopilotKeyActive()`, `ShouldSuppressCopilotSequenceKey()`
- **What**: If state machine stays in Detecting/Active/Releasing for >500ms, auto-reset to Idle
- **Why**: Copilot key sequence should complete in <100ms under normal conditions
- **Prevents**: Permanent lockup if F23 never arrives or other events are missed

### 2. **Duplicate Key Detection**
- **Where**: `UpdateCopilotKeyState()` keydown handling
- **What**: 
  - Win-down while `copilotWinSeen=true` → force reset
  - Shift-down while `copilotShiftSeen=true` → force reset
  - F23-down while `copilotF23Seen=true` and Active → force reset
- **Why**: Duplicate key-down means we missed the corresponding key-up
- **Prevents**: State machine getting stuck with incorrect tracking flags

### 3. **Unexpected Key Interruption**
- **Where**: `UpdateCopilotKeyState()` in Detecting state
- **What**: Any key other than Win/Shift/F23 pressed during Detecting → reset
- **Why**: User pressed other keys, Copilot sequence abandoned
- **Prevents**: False positives from incomplete sequences

### 4. **Abandoned Sequence Detection**
- **Where**: `UpdateCopilotKeyState()` keyup handling
- **What**: Win-up or Shift-up during Detecting state → reset
- **Why**: User released Win or Shift before completing sequence
- **Prevents**: Getting stuck in Detecting state indefinitely

### 5. **Physical Key Detection**
- **Where**: `ShouldSuppressCopilotSequenceKey()` and `IsCopilotSequenceKey()`
- **What**: Uses `GetAsyncKeyState()` to check if key is physically pressed
- **Result**: **Never suppresses physical keypresses** - only OS-generated Copilot sequence
- **Why**: User's actual Win/Shift keypresses must always work
- **Prevents**: Locking out Win and Shift keys entirely

### 6. **Unexpected Key-Up Cleanup**
- **Where**: `UpdateCopilotKeyState()` else branches
- **What**: If Win-up/Shift-up arrives unexpectedly, clear that specific flag
- **Why**: Handle out-of-order events gracefully
- **Prevents**: Stuck boolean flags causing state machine confusion

### 7. **Startup Cleanup**
- **Where**: `CleanupCopilotKeyStateOnStartup()` called in `StartLowlevelKeyboardHook()`
- **What**: 
  - Resets state machine to Idle
  - Sends Win-up and Shift-up to Windows to clear stuck state
- **Why**: Previous PowerToys crash may have left Win/Shift logically "pressed" in Windows
- **Prevents**: Keyboard remaining broken after restart

### 8. **Shutdown Cleanup**
- **Where**: `StopLowlevelKeyboardHook()`
- **What**: Calls cleanup before unhooking
- **Why**: Ensure clean keyboard state when PowerToys shuts down
- **Prevents**: Leaving Windows keyboard state corrupted

## How It Works

### Normal Copilot Key Press (with remap to Right Control):
```
1. Win-down (OS-generated) arrives
   → UpdateCopilotKeyState: Idle → Detecting
   → ShouldSuppressCopilotSequenceKey: true (OS-generated, Detecting, remap exists)
   → SUPPRESSED ✓

2. Shift-down (OS-generated) arrives  
   → UpdateCopilotKeyState: Still Detecting
   → ShouldSuppressCopilotSequenceKey: true
   → SUPPRESSED ✓

3. F23-down arrives (<100ms elapsed)
   → UpdateCopilotKeyState: Detecting → Active
   → Lookup VK_COPILOT remap → Found: Right Control
   → Inject Right Control key-down
   → REMAPPED ✓

4. F23-up arrives
   → Active → Releasing
   → Inject Right Control key-up

5. Win-up, Shift-up arrive
   → All flags cleared → Idle

Result: Right Control injected, Copilot never launched ✓
```

### Safety Scenario: User Presses Physical LShift+Arrow
```
1. LShift-down (PHYSICAL) arrives
   → GetAsyncKeyState(VK_LSHIFT) returns 0x8000 (physically pressed)
   → ShouldSuppressCopilotSequenceKey: false (physical key)
   → NOT SUPPRESSED - passes through ✓

2. Arrow-down arrives
   → Text selection works normally ✓
```

### Safety Scenario: F23 Never Arrives (Incomplete Sequence)
```
1. Win-down arrives → Detecting state (timestamp recorded)
2. Shift-down arrives → Still Detecting
3. User releases keys OR types other keys OR >500ms passes
   → Timeout or unexpected key detected
   → Force reset to Idle
   → All future Win/Shift work normally ✓
```

### Safety Scenario: PowerToys Crashes While Suppressing
```
On next PowerToys startup:
1. StartLowlevelKeyboardHook() called
2. CleanupCopilotKeyStateOnStartup() runs:
   - Resets state machine
   - Sends Win-up and Shift-up to Windows
3. Windows keyboard state cleared
4. Keyboard works normally ✓
```

## Testing Checklist

- [ ] Press Copilot key → remaps to Right Control, no Copilot launch
- [ ] Press LShift+arrows → selects text (not suppressed)
- [ ] Press Win (start Copilot sequence) then press 'A' → sequence abandoned, 'A' typed
- [ ] Press Win+Shift but no F23 → after 500ms, next Win/Shift work normally
- [ ] Kill PowerToys process mid-sequence → restart → keyboard works
- [ ] Rapidly press Copilot key multiple times → no stuck state
- [ ] Press physical Win+Shift+another key → works normally (not Copilot sequence)

## Known Limitations

1. **Only suppresses VK_LWIN and VK_LSHIFT** - The Copilot key sequence uses Left Win + Left Shift. Right Win and Right Shift always work normally.

2. **500ms timeout** - If Windows changes timing behavior, may need adjustment. Current value chosen because normal Copilot sequence completes in <100ms.

3. **Cleanup uses SendInput** - Relies on Windows accepting injected key-up events to clear state. Should work in all scenarios, but untestable without crashing PowerToys.

## Emergency Recovery (if keyboard still stuck)

If keyboard gets stuck despite safety measures:

1. **Task Manager** → End PowerToys processes
2. **Restart PowerToys** → Cleanup runs automatically
3. **If still stuck**: Restart Windows (clears all keyboard state)

## Future Enhancements (Optional)

- **Emergency escape hatch**: Press Escape 3x rapidly to force reset Copilot state
- **Broader cleanup**: On startup, send key-up for ALL modifier keys (Ctrl, Alt, Win, Shift)
- **Telemetry**: Log timeout/duplicate key events to identify patterns
- **Adaptive timeout**: Increase timeout if slow hardware detected
