/*
	Audio FX Collection version 1.1 for Windows

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

		BOOL setCutoff(UCHAR bitcrush);
		BOOL runDSP(VOID) override;

	private:
		VOID *buffer = nullptr;
		INT cutoff = 0;

		BOOL setCutoff_i16(UCHAR bitcrush);
		BOOL setCutoff_i24(UCHAR bitcrush);

		BOOL runDSP_i16(VOID);
		BOOL runDSP_i24(VOID);

		VOID dsp_loop_i16(VOID);
		VOID dsp_loop_i24(UCHAR *bytebuf);
};

#endif //AUDIOBITCRUSH_HPP
