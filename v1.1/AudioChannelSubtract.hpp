/*
	Audio FX Collection version 1.1 for Windows

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

		BOOL runDSP(VOID) override;

	private:
		VOID *buffer = nullptr;

		BOOL runDSP_i16(VOID);
		BOOL runDSP_i24(VOID);

		VOID dsp_loop_i16(VOID);
		VOID dsp_loop_i24(UCHAR *bytebuf);
};

#endif //AUDIOCHANNELSUBTRACT_HPP
