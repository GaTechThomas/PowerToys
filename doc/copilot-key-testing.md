# Copilot Key Remap – Testing Checklist (Current)

This file replaces older UI-detection notes that referenced `KeyboardManagerState.cpp`.
The active runtime behavior is implemented in:

- `src/modules/keyboardmanager/KeyboardManagerEngineLibrary/State.cpp`
- `src/modules/keyboardmanager/KeyboardManagerEngineLibrary/KeyboardEventHandlers.cpp`

## Canonical Validation Scenarios

Use this list when validating Copilot remap behavior (left side is sequence):

1. `a` -> `a`
2. `lshift + a` -> `A`
3. `rshift + a` -> `A`
4. `lshift + lctrl + rightarrow` -> select word
5. `lshift + copilot + rightarrow` -> select word
6. `rshift + lctrl + rightarrow` -> select word
7. `rshift + copilot + rightarrow` -> select word
8. `lctrl + lshift + rightarrow` -> select word
9. `copilot + lshift + rightarrow` -> select word
10. `lctrl + rshift + rightarrow` -> select word
11. `copilot + rshift + rightarrow` -> select word
12. `copilot + rightarrow` -> move cursor one word right
13. `lctrl + rightarrow` -> move cursor one word right

## Notes

- Keep hot paths quiet (no persistent diagnostic logging in keyboard hook paths).
- If behavior changes, update this file and
  `doc/devdocs/modules/keyboardmanager/copilot-key-remap-handoff.md` together.
