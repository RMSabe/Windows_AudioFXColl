/*
	Audio FX Collection version 1.2 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioBaseClass.hpp"

AudioBaseClass::AudioBaseClass(VOID)
{
	this->status = this->STATUS_UNINITIALIZED;
	this->fileout_dir = this->FILEOUT_DIR_DEFAULT;
}

AudioBaseClass::AudioBaseClass(__string filein_dir) : AudioBaseClass()
{
	this->filein_dir = filein_dir;
}

AudioBaseClass::AudioBaseClass(__string filein_dir, __string fileout_dir) : AudioBaseClass(filein_dir)
{
	this->fileout_dir = fileout_dir;
}

BOOL WINAPI AudioBaseClass::initialize(VOID)
{
	if(p_processheap == nullptr)
	{
		this->error_msg = TEXT("Invalid Process Heap.");
		this->status = this->STATUS_ERROR_GENERIC;
		return FALSE;
	}

	if(!this->file_ext_check(this->filein_dir.c_str()))
	{
		this->status = this->STATUS_ERROR_FILENOTSUPPORTED;
		return FALSE;
	}

	if(!this->filein_open())
	{
		this->status = this->STATUS_ERROR_NOFILE;
		return FALSE;
	}

	if(!this->filein_get_params())
	{
		this->filein_close();
		return FALSE;
	}

	this->BUFFER_SIZE_SAMPLES = this->BUFFER_SIZE_FRAMES*((SIZE_T) this->n_channels);
	this->BUFFER_SIZE_BYTES = this->BUFFER_SIZE_SAMPLES*((SIZE_T) (this->bit_depth/8u));

	return TRUE;
}

__string WINAPI AudioBaseClass::getLastErrorMessage(VOID)
{
	switch(this->status)
	{
		case this->STATUS_ERROR_BROKENHEADER:
			return TEXT("File header is missing information (probably corrupted).");

		case this->STATUS_ERROR_FORMATNOTSUPPORTED:
			return TEXT("Audio format is not supported.");

		case this->STATUS_ERROR_FILENOTSUPPORTED:
			return TEXT("File format is not supported.");

		case this->STATUS_ERROR_NOFILE:
			return TEXT("File does not exist, or it's not accessible.");

		case this->STATUS_ERROR_GENERIC:
			return (TEXT("Something went wrong.\r\nError Message: ") + this->error_msg);

		case this->STATUS_UNINITIALIZED:
			return TEXT("Audio object not initialized.");
	}

	return this->error_msg;
}

UINT32 WINAPI AudioBaseClass::getSampleRate(VOID)
{
	return this->sample_rate;
}

UINT16 WINAPI AudioBaseClass::getBitDepth(VOID)
{
	return this->bit_depth;
}

UINT16 WINAPI AudioBaseClass::getNumberChannels(VOID)
{
	return this->n_channels;
}

BOOL WINAPI AudioBaseClass::filein_open(VOID)
{
	this->h_filein = CreateFile(this->filein_dir.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if(this->h_filein == INVALID_HANDLE_VALUE) return FALSE;

	*this->p_filein_size_l32 = (ULONG32) GetFileSize(this->h_filein, (DWORD*) this->p_filein_size_h32);
	return TRUE;
}

VOID WINAPI AudioBaseClass::filein_close(VOID)
{
	if(this->h_filein == INVALID_HANDLE_VALUE) return;

	CloseHandle(this->h_filein);
	this->h_filein = INVALID_HANDLE_VALUE;
	this->filein_size_64 = 0u;
	return;
}

BOOL WINAPI AudioBaseClass::fileout_create(VOID)
{
	this->h_fileout = CreateFile(this->fileout_dir.c_str(), (GENERIC_READ | GENERIC_WRITE), FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, 0);
	return (this->h_fileout != INVALID_HANDLE_VALUE);
}

VOID WINAPI AudioBaseClass::fileout_close(VOID)
{
	if(this->h_fileout == INVALID_HANDLE_VALUE) return;

	CloseHandle(this->h_fileout);
	this->h_fileout = INVALID_HANDLE_VALUE;
	return;
}

BOOL WINAPI AudioBaseClass::filetemp_create(VOID)
{
	this->h_filetemp = CreateFile(this->FILETEMP_DIR, (GENERIC_READ | GENERIC_WRITE), FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, 0);
	return (this->h_filetemp != INVALID_HANDLE_VALUE);
}

BOOL WINAPI AudioBaseClass::filetemp_open(VOID)
{
	this->h_filetemp = CreateFile(this->FILETEMP_DIR, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if(this->h_filetemp == INVALID_HANDLE_VALUE) return FALSE;

	*this->p_filetemp_size_l32 = (ULONG32) GetFileSize(this->h_filetemp, (DWORD*) this->p_filetemp_size_h32);
	return TRUE;
}

VOID WINAPI AudioBaseClass::filetemp_close(VOID)
{
	if(this->h_filetemp == INVALID_HANDLE_VALUE) return;

	CloseHandle(this->h_filetemp);
	this->h_filetemp = INVALID_HANDLE_VALUE;
	this->filetemp_size_64 = 0u;
	return;
}

BOOL WINAPI AudioBaseClass::filein_get_params(VOID)
{
	const SIZE_T BUFFER_SIZE = 4096u;
	CHAR *header_info = nullptr;
	UINT16 *pu16 = nullptr;
	UINT32 *pu32 = nullptr;
	SIZE_T bytepos = 0u;
	DWORD dummy_32;

	header_info = (CHAR*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, BUFFER_SIZE);
	if(header_info == nullptr)
	{
		this->error_msg = TEXT("Heap Allocate Failed.");
		this->status = this->STATUS_ERROR_GENERIC;
		return FALSE;
	}

	SetFilePointer(this->h_filein, 0, NULL, FILE_BEGIN);
	ReadFile(this->h_filein, header_info, (DWORD) BUFFER_SIZE, &dummy_32, NULL);

	if(!this->compare_signature("RIFF", header_info, 0u))
	{
		HeapFree(p_processheap, 0, header_info);
		this->status = this->STATUS_ERROR_BROKENHEADER;
		return FALSE;
	}

	if(!this->compare_signature("WAVE", header_info, 8u))
	{
		HeapFree(p_processheap, 0, header_info);
		this->status = this->STATUS_ERROR_BROKENHEADER;
		return FALSE;
	}

	bytepos = 12u;

	while(!this->compare_signature("fmt ", header_info, bytepos))
	{
		if(bytepos >= (BUFFER_SIZE - 256u))
		{
			HeapFree(p_processheap, 0, header_info);
			this->status = this->STATUS_ERROR_BROKENHEADER;
			return FALSE;
		}

		pu32 = (UINT32*) &header_info[bytepos + 4u];
		bytepos += (SIZE_T) (*pu32 + 8u);
	}

	pu16 = (UINT16*) &header_info[bytepos + 8u];

	if(pu16[0] != 1u)
	{
		HeapFree(p_processheap, 0, header_info);
		this->status = this->STATUS_ERROR_FORMATNOTSUPPORTED;
		return FALSE;
	}

	this->n_channels = pu16[1];

	pu32 = (UINT32*) &header_info[bytepos + 12u];
	this->sample_rate = *pu32;

	pu16 = (UINT16*) &header_info[bytepos + 22u];
	this->bit_depth = *pu16;

	pu32 = (UINT32*) &header_info[bytepos + 4u];
	bytepos += (SIZE_T) (*pu32 + 8u);

	while(!this->compare_signature("data", header_info, bytepos))
	{
		if(bytepos >= (BUFFER_SIZE - 256u))
		{
			HeapFree(p_processheap, 0, header_info);
			this->status = this->STATUS_ERROR_BROKENHEADER;
			return FALSE;
		}

		pu32 = (UINT32*) &header_info[bytepos + 4u];
		bytepos += (SIZE_T) (*pu32 + 8u);
	}

	pu32 = (UINT32*) &header_info[bytepos + 4u];

	this->audio_data_begin = (ULONG64) (bytepos + 8u);
	this->audio_data_end = this->audio_data_begin + ((ULONG64) *pu32);

	HeapFree(p_processheap, 0, header_info);

	switch(this->bit_depth)
	{
		case 16u:
			this->format = this->FORMAT_I16;
			this->status = this->STATUS_INITIALIZED;
			return TRUE;

		case 24u:
			this->format = this->FORMAT_I24;
			this->status = this->STATUS_INITIALIZED;
			return TRUE;
	}

	this->format = this->FORMAT_UNSUPPORTED;
	this->status = this->STATUS_ERROR_FORMATNOTSUPPORTED;
	return FALSE;
}

BOOL WINAPI AudioBaseClass::fileout_write_header(VOID)
{
	CHAR *header_info = nullptr;
	UINT16 *pu16 = nullptr;
	UINT32 *pu32 = nullptr;
	DWORD dummy_32;

	header_info = (CHAR*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, 44);
	if(header_info == nullptr)
	{
		this->error_msg = TEXT("Heap Allocate Failed.");
		return FALSE;
	}

	header_info[0] = 'R';
	header_info[1] = 'I';
	header_info[2] = 'F';
	header_info[3] = 'F';

	pu32 = (UINT32*) &header_info[4];
	*pu32 = (UINT32) (this->filetemp_size_64 + 36u);

	header_info[8] = 'W';
	header_info[9] = 'A';
	header_info[10] = 'V';
	header_info[11] = 'E';

	header_info[12] = 'f';
	header_info[13] = 'm';
	header_info[14] = 't';
	header_info[15] = ' ';

	pu32 = (UINT32*) &header_info[16];
	*pu32 = 16u;

	pu16 = (UINT16*) &header_info[20];
	pu16[0] = 1u;
	pu16[1] = this->n_channels;

	pu32 = (UINT32*) &header_info[24];
	pu32[0] = this->sample_rate;
	pu32[1] = this->sample_rate*((UINT32) (this->n_channels*this->bit_depth/8u));

	pu16 = (UINT16*) &header_info[32];
	pu16[0] = this->n_channels*this->bit_depth/8u;
	pu16[1] = this->bit_depth;

	header_info[36] = 'd';
	header_info[37] = 'a';
	header_info[38] = 't';
	header_info[39] = 'a';

	pu32 = (UINT32*) &header_info[40];
	*pu32 = (UINT32) this->filetemp_size_64;

	SetFilePointer(this->h_fileout, 0, NULL, FILE_BEGIN);
	WriteFile(this->h_fileout, header_info, 44u, &dummy_32, NULL);
	this->fileout_pos_64 = 44u;

	HeapFree(p_processheap, 0, header_info);
	return TRUE;
}

BOOL WINAPI AudioBaseClass::file_ext_check(const TCHAR *file_dir)
{
	SIZE_T len = 0u;

	if(file_dir == nullptr) return FALSE;

	len = (SIZE_T) cstr_getlength(file_dir);

	if(len < 5u) return FALSE;

	if(cstr_compare(&file_dir[len - 4u], TEXT(".wav"))) return TRUE;
	if(cstr_compare(&file_dir[len - 4u], TEXT(".WAV"))) return TRUE;

	return FALSE;
}

BOOL WINAPI AudioBaseClass::compare_signature(const CHAR *auth, const CHAR *bytebuf, SIZE_T offset)
{
	if(auth == nullptr) return FALSE;
	if(bytebuf == nullptr) return FALSE;

	if(auth[0] != bytebuf[offset]) return FALSE;
	if(auth[1] != bytebuf[offset + 1u]) return FALSE;
	if(auth[2] != bytebuf[offset + 2u]) return FALSE;
	if(auth[3] != bytebuf[offset + 3u]) return FALSE;

	return TRUE;
}

BOOL WINAPI AudioBaseClass::rawtowav_proc(VOID)
{
	VOID *p_buffer = nullptr;

	if(this->status < 1) return FALSE;

	if(!this->filetemp_open())
	{
		this->error_msg = TEXT("Could not open temporary DSP file.");
		return FALSE;
	}

	if(!this->fileout_create())
	{
		this->filetemp_close();
		this->error_msg = TEXT("Could not create output file.");
		return FALSE;
	}

	if(!this->fileout_write_header())
	{
		this->filetemp_close();
		this->fileout_close();
		return FALSE;
	}

	this->filetemp_pos_64 = 0u;

	p_buffer = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BUFFER_SIZE_BYTES);
	if(p_buffer == nullptr)
	{
		this->error_msg = TEXT("Heap Allocate Failed.");
		return FALSE;
	}

	this->rawtowav_proc_loop(p_buffer);

	HeapFree(p_processheap, 0, p_buffer);

	this->filetemp_close();
	this->fileout_close();
	return TRUE;
}

VOID WINAPI AudioBaseClass::rawtowav_proc_loop(VOID *p_buffer)
{
	DWORD dummy_32;

	while(this->filetemp_pos_64 < this->filetemp_size_64)
	{
		ZeroMemory(p_buffer, this->BUFFER_SIZE_BYTES);

		SetFilePointer(this->h_filetemp, (LONG) *this->p_filetemp_pos_l32, (LONG*) this->p_filetemp_pos_h32, FILE_BEGIN);
		ReadFile(this->h_filetemp, p_buffer, (DWORD) this->BUFFER_SIZE_BYTES, &dummy_32, NULL);
		this->filetemp_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;

		SetFilePointer(this->h_fileout, (LONG) *this->p_fileout_pos_l32, (LONG*) this->p_fileout_pos_h32, FILE_BEGIN);
		WriteFile(this->h_fileout, p_buffer, (DWORD) this->BUFFER_SIZE_BYTES, &dummy_32, NULL);
		this->fileout_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;
	}

	return;
}
