# CoreSys Omega Pro File System (COPFS)

## Initialization

Initialize the filesystem:

```c
cop_init(NULL);
```

or

```c
cop_init(&init_task);
```

## Deinitialization

Unmount and deinitialize the filesystem:

```c
cop_deinit(NULL);
```

or

```c
cop_deinit(&deinit_task);
```

## Create a Directory

```c
cop_mkdir("/foo");
cop_mkdir("/foo/bar");
```

## Create a File

```c
cop_create("/foo/bar.txt");
```

## Write to a File

```c
const char data[] = "Hello COPFS";
cop_write("/home/user/test.txt", data, sizeof(data));
```

## Read a File

```c
char buffer[512];
cop_read("/home/user/test.txt", buffer, sizeof(buffer));
```

## Append to a File

```c
const char data[] = "Hello COPFS";
cop_append("/myfile.txt", data, sizeof(data));
```

## Delete a File or Directory

```c
cop_delete("/home/user");
```

Directories are deleted recursively.

## Execute a Binary

```c
cop_exec_file("/bin/bar.bin");
```

## Initialize the Standard Filesystem

Creates the default directories and files, including:

- `/sys/kernel/kernel.cfg`
- `/sys/system/debug.cfg`
- `/sys/system/boot/bc.sctfi`
- And more in future versions

```c
fs_init(NULL);
```

or

```c
fs_init(&cs_task_context);
```

## Deinitialize the Standard Filesystem

Removes the standard filesystem structure.

```c
fs_deinit(NULL);
```

or

```c
fs_deinit(&cs_task_context);
```
