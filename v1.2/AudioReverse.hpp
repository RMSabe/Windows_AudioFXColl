/*
	Audio FX Collection version 1.2 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOREVERSE_HPP
#define AUDIOREVERSE_HPP

#include "AudioBaseClass.hpp"

class AudioReverse : public AudioBaseClass {
	public:
		AudioReverse(VOID);
		AudioReverse(__string filein_dir);
		AudioReverse(__string filein_dir, __string fileout_dir);
		~AudioReverse(VOID);

		BOOL WINAPI runDSP(VOID) override;

	private:
		VOID *p_buffer_input = nullptr;
		VOID *p_buffer_output = nullptr;

		BOOL WINAPI buffer_alloc(SIZE_T size) override;
		VOID WINAPI buffer_free(VOID) override;

		BOOL WINAPI runDSP_i16(VOID);
		BOOL WINAPI runDSP_i24(VOID);

		VOID WINAPI dsp_init_i16(VOID);
		VOID WINAPI dsp_init_i24(BYTE *p_bytebuf);

		VOID WINAPI dsp_loop_i16(VOID);
		VOID WINAPI dsp_loop_i24(BYTE *p_bytebuf);
};

#endif //AUDIOREVERSE_HPP
