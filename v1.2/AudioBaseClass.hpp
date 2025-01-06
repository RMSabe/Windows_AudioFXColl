/*
	Audio FX Collection version 1.2 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOBASECLASS_HPP
#define AUDIOBASECLASS_HPP

#include "globldef.h"
#include "strdef.h"
#include "strdef.hpp"

class AudioBaseClass {
	public:
		AudioBaseClass(VOID);
		AudioBaseClass(__string filein_dir);
		AudioBaseClass(__string filein_dir, __string fileout_dir);

		BOOL WINAPI initialize(VOID);

		virtual BOOL WINAPI runDSP(VOID) = 0;

		__string WINAPI getLastErrorMessage(VOID);

		UINT32 WINAPI getSampleRate(VOID);
		UINT16 WINAPI getBitDepth(VOID);
		UINT16 WINAPI getNumberChannels(VOID);

		__string filein_dir = TEXT("");
		__string fileout_dir = TEXT("");

	protected:
		const TCHAR *FILETEMP_DIR = TEXT("temp.raw");
		const TCHAR *FILEOUT_DIR_DEFAULT = TEXT("output.wav");

		//All buffer size values will be set during initialization and should be treated as constants.
		SIZE_T BUFFER_SIZE_FRAMES = 512u;
		SIZE_T BUFFER_SIZE_SAMPLES = 0u;
		SIZE_T BUFFER_SIZE_BYTES = 0u;

		enum FORMATS {
			FORMAT_UNSUPPORTED = -1,
			FORMAT_NULL = 0,
			FORMAT_I16 = 1,
			FORMAT_I24 = 2
		};

		enum STATUS {
			STATUS_ERROR_BROKENHEADER = -5,
			STATUS_ERROR_FORMATNOTSUPPORTED = -4,
			STATUS_ERROR_FILENOTSUPPORTED = -3,
			STATUS_ERROR_NOFILE = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_INITIALIZED = 1
		};

		INT format = this->FORMAT_NULL;
		INT status = this->STATUS_UNINITIALIZED;

		__string error_msg = TEXT("");

		UINT32 sample_rate = 0u;
		UINT16 bit_depth = 0u;
		UINT16 n_channels = 0u;

		ULONG64 audio_data_begin = 0u;
		ULONG64 audio_data_end = 0u;

		HANDLE h_filein = INVALID_HANDLE_VALUE;
		HANDLE h_fileout = INVALID_HANDLE_VALUE;
		HANDLE h_filetemp = INVALID_HANDLE_VALUE;

		ULONG64 filein_size_64 = 0u;
		ULONG32* const p_filein_size_l32 = (ULONG32*) &this->filein_size_64;
		ULONG32* const p_filein_size_h32 = &this->p_filein_size_l32[1];

		ULONG64 filetemp_size_64 = 0u;
		ULONG32* const p_filetemp_size_l32 = (ULONG32*) &this->filetemp_size_64;
		ULONG32* const p_filetemp_size_h32 = &this->p_filetemp_size_l32[1];

		ULONG64 filein_pos_64 = 0u;
		ULONG32* const p_filein_pos_l32 = (ULONG32*) &this->filein_pos_64;
		ULONG32* const p_filein_pos_h32 = &this->p_filein_pos_l32[1];

		ULONG64 fileout_pos_64 = 0u;
		ULONG32* const p_fileout_pos_l32 = (ULONG32*) &this->fileout_pos_64;
		ULONG32* const p_fileout_pos_h32 = &this->p_fileout_pos_l32[1];

		ULONG64 filetemp_pos_64 = 0u;
		ULONG32* const p_filetemp_pos_l32 = (ULONG32*) &this->filetemp_pos_64;
		ULONG32* const p_filetemp_pos_h32 = &this->p_filetemp_pos_l32[1];

		BOOL WINAPI filein_open(VOID);
		VOID WINAPI filein_close(VOID);

		BOOL WINAPI fileout_create(VOID);
		VOID WINAPI fileout_close(VOID);

		BOOL WINAPI filetemp_create(VOID);
		BOOL WINAPI filetemp_open(VOID);
		VOID WINAPI filetemp_close(VOID);

		BOOL WINAPI filein_get_params(VOID);

		BOOL WINAPI fileout_write_header(VOID);

		BOOL WINAPI file_ext_check(const TCHAR *file_dir);

		BOOL WINAPI compare_signature(const CHAR *auth, const CHAR *bytebuf, SIZE_T offset);

		BOOL WINAPI rawtowav_proc(VOID);
		VOID WINAPI rawtowav_proc_loop(VOID *p_buffer);

		virtual BOOL WINAPI buffer_alloc(SIZE_T size) = 0;
		virtual VOID WINAPI buffer_free(VOID) = 0;
};

/*
Notes:

Multiple global variables are pointers/handles to dynamically allocated contexts. The prefix "p_" and "h_" indicates the nature of the variable.

Prefix "p_" means the variable is like a normal pointer in which (0) (NULL) is an invalid value. (Example: p_filein_size_l32)
Prefix "h_" means the variable is a context handle in which (-1) (INVALID_HANDLE_VALUE) is an invalid value. (Example: h_filein)

Since NTFS and exFAT supports files bigger than 4GB, it's a good idea to represent file size and position using 64bit integers. (Even though a .wav file will probably never be this big).
However, Windows NT file API "fileapi.h" handles file size and position using two separate 32bit integers. (low 32, high 32).
So, I've created pointers to split the 64bit integers into 2 separate 32bit integers.
Example:

filein_size_64: 64bit unsigned integer to store the input file size.
p_filein_size_l32: a pointer to the lower 32 bits of the filein_size_64 variable. Used by the fileapi.h functions.
p_filein_size_h32: a pointer to the higher 32 bits of the filein_size_64 variable. Used by the fileapi.h functions.
*/

#endif //AUDIOBASECLASS_HPP
