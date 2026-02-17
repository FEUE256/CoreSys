# Developer Documentation

## Project Overview

CoreSys is a system project with a diskwriter utility for hardware deployment and a web-based interface for management website. The project supports both software emulation (via QEMU) and hardware deployment. -j in make is not supported.

## Development Setup

### Prerequisites
- See in docs/req.md

### Building

From the project root:

```bash
# Create build directory
mkdir -p build/master

# Build with CMake (if using CMake)
make all -B

```

Use make subdirs -B for not building edk2. 
Use make DO_TEST=1 all for build with tests.
Use make DO_TEST=1 subdirs -B for build with tests without edk2.

### Testing with QEMU

```bash
cd build/master
make qemu
```

## Code Guidelines

### C Code (diskwriter)
- Use consistent naming conventions
- Include proper header guards in .h files
- Add function documentation comments
- Handle errors appropriately
- Test device interactions thoroughly

### Web Interface
- Keep HTML semantic and accessible
- Use CSS for styling (no inline styles)
- Keep JavaScript modular and commented
- Test across browsers

## Building for Release

```bash
cd build
cmake --build . --config Release
# Binaries available in bin/Release/Diskwriter/
```

## Common Tasks

### Running the Diskwriter
**Software (QEMU):**
```bash
./scripts/qemu.sh
```

**Hardware (WARNING: Destructive):**
```bash
# Unmount target disk first
umount -f /mnt/DISK_NAME
# Run diskwriter with sudo
sudo ./bin/Release/Diskwriter/diskwriter
```

### Git Workflow
```bash
./scripts/git_push.sh
```

## Debugging

- Use CMake with Debug configuration for debugging symbols
- Enable verbose build output for troubleshooting
- Test changes in QEMU before hardware deployment

## Contributing

See [CONTRIBUTING.md](../../CONTRIBUTING.md) in the project root for guidelines.

## Security

See [SECURITY.md](../../SECURITY.md) for security policies and reporting procedures.

