"C:\MinGW64\bin\g++.exe" globldef.c -std=c++11 -c -o globldef.o
"C:\MinGW64\bin\g++.exe" strdef.c -std=c++11 -c -o cstrdef.o
"C:\MinGW64\bin\g++.exe" console.c -std=c++11 -c -o console.o
"C:\MinGW64\bin\g++.exe" strdef.cpp -std=c++11 -c -o strdef.o
"C:\MinGW64\bin\g++.exe" AudioBaseClass.cpp -std=c++11 -c -o AudioBaseClass.o
"C:\MinGW64\bin\g++.exe" AudioBitCrush.cpp -std=c++11 -c -o AudioBitCrush.o
"C:\MinGW64\bin\g++.exe" AudioReverse.cpp -std=c++11 -c -o AudioReverse.o
"C:\MinGW64\bin\g++.exe" AudioChannelSwap.cpp -std=c++11 -c -o AudioChannelSwap.o
"C:\MinGW64\bin\g++.exe" AudioChannelSubtract.cpp -std=c++11 -c -o AudioChannelSubtract.o
"C:\MinGW64\bin\g++.exe" test.cpp -std=c++11 -c -o test.o

"C:\MinGW64\bin\g++.exe" test.o globldef.o cstrdef.o console.o strdef.o AudioBaseClass.o AudioBitCrush.o AudioReverse.o AudioChannelSwap.o AudioChannelSubtract.o -o test.exe

del globldef.o
del cstrdef.o
del console.o
del strdef.o
del test.o
del AudioBaseClass.o
del AudioBitCrush.o
del AudioReverse.o
del AudioChannelSwap.o
del AudioChannelSubtract.o
