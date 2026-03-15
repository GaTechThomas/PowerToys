# Build Guide (consolidated)

This repository already has canonical build guidance in:

- `AGENTS.md` (top-level contributor/build discipline)
- `tools/build/BUILD-GUIDELINES.md` (authoritative script usage)

Use those docs as source of truth.

## Quick commands

From repo root:

```powershell
# First-time setup
git submodule update --init --recursive

# Essentials restore/build
tools\build\build-essentials.cmd

# Build current folder/project context
tools\build\build.cmd

# Explicit script with options
powershell -ExecutionPolicy Bypass -File tools\build\build.ps1 -Platform x64 -Configuration Debug
```

## Why this file is short

Older content here duplicated and diverged from script behavior (for example,
references to `terminal.ps1` behavior and branch-specific notes). Keeping this
file lightweight prevents future drift.
