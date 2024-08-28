/*
	Audio FX Collection version 1.0 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOREVERSE_HPP
#define AUDIOREVERSE_HPP

#include "AudioBaseClass.hpp"

class AudioReverse : public AudioBaseClass {
	public:
		AudioReverse(VOID);
		AudioReverse(std::string filein_dir);
		AudioReverse(std::string filein_dir, std::string fileout_dir);
		~AudioReverse(VOID);

		BOOL runDSP(VOID) override;

	private:
		VOID *buffer_input = nullptr;
		VOID *buffer_output = nullptr;

		BOOL runDSP_i16(VOID);
		BOOL runDSP_i24(VOID);

		VOID dsp_init_i16(VOID);
		VOID dsp_init_i24(UCHAR *bytebuf);

		VOID dsp_loop_i16(VOID);
		VOID dsp_loop_i24(UCHAR *bytebuf);
};

#endif //AUDIOREVERSE_HPP
