# Efi_lib.h

## Mode

- FW (Firmware) (0001)
    - Args (xxxxxxxx, can be anything just for debuging at  internal)
    - xxxxxxxx
        - Error Code (xxxxxxxxxxxxxxxxxxxxxxxxxxxx) +
        - OS-Other (xx)
            - 0x020200000000010000010001xxxxxxxx00000000000000000000000001xx
                    - This error means "Could not open Loaded Image Protocol"
            - 0x020200000000010000010001xxxxxxxx00000000000000000000000002xx
                    - This error means "Could not open Simple File System Protocol"
            - 0x020200000000010000010001xxxxxxxx00000000000000000000000003xx
                    - This error means "Could not Open Volume for root directory"
            - 0x020200000000010000010001xxxxxxxx00000000000000000000000004xx
                    - This error means "Could not get root directory of ESP"
            - 0x020200000000010000010001xxxxxxxx00000000000000000000000005xx
                    - This error means "Could not Open Disk IO protocol on handle {handle}"
            - 0x020200000000010000010001xxxxxxxx00000000000000000000000006xx
                    - This error means "Could not Query GOP Mode {gop_mode}"        
            - 0x020200000000010000010001xxxxxxxx00000000000000000000000007xx
                    - This error means "Could not find file size for {in_name}"     
            - 0x020200000000010000010001xxxxxxxx00000000000000000000000008xx
                    - This error means "Could not locate HII Database Protocol!"   
            - 0x020200000000010000010001xxxxxxxx00000000000000000000000009xx
                    - This error means "Could not allocate buffer for handle list for package lists type {package_type}."   
            - 0x020200000000010000010001xxxxxxxx0000000000000000000000000Axx
                    - This error means "Could not allocate runtime descriptors memory map."   
