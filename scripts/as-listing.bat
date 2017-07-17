:: 1st arg 	- toolchain direcory
:: 2nd arg	- output directory
:: 3rd arg	- output file name
:: 4rd arg	- output file name without extension
@echo off
%1\bin\arm-none-eabi-objdump.exe -dC -j .isr_vector -j .text -j .rodata -j .ARM -j .ARM.extab -j .init_array -j .data -j .bss  %2\%3 > %2\%4.lss