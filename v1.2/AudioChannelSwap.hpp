/*
	Audio FX Collection version 1.2 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOCHANNELSWAP_HPP
#define AUDIOCHANNELSWAP_HPP

#include "AudioBaseClass.hpp"

class AudioChannelSwap : public AudioBaseClass {
	public:
		AudioChannelSwap(VOID);
		AudioChannelSwap(__string filein_dir);
		AudioChannelSwap(__string filein_dir, __string fileout_dir);
		~AudioChannelSwap(VOID);

		BOOL WINAPI runDSP(VOID) override;

	private:
		VOID *p_buffer_input = nullptr;
		VOID *p_buffer_output = nullptr;

		BOOL WINAPI buffer_alloc(SIZE_T size) override;
		VOID WINAPI buffer_free(VOID) override;

		BOOL WINAPI runDSP_i16(VOID);
		BOOL WINAPI runDSP_i24(VOID);

		VOID WINAPI dsp_loop_i16(VOID);
		VOID WINAPI dsp_loop_i24(BYTE *p_bytebuf);
};

#endif //AUDIOCHANNELSWAP_HPP
