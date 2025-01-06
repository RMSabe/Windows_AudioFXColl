/*
	Audio FX Collection version 1.2 for Windows

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
	this->filein_close();
	this->fileout_close();
	this->filetemp_close();

	this->buffer_free();
}

BOOL WINAPI AudioChannelSwap::runDSP(VOID)
{
	if(this->status < 1) return FALSE;

	if(this->h_filein == INVALID_HANDLE_VALUE)
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

BOOL WINAPI AudioChannelSwap::buffer_alloc(SIZE_T size)
{
	this->p_buffer_input = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, size);
	this->p_buffer_output = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, size);

	if(this->p_buffer_input == nullptr) return FALSE;
	if(this->p_buffer_output == nullptr) return FALSE;

	return TRUE;
}

VOID WINAPI AudioChannelSwap::buffer_free(VOID)
{
	if(this->p_buffer_input != nullptr)
	{
		HeapFree(p_processheap, 0, this->p_buffer_input);
		this->p_buffer_input = nullptr;
	}

	if(this->p_buffer_output != nullptr)
	{
		HeapFree(p_processheap, 0, this->p_buffer_output);
		this->p_buffer_output = nullptr;
	}

	return;
}

BOOL WINAPI AudioChannelSwap::runDSP_i16(VOID)
{
	if(!this->filetemp_create())
	{
		this->error_msg = TEXT("Could not create temporary DSP file.");
		return FALSE;
	}

	if(!this->buffer_alloc(this->BUFFER_SIZE_BYTES))
	{
		this->buffer_free();

		this->error_msg = TEXT("Heap Allocate Failed.");
		return FALSE;
	}

	this->filein_pos_64 = this->audio_data_begin;
	this->filetemp_pos_64 = 0u;

	this->dsp_loop_i16();

	this->buffer_free();

	this->filetemp_close();

	return this->rawtowav_proc();
}

BOOL WINAPI AudioChannelSwap::runDSP_i24(VOID)
{
	BYTE *p_bytebuf = nullptr;

	if(!this->filetemp_create())
	{
		this->error_msg = TEXT("Could not create temporary DSP file.");
		return FALSE;
	}

	p_bytebuf = (BYTE*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BUFFER_SIZE_BYTES);
	if(p_bytebuf == nullptr)
	{
		this->error_msg = TEXT("Heap Allocate Failed.");
		return FALSE;
	}

	if(!this->buffer_alloc(this->BUFFER_SIZE_SAMPLES*4u))
	{
		HeapFree(p_processheap, 0, p_bytebuf);
		this->buffer_free();

		this->error_msg = TEXT("Heap Allocate Failed.");
		return FALSE;
	}

	this->filein_pos_64 = this->audio_data_begin;
	this->filetemp_pos_64 = 0u;

	this->dsp_loop_i24(p_bytebuf);

	HeapFree(p_processheap, 0, p_bytebuf);

	this->buffer_free();

	this->filetemp_close();

	return this->rawtowav_proc();
}

VOID WINAPI AudioChannelSwap::dsp_loop_i16(VOID)
{
	INT16 *bufferin_i16 = (INT16*) this->p_buffer_input;
	INT16 *bufferout_i16 = (INT16*) this->p_buffer_output;

	SIZE_T n_frame = 0u;
	SIZE_T n_sample = 0u;
	SIZE_T n_countersample = 0u;
	SIZE_T n_channel = 0u;
	SIZE_T n_counterchannel = 0u;

	DWORD dummy_32;

	while(this->filein_pos_64 < this->audio_data_end)
	{
		ZeroMemory(bufferin_i16, this->BUFFER_SIZE_BYTES);

		SetFilePointer(this->h_filein, (LONG) *this->p_filein_pos_l32, (LONG*) this->p_filein_pos_h32, FILE_BEGIN);
		ReadFile(this->h_filein, bufferin_i16, (DWORD) this->BUFFER_SIZE_BYTES, &dummy_32, NULL);
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

		SetFilePointer(this->h_filetemp, (LONG) *this->p_filetemp_pos_l32, (LONG*) this->p_filetemp_pos_h32, FILE_BEGIN);
		WriteFile(this->h_filetemp, bufferout_i16, (DWORD) this->BUFFER_SIZE_BYTES, &dummy_32, NULL);
		this->filetemp_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;
	}

	return;
}

VOID WINAPI AudioChannelSwap::dsp_loop_i24(BYTE *p_bytebuf)
{
	INT32 *bufferin_i32 = (INT32*) this->p_buffer_input;
	INT32 *bufferout_i32 = (INT32*) this->p_buffer_output;

	SIZE_T n_frame = 0u;
	SIZE_T n_sample = 0u;
	SIZE_T n_countersample = 0u;
	SIZE_T n_channel = 0u;
	SIZE_T n_counterchannel = 0u;
	SIZE_T n_byte = 0u;

	DWORD dummy_32;

	while(this->filein_pos_64 < this->audio_data_end)
	{
		ZeroMemory(p_bytebuf, this->BUFFER_SIZE_BYTES);

		SetFilePointer(this->h_filein, (LONG) *this->p_filein_pos_l32, (LONG*) this->p_filein_pos_h32, FILE_BEGIN);
		ReadFile(this->h_filein, p_bytebuf, (DWORD) this->BUFFER_SIZE_BYTES, &dummy_32, NULL);
		this->filein_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;

		n_byte = 0u;
		for(n_sample = 0u; n_sample < this->BUFFER_SIZE_SAMPLES; n_sample++)
		{
			bufferin_i32[n_sample] = ((p_bytebuf[n_byte + 2u] << 16) | (p_bytebuf[n_byte + 1u] << 8) | (p_bytebuf[n_byte]));

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
			p_bytebuf[n_byte] = (bufferout_i32[n_sample] & 0xff);
			p_bytebuf[n_byte + 1u] = ((bufferout_i32[n_sample] >> 8) & 0xff);
			p_bytebuf[n_byte + 2u] = ((bufferout_i32[n_sample] >> 16) & 0xff);

			n_byte += 3u;
		}

		SetFilePointer(this->h_filetemp, (LONG) *this->p_filetemp_pos_l32, (LONG*) this->p_filetemp_pos_h32, FILE_BEGIN);
		WriteFile(this->h_filetemp, p_bytebuf, (DWORD) this->BUFFER_SIZE_BYTES, &dummy_32, NULL);
		this->filetemp_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;
	}

	return;
}
