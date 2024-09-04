/*
	Audio FX Collection version 1.1 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioChannelSwap.hpp"

AudioChannelSwap::AudioChannelSwap(VOID) : AudioBaseClass()
{
}

AudioChannelSwap::AudioChannelSwap(__string filein_dir) : AudioBaseClass(filein_dir)
{
}

AudioChannelSwap::AudioChannelSwap(__string filein_dir, __string fileout_dir) : AudioBaseClass(filein_dir, fileout_dir)
{
}

AudioChannelSwap::~AudioChannelSwap(VOID)
{
	if(this->buffer_input != nullptr) std::free(this->buffer_input);
	if(this->buffer_output != nullptr) std::free(this->buffer_output);

	this->filein_close();
	this->fileout_close();
	this->filetemp_close();
}

BOOL AudioChannelSwap::runDSP(VOID)
{
	if(this->status < 1) return FALSE;

	if(this->filein == INVALID_HANDLE_VALUE)
	{
		this->status = this->STATUS_ERROR_NOFILE;
		return FALSE;
	}

	if(this->n_channels < 2u)
	{
		this->error_msg = TEXT("This audio effect cannot be run on single channel audio.");
		return FALSE;
	}

	switch(this->format)
	{
		case this->FORMAT_I16:
			return this->runDSP_i16();

		case this->FORMAT_I24:
			return this->runDSP_i24();
	}

	return FALSE;
}

BOOL AudioChannelSwap::runDSP_i16(VOID)
{
	if(!this->filetemp_create())
	{
		this->error_msg = TEXT("Could not create temporary DSP file.");
		return FALSE;
	}

	this->buffer_input = std::malloc(this->BUFFER_SIZE_BYTES);
	this->buffer_output = std::malloc(this->BUFFER_SIZE_BYTES);

	this->filein_pos_64 = this->audio_data_begin;
	this->filetemp_pos_64 = 0u;

	this->dsp_loop_i16();

	std::free(this->buffer_input);
	std::free(this->buffer_output);

	this->buffer_input = nullptr;
	this->buffer_output = nullptr;

	this->filetemp_close();

	return this->rawtowav_proc();
}

BOOL AudioChannelSwap::runDSP_i24(VOID)
{
	if(!this->filetemp_create())
	{
		this->error_msg = TEXT("Could not create temporary DSP file.");
		return FALSE;
	}

	UCHAR *bytebuf = (UCHAR*) std::malloc(this->BUFFER_SIZE_BYTES);
	this->buffer_input = std::malloc(this->BUFFER_SIZE_SAMPLES*sizeof(INT));
	this->buffer_output = std::malloc(this->BUFFER_SIZE_SAMPLES*sizeof(INT));

	this->filein_pos_64 = this->audio_data_begin;
	this->filetemp_pos_64 = 0u;

	this->dsp_loop_i24(bytebuf);

	std::free(bytebuf);

	std::free(this->buffer_input);
	std::free(this->buffer_output);

	this->buffer_input = nullptr;
	this->buffer_output = nullptr;

	this->filetemp_close();

	return this->rawtowav_proc();
}

VOID AudioChannelSwap::dsp_loop_i16(VOID)
{
	SHORT *bufferin_i16 = (SHORT*) this->buffer_input;
	SHORT *bufferout_i16 = (SHORT*) this->buffer_output;

	SIZE_T n_frame = 0u;
	SIZE_T n_sample = 0u;
	SIZE_T n_countersample = 0u;
	SIZE_T n_channel = 0u;
	SIZE_T n_counterchannel = 0u;

	while(TRUE)
	{
		if(this->filein_pos_64 >= this->audio_data_end) break;

		ZeroMemory(bufferin_i16, this->BUFFER_SIZE_BYTES);

		SetFilePointer(this->filein, (LONG) *this->filein_ppos_l32, (LONG*) this->filein_ppos_h32, FILE_BEGIN);
		ReadFile(this->filein, bufferin_i16, this->BUFFER_SIZE_BYTES, NULL, NULL);
		this->filein_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;

		for(n_frame = 0u; n_frame < this->BUFFER_SIZE_FRAMES; n_frame++)
		{
			for(n_channel = 0u; n_channel < ((SIZE_T) this->n_channels); n_channel++)
			{
				n_counterchannel = ((SIZE_T) this->n_channels) - n_channel - 1u;

				n_sample = n_frame*((SIZE_T) this->n_channels) + n_channel;
				n_countersample = n_frame*((SIZE_T) this->n_channels) + n_counterchannel;

				bufferout_i16[n_sample] = bufferin_i16[n_countersample];
			}
		}

		SetFilePointer(this->filetemp, (LONG) *this->filetemp_ppos_l32, (LONG*) this->filetemp_ppos_h32, FILE_BEGIN);
		WriteFile(this->filetemp, bufferout_i16, this->BUFFER_SIZE_BYTES, NULL, NULL);
		this->filetemp_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;
	}

	return;
}

VOID AudioChannelSwap::dsp_loop_i24(UCHAR *bytebuf)
{
	INT *bufferin_i32 = (INT*) this->buffer_input;
	INT *bufferout_i32 = (INT*) this->buffer_output;

	SIZE_T n_frame = 0u;
	SIZE_T n_sample = 0u;
	SIZE_T n_countersample = 0u;
	SIZE_T n_channel = 0u;
	SIZE_T n_counterchannel = 0u;
	SIZE_T n_byte = 0u;

	while(TRUE)
	{
		if(this->filein_pos_64 >= this->audio_data_end) break;

		ZeroMemory(bytebuf, this->BUFFER_SIZE_BYTES);

		SetFilePointer(this->filein, (LONG) *this->filein_ppos_l32, (LONG*) this->filein_ppos_h32, FILE_BEGIN);
		ReadFile(this->filein, bytebuf, this->BUFFER_SIZE_BYTES, NULL, NULL);
		this->filein_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;

		n_byte = 0u;
		for(n_sample = 0u; n_sample < this->BUFFER_SIZE_SAMPLES; n_sample++)
		{
			bufferin_i32[n_sample] = ((bytebuf[n_byte + 2u] << 16) | (bytebuf[n_byte + 1u] << 8) | (bytebuf[n_byte]));

			if(bufferin_i32[n_sample] & 0x00800000) bufferin_i32[n_sample] |= 0xff800000;
			else bufferin_i32[n_sample] &= 0x007fffff; //Not really necessary, but just to be safe.

			n_byte += 3u;
		}

		for(n_frame = 0u; n_frame < this->BUFFER_SIZE_FRAMES; n_frame++)
		{
			for(n_channel = 0u; n_channel < ((SIZE_T) this->n_channels); n_channel++)
			{
				n_counterchannel = ((SIZE_T) this->n_channels) - n_channel - 1u;

				n_sample = n_frame*((SIZE_T) this->n_channels) + n_channel;
				n_countersample = n_frame*((SIZE_T) this->n_channels) + n_counterchannel;

				bufferout_i32[n_sample] = bufferin_i32[n_countersample];
			}
		}

		n_byte = 0u;
		for(n_sample = 0u; n_sample < this->BUFFER_SIZE_SAMPLES; n_sample++)
		{
			bytebuf[n_byte] = (bufferout_i32[n_sample] & 0xff);
			bytebuf[n_byte + 1u] = ((bufferout_i32[n_sample] >> 8) & 0xff);
			bytebuf[n_byte + 2u] = ((bufferout_i32[n_sample] >> 16) & 0xff);

			n_byte += 3u;
		}

		SetFilePointer(this->filetemp, (LONG) *this->filetemp_ppos_l32, (LONG*) this->filetemp_ppos_h32, FILE_BEGIN);
		WriteFile(this->filetemp, bytebuf, this->BUFFER_SIZE_BYTES, NULL, NULL);
		this->filetemp_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;
	}

	return;
}
