/*
	Audio FX Collection version 1.2 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioBitCrush.hpp"

AudioBitCrush::AudioBitCrush(VOID) : AudioBaseClass()
{
}

AudioBitCrush::AudioBitCrush(__string filein_dir) : AudioBaseClass(filein_dir)
{
}

AudioBitCrush::AudioBitCrush(__string filein_dir, __string fileout_dir) : AudioBaseClass(filein_dir, fileout_dir)
{
}

AudioBitCrush::~AudioBitCrush(VOID)
{
	this->filein_close();
	this->fileout_close();
	this->filetemp_close();

	this->buffer_free();
}

BOOL WINAPI AudioBitCrush::setCutoff(BYTE bitcrush)
{
	BYTE limit = 0u;
	BYTE b = 0u;

	switch(this->format)
	{
		case this->FORMAT_I16:
			limit = 15u;
			break;

		case this->FORMAT_I24:
			limit = 23u;
			break;

		default:
			return FALSE;
	}

	if(bitcrush >= limit)
	{
		this->error_msg = TEXT("Bit crush exceeds sample limit.");
		return FALSE;
	}

	this->cutoff = 0;

	b = 0u;
	while(b < bitcrush)
	{
		this->cutoff |= (1 << b);
		b++;
	}

	return TRUE;
}

BOOL WINAPI AudioBitCrush::runDSP(VOID)
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

BOOL WINAPI AudioBitCrush::buffer_alloc(SIZE_T size)
{
	this->p_buffer = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, size);

	return (this->p_buffer != nullptr);
}

VOID WINAPI AudioBitCrush::buffer_free(VOID)
{
	if(this->p_buffer != nullptr)
	{
		HeapFree(p_processheap, 0, this->p_buffer);
		this->p_buffer = nullptr;
	}

	return;
}

BOOL WINAPI AudioBitCrush::runDSP_i16(VOID)
{
	if(!this->filetemp_create())
	{
		this->error_msg = TEXT("Could not create temporary DSP file.");
		return FALSE;
	}

	if(!this->buffer_alloc(this->BUFFER_SIZE_BYTES))
	{
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

BOOL WINAPI AudioBitCrush::runDSP_i24(VOID)
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

VOID WINAPI AudioBitCrush::dsp_loop_i16(VOID)
{
	INT16 *buffer_i16 = (INT16*) this->p_buffer;
	SIZE_T n_sample = 0u;
	DWORD dummy_32;

	while(this->filein_pos_64 < this->audio_data_end)
	{
		ZeroMemory(buffer_i16, this->BUFFER_SIZE_BYTES);

		SetFilePointer(this->h_filein, (LONG) *this->p_filein_pos_l32, (LONG*) this->p_filein_pos_h32, FILE_BEGIN);
		ReadFile(this->h_filein, buffer_i16, (DWORD) this->BUFFER_SIZE_BYTES, &dummy_32, NULL);
		this->filein_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;

		for(n_sample = 0u; n_sample < this->BUFFER_SIZE_SAMPLES; n_sample++) buffer_i16[n_sample] &= ~((INT16) this->cutoff);

		SetFilePointer(this->h_filetemp, (LONG) *this->p_filetemp_pos_l32, (LONG*) this->p_filetemp_pos_h32, FILE_BEGIN);
		WriteFile(this->h_filetemp, buffer_i16, (DWORD) this->BUFFER_SIZE_BYTES, &dummy_32, NULL);
		this->filetemp_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;
	}

	return;
}

VOID WINAPI AudioBitCrush::dsp_loop_i24(BYTE *p_bytebuf)
{
	INT32 *buffer_i32 = (INT32*) this->p_buffer;
	SIZE_T n_sample = 0u;
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
			buffer_i32[n_sample] = ((p_bytebuf[n_byte + 2u] << 16) | (p_bytebuf[n_byte + 1u] << 8) | (p_bytebuf[n_byte]));

			if(buffer_i32[n_sample] & 0x00800000) buffer_i32[n_sample] |= 0xff800000;
			else buffer_i32[n_sample] &= 0x007fffff; //Not really necessary, but just to be safe.

			buffer_i32[n_sample] &= ~this->cutoff;

			p_bytebuf[n_byte] = (buffer_i32[n_sample] & 0xff);
			p_bytebuf[n_byte + 1u] = ((buffer_i32[n_sample] >> 8) & 0xff);
			p_bytebuf[n_byte + 2u] = ((buffer_i32[n_sample] >> 16) & 0xff);

			n_byte += 3u;
		}

		SetFilePointer(this->h_filetemp, (LONG) *this->p_filetemp_pos_l32, (LONG*) this->p_filetemp_pos_h32, FILE_BEGIN);
		WriteFile(this->h_filetemp, p_bytebuf, (DWORD) this->BUFFER_SIZE_BYTES, &dummy_32, NULL);
		this->filetemp_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;
	}

	return;
}
