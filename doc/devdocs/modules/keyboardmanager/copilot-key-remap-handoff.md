# Copilot Key Remap – Handoff Summary

## Context

Branch: `copilot-key-remap`  
Repo: `C:\dev\github\GaTechThomas\PowerToys`  
Primary area: `src/modules/keyboardmanager/KeyboardManagerEngineLibrary`

Goal was to remap the hardware Copilot key (OS emits a `Win+Shift+F23`-style sequence) to `VK_RCONTROL` behavior without leaking `Win/Shift` side-effects.

---

## Current status (where we stopped)

Most behavior is working.

### Remaining known quirk

- `copilot + lshift + rightarrow` does **not** select word on first Shift press.
- If `Shift` is pressed **twice**, it works.

User accepted this temporary state for now.

---

## Validation scenario matrix

### Passing

1. `lshift + lctrl + rightarrow` -> select word  
2. `lshift + copilot + rightarrow` -> select word  
3. `rshift + lctrl + rightarrow` -> select word  
4. `rshift + copilot + rightarrow` -> select word  
5. `lctrl + lshift + rightarrow` -> select word  
7. `lctrl + rshift + rightarrow` -> select word  
8. `copilot + rshift + rightarrow` -> select word  
9. `a` -> `a`  
10. `lshift + a` -> `A`  
11. `rshift + a` -> `A`  
12. `copilot + rightarrow` -> move cursor one word right  
13. `lctrl + rightarrow` -> move cursor one word right

### Failing (known quirk)

6. `copilot + lshift + rightarrow` -> requires Shift press twice to work reliably

---

## Files touched during this effort

- `src/modules/keyboardmanager/KeyboardManagerEngineLibrary/State.h`
- `src/modules/keyboardmanager/KeyboardManagerEngineLibrary/State.cpp`
- `src/modules/keyboardmanager/KeyboardManagerEngineLibrary/KeyboardEventHandlers.cpp`
- `src/modules/keyboardmanager/common/KeyboardManagerConstants.h`
- `src/modules/keyboardmanager/KeyboardManagerEngineLibrary/KeyboardManager.cpp`
- `src/modules/keyboardmanager/KeyboardManagerEditorLibrary/BufferValidationHelpers.cpp`

(Actual final behavior is primarily in `State.cpp` + `KeyboardEventHandlers.cpp`.)

---

## Important constraints discovered

- Hot path behavior is very sensitive to timing/races.
- Key-up injections (especially Shift) can easily cancel a user’s first real Shift press.
- Suppression logic must carefully distinguish synthetic/non-physical events vs physical user input.
- Avoid assumptions about right-side modifiers unless proven by logs.
- Keep hook path quiet (minimal/no extra logging in steady state).

---

## What to check first when resuming

1. Re-run all 13 scenarios exactly as listed above.
2. Focus on scenario 6 immediately after scenario 12 (regressions often toggled between those).
3. Inspect current logic in:
   - `State::ShouldSuppressCopilotSequenceKey(...)`
   - `State::UpdateCopilotKeyState(...)`
   - Copilot-specific path in `HandleSingleKeyRemapEvent(...)`
4. Specifically watch for any path that can emit or effectively cause a synthetic Shift-up/Shift-down race around first `LShift` press.

---

## Suggested next technical direction (if picked up again)

Treat the Copilot remap path as deterministic and minimal:

- Prefer suppressing only synthetic/non-physical sequence side-events.
- Avoid runtime Shift key-up injections in the hook path unless absolutely required.
- Keep release-state cleanup robust, but avoid canceling physical modifier intent.
- If further debugging is needed, temporarily add targeted trace for scenario 6 only, then remove once fixed.

---

## User preference / practical stop point

User explicitly chose to stop at this state because everything is acceptable except scenario 6 requiring Shift twice.

This is the exact handoff state to resume from later.
