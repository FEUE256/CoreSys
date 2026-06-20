# Args

The args of the function giving the error.

- 00 = Just buffering for 256 bit error code
- 01 = VOID
- 02 = CHAR16
- 03 = UINTN
- 04 = EFI_LBA
- 05 = UINT32
- 06 = BOOL
- 07 = EFI_GRAPHICS_OUTPUT_PROTOCOL
- 08 = char
- 09 = UINT8
- 0A = Memory_Map_Info
- 0B = No Args
- 
These will stack ex:

0203 in the args means that the first arg is a UINTN and the second arg is a CHAR16. This is just for internal debuging. It's readable from right to left. So the first arg is the rightmost one. The second arg is the one before that and so on.
