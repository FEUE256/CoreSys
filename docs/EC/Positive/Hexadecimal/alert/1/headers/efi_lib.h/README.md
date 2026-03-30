# Efi_lib.h

## Mode

- FW (Firmware) (0001)
    - Args (xxxxxxxx, can be anything just for debuging at  internal)
    - xxxxxxxx
        - Error Code (xxxxxxxxxxxxxxxxxxxxxxxxxxxx) +
        - OS-Other (xx)
            - 0x020100000000010000010001xxxxxxxx00000000000000000000000001xx
                    - This error means "Could not open file: {filename}"
            - 0x020100000000010000010001xxxxxxxx00000000000000000000000002xx
                    - This error means "Could not allocate memory for file: {filename}"
            - 0x020100000000010000010001xxxxxxxx00000000000000000000000003xx
                    - This error means "Could not read file: {filename} into buffer"
            - 0x020100000000010000010001xxxxxxxx00000000000000000000000004xx
                    - This error means "Could not Allocate buffer for disk data."
            - 0x020100000000010000010001xxxxxxxx00000000000000000000000005xx
                    - This error means "Could not open Block IO Protocol for this loaded image."
            - 0x020100000000010000010001xxxxxxxx00000000000000000000000006xx
                    - This error means "Could not find file {filename} in data partition."        
            - 0x020100000000010000010001xxxxxxxx00000000000000000000000007xx
                    - This error means "Could not find or read file {filename} to buffer."                          
            - 0x020100000000010000010001xxxxxxxx00000000000000000000000008xx
                    - This error means "Could not get size of 1st package list for type {package_type}"                          
            - 0x020100000000010000010001xxxxxxxx00000000000000000000000009xx
                    - This error means "Could not allocate buffer for package list of type {package_type}"    
            - 0x020100000000010000010001xxxxxxxx0000000000000000000000000Axx
                    - This error means "Could not export package list of type {package_type} into buffer."    
            - 0x020100000000010000010001xxxxxxxx0000000000000000000000000Bxx
                    - This error means "Could not find any memory to allocate pages for." 
 