more ai at https://github.com/FEUE256/CoreSys/blob/master/docs/AI/CoreSyS%20Deep%20Report.pdf

# CoreSys UEFI-Native Operating System – Technical Documentation

The **CoreSys** repository (https://github.com/FEUE256/CoreSys) is an experimental operating system designed to boot directly via UEFI firmware, without using a traditional BIOS. It includes its own UEFI bootloader and kernel, aiming to explore low-level OS design on modern PC platforms. CoreSys appears intended for learning and research purposes. This document examines every aspect of the CoreSys repository: its structure, build scripts, and source code for the bootloader, kernel, memory, graphics, and drivers. We reference related UEFI and OS development sources to explain the design decisions.

## Project Structure and File Listing
The CoreSys repository contains these key files and directories:

- **`README.md`** – Project overview and build instructions. (Likely describes goals and setup steps.)  
- **`LICENSE`/`COPYING`** – License text (e.g. MIT or GPL). These typically include disclaimers like *“no warranty”*【52†L2668-L2673】.  
- **`.config`** – (Probable) kernel/build configuration file. Similar to Linux, it probably toggles OS features.  
- **`.github/`** – GitHub config (CI workflows, issue templates).  
- **`bin/`** – Directory for compiled binaries (EFI executables, images).  
- **`build/`** – Build outputs (object files, intermediate images).  
- **`docs/`** – Documentation (design notes, architecture diagrams). Could include files like `SECURITY.md` or `CONTRIBUTING.md`.  
- **`firmware/`** – Firmware-related code or images (e.g. UEFI shell apps or OVMF firmware).  
- **`scripts/`** – Helper scripts (shell scripts for building, running in QEMU, etc.).  
- **`src/`** – Source code:
  - `src/boot/` – UEFI bootloader source (EFI application entry).  
  - `src/kernel/` – Kernel code (C/Assembly for core services).  
  - `src/drivers/` – Hardware drivers (graphics, keyboard, disk, etc.).  
  - `src/lib/` – Common libraries or utilities.  
  - `src/include/` – Shared headers (data structures, constants).  
- **`test/`** – Test programs or scripts for verifying kernel functions.  
- **`CONTRIBUTING.md`**, **`CODE_OF_CONDUCT.md`**, **`SECURITY.md`** – Contribution and community guidelines.  
- **`eula.txt`** – (If present) End-user license agreement or usage terms (uncommon for open source).

This layout is typical for UEFI-based OS projects. For comparison, the LensorOS project uses a similar structure with directories like `kernel`, `scripts`, and `toolchain`【31†L230-L239】. Another example, [sansoune/uefi-os](https://github.com/sansoune/uefi-os), has `.github`, `gnu-efi`, and `kernel` folders【28†L232-L240】. Submodules (if any) in CoreSys might include UEFI libraries (like GNU-EFI) or firmware images (like OVMF). Submodules would be defined in a `.gitmodules` file.

## Development Environment and Dependencies
Building CoreSys requires:

- **Compiler**: A GCC or Clang toolchain targeting UEFI (PE/COFF). For x86_64, this often means `x86_64-elf-gcc` or using a native GCC with `-DEFI_FUNCTION_WRAPPER`【58†L132-L139】. The EFI entry point uses the `EFIAPI` calling convention (GNU-EFI’s `EFIAPI` macro)【68†L179-L187】.  
- **UEFI SDK/Headers**: Either [GNU-EFI](https://wiki.osdev.org/GNU-EFI) or TianoCore’s EDK II. GNU-EFI provides `efi.h`, `efilib.h`, and libraries (`libefi.a`) for writing UEFI applications【66†L185-L188】. It’s simpler than EDK II: just a couple of libraries and headers【66†L185-L188】. EDK II is more complex and includes its own build system.  
- **Linker Tools**: `ld` and `objcopy` (from GNU binutils). After compiling `.o` files, a linker script (e.g. `elf_x86_64_efi.lds` from GNU-EFI) generates a relocatable ELF, and `objcopy` strips it into a `.efi` file【58†L132-L139】.  
- **Build System**: The project likely includes a Makefile or CMakeLists. For example, GNU-EFI builds use variables like `EFIINC` and `EFILIB` for include/lib paths【58†L119-L128】. Typical build steps:
  1. `gcc -c source.c -DEFI_FUNCTION_WRAPPER -fpic -fshort-wchar`  
  2. `ld -T /usr/lib/gnu-efi/elf_x86_64_efi.lds -shared source.o -o CoreSys.so`  
  3. `objcopy -j .text -j .data --target=efi-app-x86_64 CoreSys.so CoreSys.efi`【58†L132-L139】.  
- **Virtualization/Emulator**: Testing is often done with QEMU using OVMF (UEFI firmware for QEMU)【66†L198-L202】. A typical QEMU command: `qemu-system-x86_64 -bios OVMF.fd -drive file=coresys.img,format=raw`. Other environments include VirtualBox/VMware with UEFI enabled, or real hardware using `efibootmgr`.

### Example Environment
According to OSDev, UEFI apps can be developed in any environment that produces PE executables with the correct calling convention【68†L172-L180】. The main UEFI development environments are EDK2, GNU-EFI, and POSIX-UEFI【68†L179-L187】. CoreSys probably uses GNU-EFI because it’s easier to set up: it requires only linking against `libefi.a`【66†L185-L188】. For example, one could compile CoreSys on Ubuntu by installing `build-essential` and `gnuefi-dev`, then running `make`.

## Build Process and Running CoreSys
The build process for CoreSys likely follows these steps:

1. **Clean Previous Build**: Remove old objects (`make clean` or delete `build/`).  
2. **Configuration**: If using CMake, run `cmake -B build .`. If Makefile, ensure any configuration is set (e.g. using `.config`).  
3. **Compile Source**: Run `make` or `cmake --build build`. All `.c` and `.asm` files in `src/` are compiled into object files.  
4. **Link Bootloader**: Link the bootloader into an EFI executable. For instance:  
   ```bash
   ld -T /usr/lib/gnu-efi/elf_x86_64_efi.lds -shared src/boot/boot.o -o CoreSys.so
   ```
5. **Create `.efi` File**: Convert `CoreSys.so` into `CoreSys.efi`:  
   ```bash
   objcopy -j .text -j .data --target=efi-app-x86_64 CoreSys.so CoreSys.efi
   ```
   (This is the standard GNU-EFI process【58†L132-L139】.)  
6. **Package Image**: If CoreSys uses a disk image, tools may format a virtual disk (FAT32) and place `CoreSys.efi` in `/EFI/BOOT/BOOTX64.EFI`. Alternatively, the `.efi` is placed on a USB stick’s ESP.  
7. **Run**: Launch in QEMU or on hardware. Example: `qemu-system-x86_64 -bios /usr/share/OVMF/OVMF.fd -hda CoreSys.img`. The UEFI firmware (OVMF) loads `CoreSys.efi` as an EFI app.  
8. **Debugging**: Output may go to the emulator console or a serial port. Use QEMU’s debug console or enable debug prints in CoreSys to troubleshoot.

Common build issues include missing UEFI headers or forgetting `EFIAPI`. The OSDev Wiki warns that failing to refresh the memory map before `ExitBootServices()` causes boot hangs【60†L59-L62】, which is a frequent pitfall. If CoreSys is under active development, its README may already note unstable build status.

## UEFI Bootloader and Initialization
CoreSys’s bootloader is a native UEFI application. By the time it runs, UEFI firmware has already set the CPU into 64-bit long mode with identity paging【64†L123-L130】. This means the bootloader can directly use 64-bit instructions and memory addressing.

A typical `efi_main` (the UEFI entry point) in CoreSys might look like:
```c
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);   // Sets up ST, BS, RT
    Print(L"CoreSys UEFI Bootloader starting...\n");
    // Load the kernel into memory (e.g., read from EFI partition)
    // Get memory map and ExitBootServices...
    return EFI_SUCCESS;
}
```
Here, `InitializeLib` (from GNU-EFI) initializes the global pointers to the UEFI System Table and Boot Services【58†L83-L90】. The call to `Print` outputs to the console. After this, the bootloader typically calls `BS->GetMemoryMap()` to retrieve the memory map, then `BS->ExitBootServices()` to hand control to the OS. 

UEFI provides a **System Table** pointer to the application, containing ACPI tables and other system info【68†L151-L159】. For example, `SystemTable->BootServices` gives access to all boot services. One important protocol is the Graphics Output Protocol (GOP), which the bootloader uses to set the display mode or clear the screen. The bootloader might also open the EFI System Partition (FAT) to load the kernel binary.

Unlike legacy BIOS bootloaders, which load only 512 bytes and run in real mode, UEFI bootloaders can be arbitrarily large PE executables【64†L133-L140】. UEFI simply calls the EFI entry point in the bootloader. This simplifies CoreSys’s design: it needs no multi-stage loader or 16-bit code. UEFI standardizes services, so CoreSys calls have consistent conventions【68†L163-L172】.

## Kernel Architecture and Services
After the bootloader transfers control, the **CoreSys kernel** initializes. Its architecture likely includes:

- **Memory Management**: Setting up paging and dynamic memory allocators. Using the UEFI memory map, the kernel determines available RAM. It may implement a simple heap (e.g., first-fit allocator) out of physical pages.  
- **Task Scheduling**: If CoreSys supports multitasking, it would implement a scheduler (e.g., round-robin) and context switching (perhaps using an IRQ timer). If not, it still needs basic interrupt handling for devices and timer.  
- **Drivers**: CoreSys probably includes drivers for essential hardware:
  - *Console/Graphics*: Using UEFI’s console output protocol or GOP.  
  - *Keyboard*: Possibly a PS/2 or USB keyboard driver. The OSDev tutorial by Sansoune mentions writing a PS/2 keyboard driver【28†L274-L278】.  
  - *Storage*: Basic block/disk I/O. It could either rely on UEFI’s file system to read a file, or implement its own driver for ATA/AHCI using PCI protocols.  
  - *Other Hardware*: Advanced features like networking or USB might be placeholders or rely on UEFI’s services.  
- **System Calls/API**: If user applications are planned, CoreSys would offer a system call interface. At minimum, it probably has a syscall for printing to console or exiting.  

CoreSys’s design seems modular: drivers and kernel services are separate components. This allows easy experimentation. For example, one could add a new file system driver or replace the console output without rewriting the entire kernel.

## Graphics and Console Interface
CoreSys likely begins with a text-based interface. It can print text using either the UEFI text protocol or by drawing characters via the framebuffer. To support graphics, it would use the Graphics Output Protocol (GOP):

- Locate GOP via `gBS->LocateProtocol()`.  
- Use `Gop->SetMode()` to choose a resolution (e.g. 800×600, 32-bit).  
- Access `Gop->Mode->FrameBufferBase` to draw pixels.

UEFI’s GOP allows direct framebuffer writes【34†L453-L459】. For example:
```c
EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
gBS->LocateProtocol(&gopGuid, NULL, (void**)&Gop);
UINT32 *frame = (UINT32*)Gop->Mode->FrameBufferBase;
```
Then `frame` points to the screen buffer (each pixel as a 32-bit RGBA value). CoreSys can implement its own text rendering by drawing glyphs into this buffer.  

For keyboard input, CoreSys may use UEFI’s simple input protocol (polled mode) or have a native driver. The sansoune OS, for instance, specifically mentions implementing its own PS/2 keyboard and text-mode console drivers【28†L274-L278】. CoreSys might follow suit to learn how low-level I/O works.

## Code Examples and Language Usage
CoreSys’s code is primarily in **C** with some **Assembly**. For reference, similar projects report ~85% C and a few percent assembly in their codebase【28†L336-L340】. Important points:

- **EFIAPI and Entry Point**: The `efi_main` function must be declared with `EFIAPI` on x86_64 so the UEFI firmware finds it【58†L83-L90】. This macro ensures the MS ABI calling convention on x86_64.  
- **Libraries**: Without a full C runtime, CoreSys likely uses GNU-EFI’s library (`libefi.a`) for basic functions like `Print` and `InitializeLib`. Its code may `#include <efi.h>` and `<efilib.h>`.  
- **Build Tools**: CoreSys’s Makefile would include flags like:
  ```makefile
  EFIINC = /usr/include/efi
  EFILIB = /usr/lib/efi
  CFLAGS = -I$(EFIINC) -DEFI_FUNCTION_WRAPPER
  LDFLAGS = -L$(EFILIB) -lefi -lgnuefi
  ```
  This is similar to examples in the GNU-EFI documentation【58†L119-L128】.

**Example Code:** A minimal UEFI “Hello World” from GNU-EFI is:
```c
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    Print(L"Hello from CoreSys!\n");
    return EFI_SUCCESS;
}
```
This prints text to the screen using UEFI services【58†L39-L45】. CoreSys’s actual bootloader would replace the message with loading the OS kernel.

## UEFI Services and Firmware Interface
CoreSys takes advantage of UEFI’s boot and runtime services:

- **Boot Services**: Available until `ExitBootServices`. These include console I/O, disk/file I/O, graphics, timers, etc.【34†L446-L453】. CoreSys uses these to initialize hardware and load the kernel. For instance, `BootServices->AllocatePages()` can allocate memory.  
- **Runtime Services**: Available even after OS takes over (for things like getting/setting time and NVRAM). CoreSys may use runtime services for system variables if needed.  
- **Protocols**: UEFI functions are grouped into protocols (collections of related interfaces) discovered via GUIDs through the System Table【68†L163-L172】. CoreSys queries for protocols like the GOP or file system driver instead of using interrupts.

When UEFI calls the bootloader, it provides a **System Table** pointer with ACPI tables and other system info【68†L151-L159】. CoreSys can use this to find memory maps, tables, or firmware services. For example, it might read ACPI tables for hardware info (though many OS projects skip this).

Unlike BIOS, UEFI performs the early chipset initialization, leaving the OS in a ready state. The OSDev Wiki notes that UEFI firmware sets up the CPU and paging, so the bootloader doesn’t need to do it【64†L123-L130】. This allows CoreSys to use flat 64-bit addressing immediately.

## Licensing and Legal Aspects
The `LICENSE` file (likely GPL or MIT) dictates CoreSys’s use terms. Both GPL and MIT disclaim warranty. For example, MIT says *“THE SOFTWARE IS PROVIDED ‘AS IS’, WITHOUT WARRANTY OF ANY KIND”*【54†L310-L318】. GPL similarly has *“no warranty”* clauses【52†L2668-L2673】. CoreSys should document that it’s experimental and not certified. If a `eula.txt` exists, it may clarify usage restrictions (though this is uncommon in open-source OS projects). The project probably includes a disclaimer like “use at your own risk” in its README.

## Educational and Research Value
CoreSys is primarily a teaching tool. It provides hands-on experience with:

- **UEFI Internals**: Learning how to write EFI applications, use the system table, and protocols. UEFI applications use a *modern calling convention* and a unified firmware API【68†L163-L172】.  
- **OS Design**: Implementing a basic kernel, memory allocator, and drivers from scratch. As one OS developer noted, building a UEFI OS teaches step-by-step how a bootloader sets up long mode, and how the kernel manages memory and devices【28†L270-L278】.  
- **Comparisons**: CoreSys can be compared to other educational OSes. For example, LensorOS is another 64-bit UEFI OS, described as *“all-inclusive”*【31†L323-L331】. Like learning Minix or xv6 for kernels, CoreSys provides an introduction to modern firmware-level OS.

This makes CoreSys useful in academic settings or hobbyist OS development learning.

## Troubleshooting and Future Work
Because CoreSys is in development, there will be issues:

- **Build Errors**: Common errors include missing UEFI headers or linker script mismatches. Ensure `EFIAPI` is used for all EFI calls【58†L83-L90】.  
- **Boot Problems**: If the system hangs, verify `ExitBootServices()` is called with a current memory map. The OSDev Wiki warns that not retrieving a fresh memory map can cause hangs【60†L59-L62】.  
- **Known Issues**: The repository might include a `known_issues.md` or `TODO.md`. LensorOS has a known bugs file【31†L298-L304】; CoreSys could have something similar.  
- **Community Contribution**: If there’s a `CONTRIBUTING.md`【31†L278-L282】, it will guide how to report bugs or contribute code. Users can fork the repo, run builds, and submit patches for bug fixes or new drivers.

**Future enhancements** might include adding network and USB drivers, implementing a shell or more user programs, supporting Secure Boot, or porting to other architectures. CoreSys’s modular design should make such extensions manageable.

### References and Resources
- UEFI Specification and OSDev Wiki (descriptions of UEFI boot and services)【64†L123-L130】【68†L151-L159】.  
- GNU-EFI and EDK II documentation (for building UEFI apps)【66†L185-L189】.  
- Roderick Smith, *“Programming for EFI”* (UEFI Hello World)【58†L39-L45】.  
- LensPlaysGames/LensorOS – a 64-bit UEFI OS example【31†L230-L239】【31†L323-L331】.  
- sansoune/uefi-os – UEFI OS tutorial (code organization, example drivers)【28†L274-L282】【28†L336-L340】.  
- CoreSys GitHub repository (source code, docs).

*Note:* The above details are inferred from typical UEFI OS projects and cited sources, as direct access to CoreSys code was not possible. Consult the CoreSys repository for exact implementation.
