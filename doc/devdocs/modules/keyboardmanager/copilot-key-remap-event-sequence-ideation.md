# Copilot Key Remap – Sequence Ideation

## Background - Event Sequence

When the user presses the Copilot key, Windows generates a sequence of key events:

<br>

| Turn | Event | lwinIsDown | lshiftIsDown | copilotIsDown |
|---:|---|:---:|:---:|:---:|
| 0 | Initial (idle) | ⭕ | ⭕ | ⭕ |
| 1 | LWin Key Down | ✅ | ⭕ | ⭕ |
| 2 | LShift Key Down | ✅ | ✅ | ⭕ |
| 3 | F23 Key Down | ✅ | ✅ | ✅ |
| 4 | F23 Key Up | ✅ | ✅ | ⭕ |
| 5 | LShift Key Up | ✅ | ⭕ | ⭕ |
| 6 | LWin Key Up | ⭕ | ⭕ | ⭕ |

```mermaid
block
    columns 14

    ckd["Copilot Key Down"]:6
    space:1
    cku["Copilot Key Up"]:6
    space:1

    b1["LWin Key Down"] space b2["LShift Key Down"] space b3["F23 Key Down"]
    space:2
    c1["F23 Key Up"] space c2["LShift Key Up"] space c3["LWin Key Up"]
    space:2

    space lwinIsDown space lshiftIsDown space copilotIsDown
```

```mermaid
stateDiagram-v2
    direction LR
    [*] --> LWinIsDown: LWinDown
    LWinIsDown --> LShiftIsDown: LShiftDown
    LShiftIsDown --> CopilotIsDown: F23Down
    CopilotIsDown --> aaa
    aaa --> [*]
```




<br>
<br>
<br>

```mermaid
block
    columns 3

    ckd["Copilot Key Down"]:3
    b1["LWin Key Down"] b2["LShift Key Down"] b3["F23 Key Down"]
    space:3
    cku["Copilot Key Up"]:3
    c1["F23 Key Up"] c2["LShift Key Up"] c3["LWin Key Up"]

```

<br>
<br>
<br>
<br>
<br>




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

