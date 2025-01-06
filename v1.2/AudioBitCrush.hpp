/*
	Audio FX Collection version 1.2 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOBITCRUSH_HPP
#define AUDIOBITCRUSH_HPP

#include "AudioBaseClass.hpp"

class AudioBitCrush : public AudioBaseClass {
	public:
		AudioBitCrush(VOID);
		AudioBitCrush(__string filein_dir);
		AudioBitCrush(__string filein_dir, __string fileout_dir);
		~AudioBitCrush(VOID);

		BOOL WINAPI setCutoff(BYTE bitcrush);
		BOOL WINAPI runDSP(VOID) override;

	private:
		VOID *p_buffer = nullptr;
		INT32 cutoff = 0;

		BOOL WINAPI buffer_alloc(SIZE_T size) override;
		VOID WINAPI buffer_free(VOID) override;

		BOOL WINAPI runDSP_i16(VOID);
		BOOL WINAPI runDSP_i24(VOID);

		VOID WINAPI dsp_loop_i16(VOID);
		VOID WINAPI dsp_loop_i24(BYTE *p_bytebuf);
};

#endif //AUDIOBITCRUSH_HPP
