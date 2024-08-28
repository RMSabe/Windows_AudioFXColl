Audio FX Collection version 1.0

This is a collection of some audio effects for Windows.

All the effects are non real-time effects, meaning they generate an output audio file with the processed signal from the input file.

It only supports .wav files, 16-bit and 24-bit encoding.

Effect description:
Bit Crush: reduce the sample resolution by removing a certain amount of data bits. Only data is changed, output file format remains the same as input file.

Reverse (backward audio): copies all samples from the input file to the output file in opposite order (timewise), but without swapping the channels.

Channel Swap: invert channels from the input file.

Channel Subtract: Subtract the summed signals of all other audio channels from each audio channel.
Example: 
Stereo Audio: L = L-R, R = R-L
5.1 Audio: FL = FL - (FR + C + SL + SR + Sub), FR = FR - (FL + C + SL + SR + Sub), C = C - (FL + FR + SL + SR + Sub), ...

Extras:
I left some test C++ source files to test the effects. Also I left a batch script to compile all of them.
I used the MinGW compiler, with the compiler folder on C:\ . You might have to change the batch script depending on your compiler and its location.

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com
