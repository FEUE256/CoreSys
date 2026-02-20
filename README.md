# FÈUE CoreSys (CoreSystem)

**CoreSys** is a low-level operating system project designed for **UEFI-based systems**, written primarily in **C** and **TC**. The project explores modern firmware-native system design, minimal boot paths, and direct hardware interaction without relying on legacy BIOS layers or existing operating system kernels.

CoreSys is developed under **FÈUE (Förenta Evolutionernas Universitets Elektronik)** as a research and educational platform for experimental OS architecture.

THIS REPO IS IN ACTIVE DEVELOPMENT AND NOT YET FUNCTIONAL. EXPECT FREQUENT CHANGES AND BREAKAGES.

THIS REPO HAS SUBMODULES!

---

## Project Goals

CoreSys is built with the following long-term objectives:

- Create a **fully independent UEFI-native operating system**
- Maintain a **minimal and transparent architecture**
- Provide a **modular kernel structure** for experimentation
- Serve as a **learning platform** for low-level programming and firmware interaction
- Explore **new system design concepts** outside traditional OS constraints

This is not a Linux distribution or a fork of an existing kernel. CoreSys is built from scratch.

---

## Key Features (Planned & In Progress)

- UEFI bootloader written in C  
- Direct framebuffer graphics output  
- Memory management without legacy BIOS assumptions  
- Custom kernel core  
- Modular system services  
- Experimental driver model  
- Text-based system interface (early stages)

---

# How to Build CoreSys

## Prerequisites

Before building CoreSys, ensure you have the following installed:

- See in docs/req.md
- **Linux environment** (Ubuntu, WSL2, or similar)

---

## NOTICE
See more in the NOTICE.md

---

## Terminal

1. Open WSL or the Terminal
2. Change Folder
```bash
cd Path/to/CoreSys/build/master
```
3. Build
```bash
make all -B
```

## To clean

1. Open WSL or the Terminal
2. Change Folder
```bash
cd Path/to/CoreSys/build/master
```
3. Clean
```bash
make clean
```
 
Read more in the docs/ folder.
