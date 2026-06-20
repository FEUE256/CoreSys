# Efi_lib.h

## Mode

- FW (Firmware) (0001)
    - Args (xxxxxxxx, can be anything just for debuging at  internal)
    - xxxxxxxx
        - Error Code (xxxxxxxxxxxxxxxxxxxxxxxxxxxx) +
        - OS-Other (xx)
            - 0x020300000000010000010001xxxxxxxx00000000000000000000000001xx
                    - This error means "Could not get file info for file: {filename}"
            0x020300000000010000010001xxxxxxxx00000000000000000000000002xx
                    - This error means "Could not locate any Block IO Protocols"
            0x020300000000010000010001xxxxxxxx00000000000000000000000003xx
                    - This error means "Could not find Block IO protocol for disk with ID {disk_mediaID}"
            0x020300000000010000010001xxxxxxxx00000000000000000000000004xx
                    - This error means "Could not read Disk LBAs into buffer"
            0x020300000000010000010001xxxxxxxx00000000000000000000000005xx
                    - This error means "Could not locate GOP"
            0x020300000000010000010001xxxxxxxx00000000000000000000000006xx
                    - This error means "Could not set GOP mode {gop_mode}"
            0x020300000000010000010001xxxxxxxx00000000000000000000000007xx           
                    - This error means "Could not find or get MediaID value for disk image"  
            0x020300000000010000010001xxxxxxxx00000000000000000000000008xx           
                    - This error means "Could not find disk lba value for {in_name}"    
            0x020300000000010000010001xxxxxxxx00000000000000000000000009xx           
                    - This error means "Could not get size of list of handles for HII package lists for type {package_type}" 
            0x020300000000010000010001xxxxxxxx0000000000000000000000000Axx           
                    - This error means "Could not get list of handles for HII package lists for type {package_type} into buffer." 
            0x020300000000010000010001xxxxxxxx0000000000000000000000000Bxx           
                    - This error means "SetVirtualAddressMap()" 