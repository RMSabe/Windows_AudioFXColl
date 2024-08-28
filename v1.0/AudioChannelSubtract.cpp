/*
	Audio FX Collection version 1.0 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioChannelSubtract.hpp"

AudioChannelSubtract::AudioChannelSubtract(VOID) : AudioBaseClass()
{
}

AudioChannelSubtract::AudioChannelSubtract(std::string filein_dir) : AudioBaseClass(filein_dir)
{
}

AudioChannelSubtract::AudioChannelSubtract(std::string filein_dir, std::string fileout_dir) : AudioBaseClass(filein_dir, fileout_dir)
{
}

AudioChannelSubtract::~AudioChannelSubtract(VOID)
{
	if(this->buffer != nullptr) std::free(this->buffer);

	this->filein_close();
	this->fileout_close();
	this->filetemp_close();
}

BOOL AudioChannelSubtract::runDSP(VOID)
{
	if(this->status < 1) return FALSE;

	if(this->filein == INVALID_HANDLE_VALUE)
	{
		this->status = this->STATUS_ERROR_NOFILE;
		return FALSE;
	}

	if(this->n_channels < 2u)
	{
		this->error_msg = "This effect cannot be run on single channel audio.";
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

BOOL AudioChannelSubtract::runDSP_i16(VOID)
{
	if(!this->filetemp_create())
	{
		this->error_msg = "Could not create temporary DSP file.";
		return FALSE;
	}

	this->buffer = std::malloc(this->BUFFER_SIZE_BYTES);

	this->filein_pos_64 = this->audio_data_begin;
	this->filetemp_pos_64 = 0u;

	this->dsp_loop_i16();

	std::free(this->buffer);
	this->buffer = nullptr;

	this->filetemp_close();

	return this->rawtowav_proc();
}

BOOL AudioChannelSubtract::runDSP_i24(VOID)
{
	if(!this->filetemp_create())
	{
		this->error_msg = "Could not create temporary DSP file.";
		return FALSE;
	}

	UCHAR *bytebuf = (UCHAR*) std::malloc(this->BUFFER_SIZE_BYTES);
	this->buffer = std::malloc(this->BUFFER_SIZE_SAMPLES*sizeof(INT));

	this->filein_pos_64 = this->audio_data_begin;
	this->filetemp_pos_64 = 0u;

	this->dsp_loop_i24(bytebuf);

	std::free(bytebuf);

	std::free(this->buffer);
	this->buffer = nullptr;

	this->filetemp_close();

	return this->rawtowav_proc();
}

VOID AudioChannelSubtract::dsp_loop_i16(VOID)
{
	SHORT *buffer_i16 = (SHORT*) this->buffer;

	SIZE_T n_frame = 0u;
	SIZE_T n_sample = 0u;
	SIZE_T n_channel = 0u;

	const INT SAMPLE_MAX_VALUE = 0x7fff;
	const INT SAMPLE_MIN_VALUE = -0x8000;

	INT mono_sample = 0;
	INT channel_sample = 0;

	while(TRUE)
	{
		if(this->filein_pos_64 >= this->audio_data_end) break;

		ZeroMemory(buffer_i16, this->BUFFER_SIZE_BYTES);

		SetFilePointer(this->filein, (LONG) *this->filein_ppos_l32, (LONG*) this->filein_ppos_h32, FILE_BEGIN);
		ReadFile(this->filein, buffer_i16, this->BUFFER_SIZE_BYTES, NULL, NULL);
		this->filein_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;

		for(n_frame = 0u; n_frame < this->BUFFER_SIZE_FRAMES; n_frame++)
		{
			mono_sample = 0;
			for(n_channel = 0u; n_channel < ((SIZE_T) this->n_channels); n_channel++)
			{
				n_sample = n_frame*((SIZE_T) this->n_channels) + n_channel;
				mono_sample += (INT) buffer_i16[n_sample];
			}

			for(n_channel = 0u; n_channel < ((SIZE_T) this->n_channels); n_channel++)
			{
				n_sample = n_frame*((SIZE_T) this->n_channels) + n_channel;

				channel_sample = (INT) buffer_i16[n_sample];
				channel_sample *= (INT) this->n_channels;
				channel_sample -= mono_sample;
				channel_sample /= (INT) this->n_channels;

				if(channel_sample >= SAMPLE_MAX_VALUE) buffer_i16[n_sample] = (SHORT) SAMPLE_MAX_VALUE;
				else if(channel_sample <= SAMPLE_MIN_VALUE) buffer_i16[n_sample] = (SHORT) SAMPLE_MIN_VALUE;
				else buffer_i16[n_sample] = (SHORT) channel_sample;
			}
		}

		SetFilePointer(this->filetemp, (LONG) *this->filetemp_ppos_l32, (LONG*) this->filetemp_ppos_h32, FILE_BEGIN);
		WriteFile(this->filetemp, buffer_i16, this->BUFFER_SIZE_BYTES, NULL, NULL);
		this->filetemp_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;
	}

	return;
}

VOID AudioChannelSubtract::dsp_loop_i24(UCHAR *bytebuf)
{
	INT *buffer_i32 = (INT*) this->buffer;

	SIZE_T n_frame = 0u;
	SIZE_T n_sample = 0u;
	SIZE_T n_channel = 0u;
	SIZE_T n_byte = 0u;

	const INT SAMPLE_MAX_VALUE = 0x7fffff;
	const INT SAMPLE_MIN_VALUE = -0x800000;

	INT mono_sample = 0;

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
			buffer_i32[n_sample] = ((bytebuf[n_byte + 2u] << 16) | (bytebuf[n_byte + 1u] << 8) | (bytebuf[n_byte]));

			if(buffer_i32[n_sample] & 0x00800000) buffer_i32[n_sample] |= 0xff800000;
			else buffer_i32[n_sample] &= 0x007fffff; //Not really necessary, but just to be safe.

			n_byte += 3u;
		}

		for(n_frame = 0u; n_frame < this->BUFFER_SIZE_FRAMES; n_frame++)
		{
			mono_sample = 0;

			for(n_channel = 0u; n_channel < ((SIZE_T) this->n_channels); n_channel++)
			{
				n_sample = n_frame*((SIZE_T) this->n_channels) + n_channel;
				mono_sample += buffer_i32[n_sample];
			}

			for(n_channel = 0u; n_channel < ((SIZE_T) this->n_channels); n_channel++)
			{
				n_sample = n_frame*((SIZE_T) this->n_channels) + n_channel;

				buffer_i32[n_sample] *= (INT) this->n_channels;
				buffer_i32[n_sample] -= mono_sample;
				buffer_i32[n_sample] /= (INT) this->n_channels;

				if(buffer_i32[n_sample] >= SAMPLE_MAX_VALUE) buffer_i32[n_sample] = SAMPLE_MAX_VALUE;
				else if(buffer_i32[n_sample] <= SAMPLE_MIN_VALUE) buffer_i32[n_sample] = SAMPLE_MIN_VALUE;
			}
		}

		n_byte = 0u;
		for(n_sample = 0u; n_sample < this->BUFFER_SIZE_SAMPLES; n_sample++)
		{
			bytebuf[n_byte] = (buffer_i32[n_sample] & 0xff);
			bytebuf[n_byte + 1u] = ((buffer_i32[n_sample] >> 8) & 0xff);
			bytebuf[n_byte + 2u] = ((buffer_i32[n_sample] >> 16) & 0xff);

			n_byte += 3u;
		}

		SetFilePointer(this->filetemp, (LONG) *this->filetemp_ppos_l32, (LONG*) this->filetemp_ppos_h32, FILE_BEGIN);
		WriteFile(this->filetemp, bytebuf, this->BUFFER_SIZE_BYTES, NULL, NULL);
		this->filetemp_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;
	}

	return;
}
