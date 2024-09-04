/*
	Audio FX Collection version 1.1 for Windows

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

		BOOL runDSP(VOID) override;

	private:
		VOID *buffer_input = nullptr;
		VOID *buffer_output = nullptr;

		BOOL runDSP_i16(VOID);
		BOOL runDSP_i24(VOID);

		VOID dsp_loop_i16(VOID);
		VOID dsp_loop_i24(UCHAR *bytebuf);
};

#endif //AUDIOCHANNELSWAP_HPP
