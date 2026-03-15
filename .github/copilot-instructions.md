---
description: 'PowerToys AI contributor guidance'
---

# PowerToys – Copilot Instructions

Concise guidance for AI contributions. For complete details, see [AGENTS.md](../AGENTS.md).

## Key Rules

- Atomic PRs: one logical change, no drive-by refactors
- Add tests when changing behavior
- Keep hot paths quiet (no logging in hooks/tight loops)

## Copilot Key Remap Behavior

- Support long holds of the Copilot key without timing out or sticking modifiers when selecting text.
- Do not inject VK_RSHIFT key-up because RShift is not part of the Copilot hardware sequence (Win+Shift+F23).
- Validation scenarios for Copilot key remap:
  - `lshift + lctrl + rightarrow` -> select word
  - `lshift + copilot + rightarrow` -> select word
  - `rshift + lctrl + rightarrow` -> select word
  - `rshift + copilot + rightarrow` -> select word
  - `lctrl + lshift + rightarrow` -> select word
  - `copilot + lshift + rightarrow` -> select word
  - `lctrl + rshift + rightarrow` -> select word
  - `copilot + rshift + rightarrow` -> select word
  - `a` -> `a`
  - `lshift + a` -> `A`
  - `rshift + a` -> `A`
  - `copilot + rightarrow` -> move cursor one word right
  - `lctrl + rightarrow` -> move cursor one word right
- After each suggested change and after each code change, explicitly compare behavior against the full provided validation scenario list.

## Style Enforcement

- C#: `src/.editorconfig`, StyleCop.Analyzers
- C++: `src/.clang-format`
- XAML: XamlStyler

## When to Ask for Clarification

- Ambiguous spec after scanning docs
- Cross-module impact unclear
- Security, elevation, or installer changes

## Component-Specific Instructions

These are auto-applied based on file location:
- [Runner & Settings UI](.github/instructions/runner-settings-ui.instructions.md)
- [Common Libraries](.github/instructions/common-libraries.instructions.md)

## Detailed Documentation

- [Architecture](../doc/devdocs/core/architecture.md)
- [Coding Style](../doc/devdocs/development/style.md)