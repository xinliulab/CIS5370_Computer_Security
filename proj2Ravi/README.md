# Project 2: Design Your Own Game Modifier

## Overview
For this project, you will design your own game modifier targeting our simple game engine (`toybox.h`) and the simple Tetris game (`tetris.c`). This project offers two options, and you may choose to work on either one. (If you complete both projects under the specified conditions, you can earn extra credit as described below.)

**Deadline:** Tentatively set to March 4th.

---

## Option 1: Modify the Game Speed

**Objective:**  
Modify the game’s speed by altering its time data. For example, by intercepting and modifying the return value of `gettimeofday`, you can accelerate or decelerate the game’s progress.

**Background:**  
In class, we showed an example where GDB intercepts the `gettimeofday` call and modifies its return value to change the game’s progress. Please study how the GDB method works and then choose a different tool or method to achieve the same effect.

**Requirements:**  
- **Same Effect:** Your solution should intercept and modify the time value (or equivalent) in order to alter the game speed.
- **No Source Code Modification:** Do not change the source code of `toybox.h` or `tetris.c`.
- **Do Not Use GDB:** You must not use GDB.
- **Do Not Use a Third-Party libc Replacement:** Substituting `gettimeofday` via a custom libc is not allowed (we will cover libc attack techniques next week, but they cannot be used in this project).
- **Report Submission:** In your report, describe your method and its underlying principles, detail your code debugging process, and provide evidence of your runtime results.

---

## Option 2: Modify Game Memory Data

**Objective:**  
Design a game “cheat” tool that modifies in-game memory data in real time (for example, by changing the in-game money value).

**Background:**  
We provided a C++ source code example that modifies the money value in Red Alert 2. In the provided scenario, the game (“Red Alert 2”) runs on a Windows environment (prior to Windows 10) inside VirtualBox. The cheat tool works by first finding the VirtualBox process ID. It then searches through the process’s readable and writable memory regions to locate areas corresponding to the money value. After some gameplay (during which the money value changes), it filters the previously found addresses based on the new money value. Through iterative filtering, it becomes possible to pinpoint the memory locations that truly store the money value, which can then be modified.

**Requirements:**  
- **Open-Ended Implementation:** There are no strict requirements regarding the final effect. You may follow the example above or propose your own innovative approach.  
  *For example, you might design the tool so that if a discovered memory block does not match the expected block, the tool modifies the memory data to replace it with a matching block, or even removes the block altogether regardless of a match.*
- **Report Submission:** Your report must describe your method and its underlying principles, document your code debugging process, and include evidence of your solution in action (such as runtime results or screenshots).
- **Voting and Extra Credit:** We will vote to select the top three best game cheat tools designed under Option 2 on Mar 4th, awarding extra credit points of **5, 3, and 2** respectively.

---

## Extra Credit

If you complete **both** projects—and provided that your Option 2 solution does **not** use memory data modification to simultaneously change game progress (for example, if you do not affect game speed via memory modification) or if your Option 2 solution does not modify game progress at all—you will receive an additional **5 extra credit points**.

---

## Submission Requirements

For **both options**, you must submit a detailed report that includes:
- An explanation of your method and its underlying principles.
- A description of your code debugging process.
- Evidence of your solution in action (runtime results, screenshots, etc.).

---

Good luck, and be creative in your approach!
