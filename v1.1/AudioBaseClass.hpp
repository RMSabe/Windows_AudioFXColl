/*
	Audio FX Collection version 1.1 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOBASECLASS_HPP
#define AUDIOBASECLASS_HPP

#include "globaldef.h"
#include "stringdef.hpp"

class AudioBaseClass {
	public:
		AudioBaseClass(VOID);
		AudioBaseClass(__string filein_dir);
		AudioBaseClass(__string filein_dir, __string fileout_dir);

		VOID setInputFileDirectory(__string file_dir);
		__string getInputFileDirectory(VOID);

		VOID setOutputFileDirectory(__string file_dir);
		__string getOutputFileDirectory(VOID);

		BOOL initialize(VOID);

		virtual BOOL runDSP(VOID) = 0;

		__string getLastErrorMessage(VOID);

		UINT getSampleRate(VOID);
		USHORT getBitDepth(VOID);
		USHORT getNumberChannels(VOID);

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

		UINT sample_rate = 0u;
		USHORT bit_depth = 0u;
		USHORT n_channels = 0u;

		ULONG64 audio_data_begin = 0u;
		ULONG64 audio_data_end = 0u;

		__string filein_dir = TEXT("");
		__string fileout_dir = TEXT("");

		HANDLE filein = INVALID_HANDLE_VALUE;
		HANDLE fileout = INVALID_HANDLE_VALUE;
		HANDLE filetemp = INVALID_HANDLE_VALUE;

		ULONG64 filein_size_64 = 0u;
		ULONG* const filein_psize_l32 = (ULONG*) &this->filein_size_64;
		ULONG* const filein_psize_h32 = &this->filein_psize_l32[1];

		ULONG64 filetemp_size_64 = 0u;
		ULONG* const filetemp_psize_l32 = (ULONG*) &this->filetemp_size_64;
		ULONG* const filetemp_psize_h32 = &this->filetemp_psize_l32[1];

		ULONG64 filein_pos_64 = 0u;
		ULONG* const filein_ppos_l32 = (ULONG*) &this->filein_pos_64;
		ULONG* const filein_ppos_h32 = &this->filein_ppos_l32[1];

		ULONG64 fileout_pos_64 = 0u;
		ULONG* const fileout_ppos_l32 = (ULONG*) &this->fileout_pos_64;
		ULONG* const fileout_ppos_h32 = &this->fileout_ppos_l32[1];

		ULONG64 filetemp_pos_64 = 0u;
		ULONG* const filetemp_ppos_l32 = (ULONG*) &this->filetemp_pos_64;
		ULONG* const filetemp_ppos_h32 = &this->filetemp_ppos_l32[1];

		BOOL filein_open(VOID);
		VOID filein_close(VOID);

		BOOL fileout_create(VOID);
		VOID fileout_close(VOID);

		BOOL filetemp_create(VOID);
		BOOL filetemp_open(VOID);
		VOID filetemp_close(VOID);

		BOOL filein_get_params(VOID);

		VOID fileout_write_header(VOID);

		BOOL file_ext_check(const TCHAR *file_dir);

		BOOL compare_signature(const CHAR *auth, const CHAR *bytebuf, SIZE_T offset);
		BOOL compare_signature(const WCHAR *auth, const WCHAR *wordbuf, SIZE_T offset);

		BOOL rawtowav_proc(VOID);
		VOID rawtowav_proc_loop(VOID *buffer);
};

/*
Notes:
Since NTFS and exFAT supports files bigger than 4GB, it's a good idea to represent file size and position using 64bit integers. (Even though a .wav file will probably never be this big).
However, Windows NT file API "fileapi.h" handles file size and position using two separate 32bit integers. (low 32, high 32).
So, I've created pointers to split the 64bit integers into 2 separate 32bit integers.
Example:

filein_size_64: 64bit unsigned integer to store the input file size.
filein_psize_l32: a pointer to the lower 32 bits of the filein_size_64 variable. Used by the fileapi.h functions.
filein_psize_h32: a pointer to the higher 32 bits of the filein_size_64 variable. Used by the fileapi.h functions.
*/

#endif //AUDIOBASECLASS_HPP
