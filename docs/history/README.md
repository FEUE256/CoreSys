# History

This documentation starts at `2026-02-20`.

This document contains major milestones, normal updates, and development progress for CoreSys. It also works as a small development blog for the project.

All entries were written by Nils Efverman.

---

## 0.1.2.5v (2026-05-30)

Small update.

### Build Performance

Build executed via the root Makefile using WSL2:

```bash
time make all
```

#### Build Timing

* Real: `20.609s`
* User: `8.352s`
* Sys: `1.778s`

---

## 0.1.2.4v (2026-05-24)

Added Lua and BusyBox to CoreSys Linux.

Also added a small CIOS testing directory:

```text
test/
```

---

## 0.1.2.3 / 0.1.2.2 (2026-05-17)

* Added project changes
* Small `src/` update

---

## 0.1.2.1v (2026-05-16)

Added small updates to the kernel and init system.

---

## 0.1.2.0v (Date Unknown)

Added CSE:

```text
CoreSys Encryption (SHA256)
```

---

## Added Core Kernel Features (2026-05-03)

Added:

* Task creation
* System calls

---

## Small Things (2026-05-03)

My computer has not been working correctly, which caused delays.

Changes:

* Added `bpkg` to the kernel
* Fixed `ne.cfg`
* Added `pyyaml` and `pip` to dependency lists
* Added Arch Linux support

---

### Disclaimer

Build times are measured via SSH on a Raspberry Pi 4 with 4 GB RAM.

---

## 12.0 CSC (2026-04-24)

Added a new minimal GUI structure.

This system will not receive major GUI development until further notice.

### Build Timing

* Real: `33.666s`
* User: `5.547s`
* Sys: `1.750s`

---

## 11.0 CSC (2026-04-24)

The 11.0 CSC release introduced:

* A completely new kernel design
* New documentation
* New scripts
* YAML configuration support
* Updated `dist/CoreSys.img`

### Build Timing

* Real: `27.979s`
* User: `5.436s`
* Sys: `1.714s`

---

## API (2026-04-19)

Fixed the CoreSys API.

---

## ExitBootServices (2026-04-09)

Verified that `ExitBootServices()` and the kernel work correctly together.

---

## UEFI Shell and Recovery (2026-04-08 → 2026-04-09)

Fixed the recovery system in the CoreSys bootloader.

---

## Small Fixes (2026-04-07)

Small maintenance fixes and improvements.

---

## Big Update Coming (2026-03-09 → After cd3f594 Release)

First, sorry for the long wait.

Time of writing: `2026-03-27`

I have been sick, and my computer has also been unstable. Despite the delays, there has been a lot of behind-the-scenes work for the CoreSys environment.

Major changes:

* New error code system
* New `src/` file structure
* Many additional internal improvements

---

## Things (2026-03-09)

Just some smaller and more relaxed development work.

I honestly do not remember everything added here, but some small improvements were made.

---

## Some Quick Updates (2026-03-07)

Minor repository and GitHub updates, including:

* Administrative/legal updates
* Public site improvements
* Updated `.gitignore`
* Documentation updates
* Improved file structure clarity

---

## First EDK Headers Working (2026-02-21)

Today was a major day for CoreSys.

We successfully used EDK headers inside CoreSys.

Example source:

```text
CoreSys/src/CoreSys/efi.c
```

This feels similar to a "Hello, World" moment for the project.

Now we can focus on developing a working bootloader.

---

## Added EDK Header Support (2026-02-21)

Added support for `EDK II` x86_64 headers in CoreSys.

This is a major step toward building:

* A working bootloader
* UEFI applications
* Better firmware integration

---

## New LICENSE (2026-02-20)

As of `2026-02-20`, the project license changed from:

```text
MIT LICENSE
```

to:

```text
FÈUE LICENSE V1
```

See:

```text
../../NOTICE.md
```

Note:
`NOTICE.md` may later be removed or repurposed for other topics.
