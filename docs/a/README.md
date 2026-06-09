# Arduino Guide

Reqs:
    CoreSys
    Power User
    Arduino Uno SMD
    Arduino IDE
    pwsh
    WSL

Baud Rate: 115200

COM1 = 0x3F8
COM2 = 0x2F8
COM3 = 0x3E8
COM4 = 0x2E8

1. Flash src/arduino/main.cpp to the arduino
2. Write pin 13 -> LED -> resistor -> GND
3. Look up whitch COM port the arduino is at via [System.IO.Ports.SerialPort]::GetPortNames() and then Get-CimInstance Win32_SerialPort | Where-Object {$_.DeviceID -eq "[THE PORT]"} | Select-Object DeviceID, Description eg  Get-CimInstance Win32_SerialPort | Where-Object {$_.DeviceID -eq "COM4"} | Select-Object DeviceID, Description for evry pot and then look for Arduino uno remeber that name eg COM2
4. Edit scripts/VM/a.sh line 36 to the /dev/tyyS[x] from your COM[x], but - 1 on windows eg COM4 -> /dev/ttyS3 and COM1 -> /dev/ttyS0
5. Edit src/CoreSys/kernel/serial/main.h and look for #define a_port = xxxxx and change the 0x2E8 to the COM port to 0x in the table in this doc
6. Build CoreSys and run with make a in build/master (run make with sudo)
7. Go to the kernel terminal and then type on or off

(If the COM[x] is over 4 try to get it under 4 (bsc of hw limitations via outb))