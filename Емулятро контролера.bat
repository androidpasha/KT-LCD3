:loop
  start "" SerialSend.exe /baudrate 9600 /hex "\x41\x10\x24\x01\xF3\x00\x6A\x02\x02\x00\x00\x41" /wait /b
  pathping 127.0.0.1 -n -q 1 -p 100 >nul
 goto loop

rem а лучше в cmd запустить команду на 13,6 км/час:
rem color 1b &  for /l %q in (0) do SerialSend.exe /baudrate 9600 /hex "\x41\x10\x24\x02\x25\x00\x6A\x02\x02\x00\x00\x41" & pathping 127.0.0.1 -n -q 1 -p 100 >nul


rem rem ping -n 1 -w 100000 192.168.254.254 >nul