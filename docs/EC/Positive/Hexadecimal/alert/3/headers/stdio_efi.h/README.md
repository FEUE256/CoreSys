# efi_lib.h

## Mode

- FW (Firmware) (0001)
    - Args (xxxxxxxx, can be anything just for debuging at  internal)
    - xxxxxxxx
        - Error Code (xxxxxxxxxxxxxxxxxxxxxxxxxxxx) +
        - OS-Other (xx)
            - 0x020300000000010000020001xxxxxxxx00000000000000000000000001xx
                    - This error means "EFI_UNSUPPORTED"
            - 0x020300000000010000020001xxxxxxxx00000000000000000000000002xx
                    - This error means "EFI_BUFFER_TOO_SMALL"
            - 0x020300000000010000020001xxxxxxxx00000000000000000000000003xx
                    - This error means "EFI_DEVICE_ERROR"
            - 0x020300000000010000020001xxxxxxxx00000000000000000000000004xx
                    - This error means "EFI_NOT_FOUND"
            - 0x020300000000010000020001xxxxxxxx00000000000000000000000005xx
                    - This error means "EFI_CRC_ERROR"