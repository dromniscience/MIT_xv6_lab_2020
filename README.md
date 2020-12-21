# RISC-V xv6 Lab Fall 2020

## Progress
- [x] [Lab: Xv6 and Unix utilities](https://pdos.csail.mit.edu/6.S081/2020/labs/util.html)
- [x] [Lab: System calls](https://pdos.csail.mit.edu/6.S081/2020/labs/syscall.html)
- [x] [Lab: Page tables](https://pdos.csail.mit.edu/6.S081/2020/labs/pgtbl.html)
- [x] [Lab: Traps](https://pdos.csail.mit.edu/6.S081/2020/labs/traps.html)
- [x] [Lab: Lazy page allocation](https://pdos.csail.mit.edu/6.S081/2020/labs/lazy.html)
- [x] [Lab: Copy-on-Write Fork for xv6](https://pdos.csail.mit.edu/6.S081/2020/labs/cow.html)
- [x] [Lab: Multithreading](https://pdos.csail.mit.edu/6.S081/2020/labs/thread.html)
- [x] [Lab: Locks](https://pdos.csail.mit.edu/6.S081/2020/labs/lock.html)
- [x] [Lab: File system](https://pdos.csail.mit.edu/6.S081/2020/labs/fs.html)
- [x] [Lab: Mmap](https://pdos.csail.mit.edu/6.S081/2020/labs/mmap.html)
- [ ] [Lab: Networking](https://pdos.csail.mit.edu/6.S081/2020/labs/net.html) (This one is not on my to-do list)

## Code
This repo contains the solution to each lab except for the last one, which I may decide to do in the future. Though these codes have passed the corresponding tests, subtle bugs are likely to present. Comment are added to facilitate reading and maintaining the code.
**If you are a student to whom these labs are assigned as exercise, please do NOT plagiarize.**
Also, in this branch, I displayed my lab reports which mainly record how I accomplished the lab, what schemes were adopted, why certain tricks had to be done in the way illustrated in the code and what could be further improved. Suggestions are welcome, and you could email your advice to me at the address listed in the front of each report.
**All reports are in Chinese.**

## Review
These labs vary quite a lot in terms of difficulty. Yet they have something in common: They are carefully and elegantly designed, catering to both interest and challenge.
The list of the difficulty in descending order from my perspective runs as follow, but I should declare in advance that "difficulty" within this context purely refers to how intricate and creative the schemes can be, regardless of their gross workload or the time spent understanding the supporting materials.

1. mmap (If you want to implement a high-efficient system)
2. COW (If you want to make it as robust as possible)
3. Page table (Be inventive and scrupulous)
4. Lazy allocation (Figure out all necessary spots that need modifying)
5. Traps (Think before you leap, with special regard to how sysreturn works)
6. File system (be familiar with interface provided by upper layers of xv6 file system)
7. Utils (Probably your first time to use Unix tools such as pipe)
8. Multithread (Do not panic, since it is done on your OS instead of xv6)
9. System calls (so long as you pay no heed to optional challenges :)

