/*
	Audio FX Collection version 1.2 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOCHANNELSUBTRACT_HPP
#define AUDIOCHANNELSUBTRACT_HPP

#include "AudioBaseClass.hpp"

class AudioChannelSubtract : public AudioBaseClass {
	public:
		AudioChannelSubtract(VOID);
		AudioChannelSubtract(__string filein_dir);
		AudioChannelSubtract(__string filein_dir, __string fileout_dir);
		~AudioChannelSubtract(VOID);

		BOOL WINAPI runDSP(VOID) override;

	private:
		VOID *p_buffer = nullptr;

		BOOL WINAPI buffer_alloc(SIZE_T size) override;
		VOID WINAPI buffer_free(VOID) override;

		BOOL WINAPI runDSP_i16(VOID);
		BOOL WINAPI runDSP_i24(VOID);

		VOID WINAPI dsp_loop_i16(VOID);
		VOID WINAPI dsp_loop_i24(BYTE *p_bytebuf);
};

#endif //AUDIOCHANNELSUBTRACT_HPP
