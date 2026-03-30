# CoreSys Makefile Cheat Sheet

> All commands assume you are in the `build/master` directory.

---

1. Build Everything
```bash
make all -B 
```
This command will build all components of the project, regardless of whether they have been modified since the last build. The `-B` flag forces a rebuild of all targets.

2. Build Only Main EDK2
```bash
make edk2 -B 
```
This command will build only the EDK2 component of the project. Similar to the previous command, it forces a rebuild of the EDK2 target.

3. Build Tests and Secondary EDK2
```bash
make edk2_3 -B 
```
This command will build the tests and secondary EDK2 component of the project. Again, it forces a rebuild of the specified target.

4. Build Subdirectories
```bash
make subdirs -B 
```
This command will build all subdirectories of the project. It forces a rebuild of all targets within the subdirectories.

5. Clean Everything
```bash
make clean -B
```
This command will clean all build artifacts from the project. The `-B` flag ensures that all targets are cleaned, regardless of their current state. Note that parallel execution is not typically necessary for cleaning, as it is a single operation that removes files.

6. Clean Subdirectories Only (Skip EDK2/EDK23)
```bash
make clean -B EDK_NOT_CLEAN=1
```
This command will clean all subdirectories of the project, but it will skip cleaning the EDK2 and EDK23 components. The `EDK_NOT_CLEAN=1` variable is used to indicate that these components should not be cleaned. The `-B` flag ensures that all specified targets are cleaned, regardless of their current state.

7. Dry Run (Preview Commands)
```bash
make -n all
```
This command will perform a dry run of the build process, showing the commands that would be executed without actually running them. This is useful for previewing the build steps and ensuring that the correct commands will be executed when you run the actual build.

8. Notes
Ensure config files exist:
- `../../.config/build.cfg`
- `../../.config/config.cfg`

Required environment variables:
- `EDK2_DIR`
- `EDK2_BUILD_DIR`
- `EDK23_DIR`
- `BASETOOLS2_DIR`
- `SUBDIRS` (list of subdirectories)

Build flags:
- `-B` forces rebuild even if targets are up-to-date.

9. Qemu
Run:
`make qemu -B` for starting qemu. This can be combined with others like `clean`, `all`, etc.
