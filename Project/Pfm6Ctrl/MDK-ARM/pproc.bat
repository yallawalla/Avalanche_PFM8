del   ..\..\..\bin\*.bin, ..\..\..\bin\*.hex
copy  output\*.hex  ..\..\..\bin\*.hex
srec_cat.exe ..\..\..\bin\Pfm8Ctrl.hex -Intel -crop 0x8008000 -offset -0x8008000 -o ..\..\..\bin\Pfm8Ctrl.bin -Binary
