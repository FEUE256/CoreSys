# Efi_lib.h

## Mode

- FW (Firmware) (0001)
    - Args (xxxxxxxx, can be anything just for debuging at  internal)
    - xxxxxxxx
        - Error Code (xxxxxxxxxxxxxxxxxxxxxxxxxxxx) +
        - OS-Other (xx)
            - 0x010200000000010000010001xxxxxxxx00000000000000000000000001xx
                    - This error means "Could not find file {in_data} in data partition"
            - 0x010200000000010000010001xxxxxxxx00000000000000000000000002xx
                    - This error means "Could not find or read data partition file {in_name} to buffer\
