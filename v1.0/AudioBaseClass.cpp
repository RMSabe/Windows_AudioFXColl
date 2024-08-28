/*
	Audio FX Collection version 1.0 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioBaseClass.hpp"

AudioBaseClass::AudioBaseClass(VOID)
{
	this->status = this->STATUS_UNINITIALIZED;
	this->fileout_dir = this->FILEOUT_DIR_DEFAULT;
}

AudioBaseClass::AudioBaseClass(std::string filein_dir) : AudioBaseClass()
{
	this->filein_dir = filein_dir;
}

AudioBaseClass::AudioBaseClass(std::string filein_dir, std::string fileout_dir) : AudioBaseClass(filein_dir)
{
	this->fileout_dir = fileout_dir;
}

VOID AudioBaseClass::setInputFileDirectory(std::string file_dir)
{
	this->filein_dir = file_dir;
	return;
}

std::string AudioBaseClass::getInputFileDirectory(VOID)
{
	return this->filein_dir;
}

VOID AudioBaseClass::setOutputFileDirectory(std::string file_dir)
{
	this->fileout_dir = file_dir;
	return;
}

std::string AudioBaseClass::getOutputFileDirectory(VOID)
{
	return this->fileout_dir;
}

BOOL AudioBaseClass::initialize(VOID)
{
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

std::string AudioBaseClass::getLastErrorMessage(VOID)
{
	switch(this->status)
	{
		case this->STATUS_ERROR_BROKENHEADER:
			return "File header is missing information (probably corrupted).";

		case this->STATUS_ERROR_FORMATNOTSUPPORTED:
			return "Audio format is not supported.";

		case this->STATUS_ERROR_FILENOTSUPPORTED:
			return "File format is not supported.";

		case this->STATUS_ERROR_NOFILE:
			return "File does not exist, or it's not accessible.";

		case this->STATUS_ERROR_GENERIC:
			return "Something went wrong.";

		case this->STATUS_UNINITIALIZED:
			return "Audio object not initialized.";
	}

	return this->error_msg;
}

UINT AudioBaseClass::getSampleRate(VOID)
{
	return this->sample_rate;
}

USHORT AudioBaseClass::getBitDepth(VOID)
{
	return this->bit_depth;
}

USHORT AudioBaseClass::getNumberChannels(VOID)
{
	return this->n_channels;
}

BOOL AudioBaseClass::filein_open(VOID)
{
	this->filein = CreateFile(this->filein_dir.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if(this->filein == INVALID_HANDLE_VALUE) return FALSE;

	*this->filein_psize_l32 = GetFileSize(this->filein, this->filein_psize_h32);
	return TRUE;
}

VOID AudioBaseClass::filein_close(VOID)
{
	if(this->filein == INVALID_HANDLE_VALUE) return;

	CloseHandle(this->filein);
	this->filein = INVALID_HANDLE_VALUE;
	this->filein_size_64 = 0u;
	return;
}

BOOL AudioBaseClass::fileout_create(VOID)
{
	this->fileout = CreateFile(this->fileout_dir.c_str(), (GENERIC_READ | GENERIC_WRITE), FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, 0);
	return (this->fileout != INVALID_HANDLE_VALUE);
}

VOID AudioBaseClass::fileout_close(VOID)
{
	if(this->fileout == INVALID_HANDLE_VALUE) return;

	CloseHandle(this->fileout);
	this->fileout = INVALID_HANDLE_VALUE;
	return;
}

BOOL AudioBaseClass::filetemp_create(VOID)
{
	this->filetemp = CreateFile(this->FILETEMP_DIR, (GENERIC_READ | GENERIC_WRITE), FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, 0);
	return (this->filetemp != INVALID_HANDLE_VALUE);
}

BOOL AudioBaseClass::filetemp_open(VOID)
{
	this->filetemp = CreateFile(this->FILETEMP_DIR, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if(this->filetemp == INVALID_HANDLE_VALUE) return FALSE;

	*this->filetemp_psize_l32 = GetFileSize(this->filetemp, this->filetemp_psize_h32);
	return TRUE;
}

VOID AudioBaseClass::filetemp_close(VOID)
{
	if(this->filetemp == INVALID_HANDLE_VALUE) return;

	CloseHandle(this->filetemp);
	this->filetemp = INVALID_HANDLE_VALUE;
	this->filetemp_size_64 = 0u;
	return;
}

BOOL AudioBaseClass::filein_get_params(VOID)
{
	const SIZE_T BUFFER_SIZE = 4096u;
	CHAR *header_info = (CHAR*) std::malloc(BUFFER_SIZE);
	USHORT *pu16 = nullptr;
	UINT *pu32 = nullptr;

	SIZE_T bytepos = 0u;

	SetFilePointer(this->filein, 0, NULL, FILE_BEGIN);
	ReadFile(this->filein, header_info, BUFFER_SIZE, NULL, NULL);

	if(!this->compare_signature("RIFF", header_info, 0u))
	{
		std::free(header_info);
		this->status = this->STATUS_ERROR_BROKENHEADER;
		return FALSE;
	}

	if(!this->compare_signature("WAVE", header_info, 8u))
	{
		std::free(header_info);
		this->status = this->STATUS_ERROR_BROKENHEADER;
		return FALSE;
	}

	bytepos = 12u;

	while(!this->compare_signature("fmt ", header_info, bytepos))
	{
		if(bytepos >= (BUFFER_SIZE - 256u))
		{
			std::free(header_info);
			this->status = this->STATUS_ERROR_BROKENHEADER;
			return FALSE;
		}

		pu32 = (UINT*) &header_info[bytepos + 4u];
		bytepos += (SIZE_T) (*pu32 + 8u);
	}

	pu16 = (USHORT*) &header_info[bytepos + 8u];

	if(pu16[0] != 1u)
	{
		std::free(header_info);
		this->status = this->STATUS_ERROR_FORMATNOTSUPPORTED;
		return FALSE;
	}

	this->n_channels = pu16[1];

	pu32 = (UINT*) &header_info[bytepos + 12u];
	this->sample_rate = *pu32;

	pu16 = (USHORT*) &header_info[bytepos + 22u];
	this->bit_depth = *pu16;

	pu32 = (UINT*) &header_info[bytepos + 4u];
	bytepos += (SIZE_T) (*pu32 + 8u);

	while(!this->compare_signature("data", header_info, bytepos))
	{
		if(bytepos >= (BUFFER_SIZE - 256u))
		{
			std::free(header_info);
			this->status = this->STATUS_ERROR_BROKENHEADER;
			return FALSE;
		}

		pu32 = (UINT*) &header_info[bytepos + 4u];
		bytepos += (SIZE_T) (*pu32 + 8u);
	}

	pu32 = (UINT*) &header_info[bytepos + 4u];

	this->audio_data_begin = (ULONG64) (bytepos + 8u);
	this->audio_data_end = this->audio_data_begin + ((ULONG64) *pu32);

	std::free(header_info);

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

VOID AudioBaseClass::fileout_write_header(VOID)
{
	CHAR *header_info = (CHAR*) std::malloc(44);
	USHORT *pu16 = nullptr;
	UINT *pu32 = nullptr;

	header_info[0] = 'R';
	header_info[1] = 'I';
	header_info[2] = 'F';
	header_info[3] = 'F';

	pu32 = (UINT*) &header_info[4];
	*pu32 = (UINT) (this->filetemp_size_64 + 36u);

	header_info[8] = 'W';
	header_info[9] = 'A';
	header_info[10] = 'V';
	header_info[11] = 'E';

	header_info[12] = 'f';
	header_info[13] = 'm';
	header_info[14] = 't';
	header_info[15] = ' ';

	pu32 = (UINT*) &header_info[16];
	*pu32 = 16u;

	pu16 = (USHORT*) &header_info[20];
	pu16[0] = 1u;
	pu16[1] = this->n_channels;

	pu32 = (UINT*) &header_info[24];
	pu32[0] = this->sample_rate;
	pu32[1] = this->sample_rate*((UINT) (this->n_channels*this->bit_depth/8u));

	pu16 = (USHORT*) &header_info[32];
	pu16[0] = this->n_channels*this->bit_depth/8u;
	pu16[1] = this->bit_depth;

	header_info[36] = 'd';
	header_info[37] = 'a';
	header_info[38] = 't';
	header_info[39] = 'a';

	pu32 = (UINT*) &header_info[40];
	*pu32 = (UINT) this->filetemp_size_64;

	SetFilePointer(this->fileout, 0, NULL, FILE_BEGIN);
	WriteFile(this->fileout, header_info, 44, NULL, NULL);
	this->fileout_pos_64 = 44u;

	std::free(header_info);
	return;
}

BOOL AudioBaseClass::file_ext_check(const CHAR *file_dir)
{
	if(file_dir == nullptr) return FALSE;

	SIZE_T len = 0u;
	while(file_dir[len] != '\0') len++;

	if(len < 5u) return FALSE;

	if(this->compare_signature(".wav", file_dir, (len - 4u))) return TRUE;
	if(this->compare_signature(".WAV", file_dir, (len - 4u))) return TRUE;

	return FALSE;
}

BOOL AudioBaseClass::compare_signature(const CHAR *auth, const CHAR *bytebuf, SIZE_T offset)
{
	if(auth == nullptr) return FALSE;
	if(bytebuf == nullptr) return FALSE;

	if(auth[0] != bytebuf[offset]) return FALSE;
	if(auth[1] != bytebuf[offset + 1u]) return FALSE;
	if(auth[2] != bytebuf[offset + 2u]) return FALSE;
	if(auth[3] != bytebuf[offset + 3u]) return FALSE;

	return TRUE;
}

BOOL AudioBaseClass::rawtowav_proc(VOID)
{
	if(this->status < 1) return FALSE;

	if(!this->filetemp_open())
	{
		this->error_msg = "Could not open temporary DSP file.";
		return FALSE;
	}

	if(!this->fileout_create())
	{
		this->filetemp_close();
		this->error_msg = "Could not create output file.";
		return FALSE;
	}

	this->fileout_write_header();
	this->filetemp_pos_64 = 0u;

	VOID *buffer = std::malloc(this->BUFFER_SIZE_BYTES);

	this->rawtowav_proc_loop(buffer);

	std::free(buffer);

	this->filetemp_close();
	this->fileout_close();
	return TRUE;
}

VOID AudioBaseClass::rawtowav_proc_loop(VOID *buffer)
{
	while(TRUE)
	{
		if(this->filetemp_pos_64 >= this->filetemp_size_64) break;

		ZeroMemory(buffer, this->BUFFER_SIZE_BYTES);

		SetFilePointer(this->filetemp, (LONG) *this->filetemp_ppos_l32, (LONG*) this->filetemp_ppos_h32, FILE_BEGIN);
		ReadFile(this->filetemp, buffer, this->BUFFER_SIZE_BYTES, NULL, NULL);
		this->filetemp_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;

		SetFilePointer(this->fileout, (LONG) *this->fileout_ppos_l32, (LONG*) this->fileout_ppos_h32, FILE_BEGIN);
		WriteFile(this->fileout, buffer, this->BUFFER_SIZE_BYTES, NULL, NULL);
		this->fileout_pos_64 += (ULONG64) this->BUFFER_SIZE_BYTES;
	}

	return;
}
