# CoreSys OS Edition — User Documentation

This document explains how to **use the CoreSys OS Edition website**, how to navigate it, and what each section is for.

---

## 1. What this page is

This page is the **official CoreSys OS landing interface**.  
It is not the operating system itself — it is the **control + documentation portal** for:

- Understanding CoreSys architecture
- Downloading system images
- Reading technical documentation
- Exploring kernel and boot design

---

## 2. How to use the navigation

At the top of the page you will see:

### Navigation bar

Chapter I -> IV
Download 
Docs
Legal


### How it works:

- Clicking a chapter scrolls to that section
- No page reload is required
- Everything is a single static page

---

## 3. Font selector (UI control)

There is a font dropdown:

### How to use it:

1. Click the dropdown
2. Choose a font:
   - Audiowide (futuristic UI style)
   - Orbitron (technical dashboard style)
   - Mono (default system style)

### What it does:

It changes **only the text rendering style** on the page.  
It does NOT affect functionality.

---

## 4. Chapters (Core documentation)

The page is structured into 4 technical chapters.

---

### Chapter I — System Genesis

This section explains:

- How CoreSys starts at UEFI firmware level
- Bootloader transition process
- Kernel entry point

### Key concept:

UEFI → Bootloader → Init Layer → Kernel → User Mode


This is the **core execution pipeline**.

---

### Chapter II — Boot Architecture

This section describes:

- Memory initialization
- Stack setup
- Interrupt preparation
- Hardware abstraction before kernel load

### Key idea:

The boot system is designed to be:
- deterministic
- BIOS-free
- UEFI-only

---

### Chapter III — Kernel Model

This section explains:

- 64-bit flat memory model
- No POSIX compatibility layer
- Minimal syscall system

### Important:

The kernel is NOT designed for compatibility.  
It is designed for **control and research**.

---

### Chapter IV — System Philosophy

This section explains design intent:

- CoreSys is a research OS
- Not a consumer OS
- All subsystems are replaceable
- Interfaces are experimental

### Summary:

The goal is understanding OS design from first principles.

---

## 5. Download section

### What you see:

CoreSys.img
CyberBoot.img


### How to use:

- Click the file link
- Download disk image
- Boot in a UEFI-compatible environment (VM or real hardware)

### Warning:

These are raw system images — not standard applications.

---

## 6. Documentation section (Docs)

### What it is:

This is the entry point to deeper technical documentation.

### It includes:

- Architecture breakdown
- Boot process explanation
- Kernel design details
- API reference

### Link:

Open Documentation 
This leads to: /docs/README.md


---

## 7. Legal section

### Purpose:

Explains usage restrictions and compliance notes.

### Key idea:

CoreSys is experimental software and may not comply with certain regional regulatory requirements.

---

## 8. How the system is structured (simple view)

Website (this page)
↓
Documentation (/docs)
↓
System Design (boot → kernel → API)
↓
Execution (UEFI environment)

Website (this page)
↓
Documentation (/docs)
↓
System Design (boot → kernel → API)
↓
Execution (UEFI environment)


---

## 9. Quick usage summary

To use this site:

- Click chapters → read system design
- Use font selector → change UI style
- Go to Download → get system images
- Go to Docs → technical deep dive

