/*
	Audio FX Collection version 1.2 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioReverse.hpp"

AudioReverse::AudioReverse(VOID) : AudioBaseClass()
{
}

AudioReverse::AudioReverse(__string filein_dir) : AudioBaseClass(filein_dir)
{
}

AudioReverse::AudioReverse(__string filein_dir, __string fileout_dir) : AudioBaseClass(filein_dir, fileout_dir)
{
}

AudioReverse::~AudioReverse(VOID)
{
	this->filein_close();
	this->fileout_close();
	this->filetemp_close();

	this->buffer_free();
}

BOOL WINAPI AudioReverse::runDSP(VOID)
{
	if(this->status < 1) return FALSE;

	if(this->h_filein == INVALID_HANDLE_VALUE)
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

BOOL WINAPI AudioReverse::buffer_alloc(SIZE_T size)
{
	this->p_buffer_input = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, size);
	this->p_buffer_output = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, size);

	if(this->p_buffer_input == nullptr) return FALSE;
	if(this->p_buffer_output == nullptr) return FALSE;

	return TRUE;
}

VOID WINAPI AudioReverse::buffer_free(VOID)
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

BOOL WINAPI AudioReverse::runDSP_i16(VOID)
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

	this->dsp_init_i16();
	this->dsp_loop_i16();

	this->buffer_free();

	this->filetemp_close();

	return this->rawtowav_proc();
}

BOOL WINAPI AudioReverse::runDSP_i24(VOID)
{
	BYTE *p_bytebuf = nullptr;

	if(!this->filetemp_create())
	{
		this->error_msg = TEXT("Could not create temporary DSP file.");
		return FALSE;
	}

	UCHAR *bytebuf = (UCHAR*) std::malloc(this->BUFFER_SIZE_BYTES);

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

	this->dsp_init_i24(p_bytebuf);
	this->dsp_loop_i24(p_bytebuf);

	HeapFree(p_processheap, 0, p_bytebuf);

	this->buffer_free();

	this->filetemp_close();

	return this->rawtowav_proc();
}

VOID WINAPI AudioReverse::dsp_init_i16(VOID)
{
	const ULONG64 AUDIO_DATALENGTH_BYTES = this->audio_data_end - this->audio_data_begin;
	const ULONG64 AUDIO_DATALENGTH_SAMPLES = AUDIO_DATALENGTH_BYTES/2u;
	const ULONG64 AUDIO_DATALENGTH_FRAMES = AUDIO_DATALENGTH_SAMPLES/((ULONG64) this->n_channels);

	const SIZE_T N_FRAMES_REMAINING = (SIZE_T) (AUDIO_DATALENGTH_FRAMES%((ULONG64) this->BUFFER_SIZE_FRAMES));
	const SIZE_T N_SAMPLES_REMAINING = N_FRAMES_REMAINING*((SIZE_T) this->n_channels);
	const SIZE_T N_BYTES_REMAINING = N_SAMPLES_REMAINING*2u;

	INT16 *bufferin_i16 = (INT16*) this->p_buffer_input;
	INT16 *bufferout_i16 = (INT16*) this->p_buffer_output;

	SIZE_T n_frame = 0u;
	SIZE_T n_counterframe = 0u;
	SIZE_T n_sample = 0u;
	SIZE_T n_countersample = 0u;
	SIZE_T n_channel = 0u;

	DWORD dummy_32;

	this->filein_pos_64 = this->audio_data_end - ((ULONG64) N_BYTES_REMAINING);

	SetFilePointer(this->h_filein, (LONG) *this->p_filein_pos_l32, (LONG*) this->p_filein_pos_h32, FILE_BEGIN);
	ReadFile(this->h_filein, bufferin_i16, (DWORD) N_BYTES_REMAINING, &dummy_32, NULL);
	this->filein_pos_64 -= (ULONG64) this->BUFFER_SIZE_BYTES;

	for(n_frame = 0u; n_frame < N_FRAMES_REMAINING; n_frame++)
	{
		n_counterframe = N_FRAMES_REMAINING - n_frame - 1u;
		for(n_channel = 0u; n_channel < ((SIZE_T) this->n_channels); n_channel++)
		{
			n_sample = n_frame*((SIZE_T) this->n_channels) + n_channel;
			n_countersample = n_counterframe*((SIZE_T) this->n_channels) + n_channel;

			bufferout_i16[n_sample] = bufferin_i16[n_countersample];
		}
	}

	SetFilePointer(this->h_filetemp, 0, NULL, FILE_BEGIN);
	WriteFile(this->h_filetemp, bufferout_i16, (DWORD) N_BYTES_REMAINING, &dummy_32, NULL);
	this->filetemp_pos_64 = (ULONG64) N_BYTES_REMAINING;

	return;
}

VOID WINAPI AudioReverse::dsp_init_i24(BYTE *p_bytebuf)
{
	const ULONG64 AUDIO_DATALENGTH_BYTES = this->audio_data_end - this->audio_data_begin;
	const ULONG64 AUDIO_DATALENGTH_SAMPLES = AUDIO_DATALENGTH_BYTES/3u;
	const ULONG64 AUDIO_DATALENGTH_FRAMES = AUDIO_DATALENGTH_SAMPLES/((ULONG64) this->n_channels);

	const SIZE_T N_FRAMES_REMAINING = (SIZE_T) (AUDIO_DATALENGTH_FRAMES%((ULONG64) this->BUFFER_SIZE_FRAMES));
	const SIZE_T N_SAMPLES_REMAINING = N_FRAMES_REMAINING*((SIZE_T) this->n_channels);
	const SIZE_T N_BYTES_REMAINING = N_SAMPLES_REMAINING*3u;

	INT32 *bufferin_i32 = (INT32*) this->p_buffer_input;
	INT32 *bufferout_i32 = (INT32*) this->p_buffer_output;

	SIZE_T n_frame = 0u;
	SIZE_T n_counterframe = 0u;
	SIZE_T n_sample = 0u;
	SIZE_T n_countersample = 0u;
	SIZE_T n_channel = 0u;
	SIZE_T n_byte = 0u;

	DWORD dummy_32;

	this->filein_pos_64 = this->audio_data_end - ((ULONG64) N_BYTES_REMAINING);

	SetFilePointer(this->h_filein, (LONG) *this->p_filein_pos_l32, (LONG*) this->p_filein_pos_h32, FILE_BEGIN);
	ReadFile(this->h_filein, p_bytebuf, (DWORD) N_BYTES_REMAINING, &dummy_32, NULL);
	this->filein_pos_64 -= (ULONG64) this->BUFFER_SIZE_BYTES;

	n_byte = 0u;
	for(n_sample = 0u; n_sample < N_SAMPLES_REMAINING; n_sample++)
	{
		bufferin_i32[n_sample] = ((p_bytebuf[n_byte + 2u] << 16) | (p_bytebuf[n_byte + 1u] << 8) | (p_bytebuf[n_byte]));

		if(bufferin_i32[n_sample] & 0x00800000) bufferin_i32[n_sample] |= 0xff800000;
		else bufferin_i32[n_sample] &= 0x007fffff; //Not really necessary, but just to be safe.

		n_byte += 3u;
	}

	for(n_frame = 0u; n_frame < N_FRAMES_REMAINING; n_frame++)
	{
		n_counterframe = N_FRAMES_REMAINING - n_frame - 1u;
		for(n_channel = 0u; n_channel < ((SIZE_T) this->n_channels); n_channel++)
		{
			n_sample = n_frame*((SIZE_T) this->n_channels) + n_channel;
			n_countersample = n_counterframe*((SIZE_T) this->n_channels) + n_channel;

			bufferout_i32[n_sample] = bufferin_i32[n_countersample];
		}
	}

	n_byte = 0u;
	for(n_sample = 0u; n_sample < N_SAMPLES_REMAINING; n_sample++)
	{
		p_bytebuf[n_byte] = (bufferout_i32[n_sample] & 0xff);
		p_bytebuf[n_byte + 1u] = ((bufferout_i32[n_sample] >> 8) & 0xff);
		p_bytebuf[n_byte + 2u] = ((bufferout_i32[n_sample] >> 16) & 0xff);

		n_byte += 3u;
	}

	SetFilePointer(this->h_filetemp, 0, NULL, FILE_BEGIN);
	WriteFile(this->h_filetemp, p_bytebuf, (DWORD) N_BYTES_REMAINING, &dummy_32, NULL);
	this->filetemp_pos_64 = (ULONG64) N_BYTES_REMAINING;

	return;
}

VOID WINAPI AudioReverse::dsp_loop_i16(VOID)
{
	INT16 *bufferin_i16 = (INT16*) this->p_buffer_input;
	INT16 *bufferout_i16 = (INT16*) this->p_buffer_output;

	SIZE_T n_frame = 0u;
	SIZE_T n_counterframe = 0u;
	SIZE_T n_sample = 0u;
	SIZE_T n_countersample = 0u;
	SIZE_T n_channel = 0u;

	DWORD dummy_32;

	while(this->filein_pos_64 >= this->audio_data_begin)
	{
		ZeroMemory(bufferin_i16, this->BUFFER_SIZE_BYTES);

		SetFilePointer(this->h_filein, (LONG) *this->p_filein_pos_l32, (LONG*) this->p_filein_pos_h32, FILE_BEGIN);
		ReadFile(this->h_filein, bufferin_i16, (DWORD) this->BUFFER_SIZE_BYTES, &dummy_32, NULL);

		if(this->filein_pos_64 <= ((ULONG64) this->BUFFER_SIZE_BYTES)) this->filein_pos_64 = 0u;
		else this->filein_pos_64 -= (ULONG64) this->BUFFER_SIZE_BYTES;

		for(n_frame = 0u; n_frame < this->BUFFER_SIZE_FRAMES; n_frame++)
		{
			n_counterframe = this->BUFFER_SIZE_FRAMES - n_frame - 1u;
			for(n_channel = 0u; n_channel < ((SIZE_T) this->n_channels); n_channel++)
			{
				n_sample = n_frame*((SIZE_T) this->n_channels) + n_channel;
				n_countersample = n_counterframe*((SIZE_T) this->n_channels) + n_channel;

				bufferout_i16[n_sample] = bufferin_i16[n_countersample];
			}
		}

		SetFilePointer(this->h_filetemp, (LONG) *this->p_filetemp_pos_l32, (LONG*) this->p_filetemp_pos_h32, FILE_BEGIN);
		WriteFile(this->h_filetemp, bufferout_i16, (DWORD) this->BUFFER_SIZE_BYTES, &dummy_32, NULL);
		this->filetemp_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;
	}

	return;
}

VOID WINAPI AudioReverse::dsp_loop_i24(BYTE *p_bytebuf)
{
	INT32 *bufferin_i32 = (INT32*) this->p_buffer_input;
	INT32 *bufferout_i32 = (INT32*) this->p_buffer_output;

	SIZE_T n_frame = 0u;
	SIZE_T n_counterframe = 0u;
	SIZE_T n_sample = 0u;
	SIZE_T n_countersample = 0u;
	SIZE_T n_channel = 0u;
	SIZE_T n_byte = 0u;

	DWORD dummy_32;

	while(this->filein_pos_64 >= this->audio_data_begin)
	{
		ZeroMemory(p_bytebuf, this->BUFFER_SIZE_BYTES);

		SetFilePointer(this->h_filein, (LONG) *this->p_filein_pos_l32, (LONG*) this->p_filein_pos_h32, FILE_BEGIN);
		ReadFile(this->h_filein, p_bytebuf, (DWORD) this->BUFFER_SIZE_BYTES, &dummy_32, NULL);

		if(this->filein_pos_64 <= ((ULONG64) this->BUFFER_SIZE_BYTES)) this->filein_pos_64 = 0u;
		else this->filein_pos_64 -= (ULONG64) this->BUFFER_SIZE_BYTES;

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
			n_counterframe = this->BUFFER_SIZE_FRAMES - n_frame - 1u;
			for(n_channel = 0u; n_channel < ((SIZE_T) this->n_channels); n_channel++)
			{
				n_sample = n_frame*((SIZE_T) this->n_channels) + n_channel;
				n_countersample = n_counterframe*((SIZE_T) this->n_channels) + n_channel;

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
