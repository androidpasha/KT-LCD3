:loop
start "" SerialSend.exe /baudrate 9600 /hex "\x41\x10\x24\x01\xF3\x00\x6A\x02\x02\x00\x00\x41"

pathping 127.0.0.1 -n -q 1 -p 300 >nul
 goto loop
rem