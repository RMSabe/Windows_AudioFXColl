/*
	Audio FX Collection version 1.0 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioBitCrush.hpp"

AudioBitCrush::AudioBitCrush(VOID) : AudioBaseClass()
{
}

AudioBitCrush::AudioBitCrush(std::string filein_dir) : AudioBaseClass(filein_dir)
{
}

AudioBitCrush::AudioBitCrush(std::string filein_dir, std::string fileout_dir) : AudioBaseClass(filein_dir, fileout_dir)
{
}

AudioBitCrush::~AudioBitCrush(VOID)
{
	if(this->buffer != nullptr) std::free(this->buffer);

	this->filein_close();
	this->fileout_close();
	this->filetemp_close();
}

BOOL AudioBitCrush::setCutoff(UCHAR bitcrush)
{
	switch(this->format)
	{
		case this->FORMAT_I16:
			return this->setCutoff_i16(bitcrush);

		case this->FORMAT_I24:
			return this->setCutoff_i24(bitcrush);
	}

	return FALSE;
}

BOOL AudioBitCrush::runDSP(VOID)
{
	if(this->status < 1) return FALSE;

	if(this->filein == INVALID_HANDLE_VALUE)
	{
		this->status = this->STATUS_ERROR_NOFILE;
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

BOOL AudioBitCrush::setCutoff_i16(UCHAR bitcrush)
{
	if(bitcrush == 0u || bitcrush >= 15u)
	{
		this->error_msg = "Bit crush exceeds sample limit.";
		return FALSE;
	}

	this->cutoff = 0;

	UCHAR b = 0u;
	while(b < bitcrush)
	{
		this->cutoff |= (1 << b);
		b++;
	}

	return TRUE;
}

BOOL AudioBitCrush::setCutoff_i24(UCHAR bitcrush)
{
	if(bitcrush == 0u || bitcrush >= 23u)
	{
		this->error_msg = "Bit crush exceeds sample limit.";
		return FALSE;
	}

	this->cutoff = 0;

	UCHAR b = 0u;
	while(b < bitcrush)
	{
		this->cutoff |= (1 << b);
		b++;
	}

	return TRUE;
}

BOOL AudioBitCrush::runDSP_i16(VOID)
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

BOOL AudioBitCrush::runDSP_i24(VOID)
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

VOID AudioBitCrush::dsp_loop_i16(VOID)
{
	SHORT *buffer_i16 = (SHORT*) this->buffer;
	SIZE_T n_sample = 0u;

	while(TRUE)
	{
		if(this->filein_pos_64 >= this->audio_data_end) break;

		ZeroMemory(buffer_i16, this->BUFFER_SIZE_BYTES);

		SetFilePointer(this->filein, (LONG) *this->filein_ppos_l32, (LONG*) this->filein_ppos_h32, FILE_BEGIN);
		ReadFile(this->filein, buffer_i16, this->BUFFER_SIZE_BYTES, NULL, NULL);
		this->filein_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;

		for(n_sample = 0u; n_sample < this->BUFFER_SIZE_SAMPLES; n_sample++) buffer_i16[n_sample] &= ~((SHORT) this->cutoff);

		SetFilePointer(this->filetemp, (LONG) *this->filetemp_ppos_l32, (LONG*) this->filetemp_ppos_h32, FILE_BEGIN);
		WriteFile(this->filetemp, buffer_i16, this->BUFFER_SIZE_BYTES, NULL, NULL);
		this->filetemp_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;
	}

	return;
}

VOID AudioBitCrush::dsp_loop_i24(UCHAR *bytebuf)
{
	INT *buffer_i32 = (INT*) this->buffer;
	SIZE_T n_sample = 0u;
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
			buffer_i32[n_sample] = ((bytebuf[n_byte + 2u] << 16) | (bytebuf[n_byte + 1u] << 8) | (bytebuf[n_byte]));

			if(buffer_i32[n_sample] & 0x00800000) buffer_i32[n_sample] |= 0xff800000;
			else buffer_i32[n_sample] &= 0x007fffff; //Not really necessary, but just to be safe.

			buffer_i32[n_sample] &= ~this->cutoff;

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
