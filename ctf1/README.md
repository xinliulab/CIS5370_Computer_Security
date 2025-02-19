# **README: CTF Part I - Solving CTF Problems Offline**  

## **Overview**  
This **CTF challenge** is designed to help you apply the **buffer overflow** and **stack exploitation** techniques learned in class to real-world security problems. You will solve challenges at [HackUCF CTF](https://ctf.hackucf.org/challenges) and submit your findings.  

## **What You Need to Do**  
1. **Choose a challenge** related to **buffer overflow** or **stack-based vulnerabilities** from [HackUCF CTF](https://ctf.hackucf.org/challenges).  
2. **Exploit the vulnerability** to retrieve the **flag**.  
3. **Submit your flag on the HackUCF CTF website** to verify your solution.  
4. **Write a report** explaining how you solved the challenge.  

## **What is a Flag?**  
A **flag** is a secret string (e.g., `CTF{example_flag}`) hidden in the challenge. Your goal is to **use exploitation techniques** to trigger a vulnerability and obtain the flag.  

Example:  
- Connecting to the challenge using `nc` (netcat):  
  ```bash
  nc ctf.hackucf.org 9000
  ```
- If your exploit works, the server will return a flag:  
  ```bash
  Here is your flag: CTF{example_flag}
  ```
- You must submit this flag on the **HackUCF CTF website** to get points.  

## **Submission Instructions**  
- Submit a **PDF write-up** via **GitHub Classroom**: [Assignment Link](https://classroom.github.com/a/6H7B2yRf).  
- Attach **any scripts or programs** you wrote to complete the challenge.  
- Your write-up must include:  
  - The **problem name** and its **point value** from the scoreboard.  
  - The **flag** you retrieved.  
  - A **detailed explanation** of how you solved it.  
  - **Screenshots** of your process, including exploitation and flag retrieval.  

## **Scoring System**  
- The **total score for this challenge is 30 points**.  
- Each **CTF point** earned on the scoreboard corresponds to **one point** for this challenge.  
- Example:  
  - If you solve **bof3** (worth **25 points** on the scoreboard), your score will be **25/30**.  
  - If you solve multiple challenges and earn more than **30 points**, your final score will still be capped at **30/30**.  

---

If you have any questions, feel free to ask.  

**Good luck, and happy hacking!** 🚀  


