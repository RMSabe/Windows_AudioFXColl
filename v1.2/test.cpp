#include "globldef.h"
#include "strdef.h"
#include "console.h"

#include "strdef.hpp"

#include "AudioBaseClass.hpp"
#include "AudioBitCrush.hpp"
#include "AudioReverse.hpp"
#include "AudioChannelSwap.hpp"
#include "AudioChannelSubtract.hpp"

AudioBaseClass *p_audio = nullptr;

__string tstr = TEXT("");

extern BOOL WINAPI app_init(VOID);
extern VOID WINAPI app_deinit(VOID);

extern VOID WINAPI print_cmd_list(VOID);

extern VOID WINAPI bitcrush_proc(VOID);
extern VOID WINAPI reverse_proc(VOID);
extern VOID WINAPI channelswap_proc(VOID);
extern VOID WINAPI channelsubtract_proc(VOID);

INT main(INT argc, TCHAR **argv)
{
	if(!app_init())
	{
		app_deinit();
		return 1;
	}

	if(argc < 2)
	{
		console_stdout_write(TEXT("Error: missing arguments\r\n\r\n"));
		print_cmd_list();
		console_wait_keypress(NULL);
		app_deinit();
		return 1;
	}

	tstr = argv[1];
	tstr = cppstr_tolower(tstr);

	if(!tstr.compare(TEXT("bitcrush"))) bitcrush_proc();
	else if(!tstr.compare(TEXT("reverse"))) reverse_proc();
	else if(!tstr.compare(TEXT("channelswap"))) channelswap_proc();
	else if(!tstr.compare(TEXT("channelsubtract"))) channelsubtract_proc();
	else
	{
		console_stdout_write(TEXT("Invalid argument\r\n\r\n"));
		print_cmd_list();
		console_wait_keypress(NULL);
	}

	app_deinit();
	return 0;
}

BOOL WINAPI app_init(VOID)
{
	p_processheap = GetProcessHeap();
	if(p_processheap == nullptr)
	{
		MessageBox(NULL, TEXT("Invalid Process Heap."), TEXT("INIT ERROR"), (MB_ICONSTOP | MB_OK));
		return FALSE;
	}

	if(!console_init())
	{
		MessageBox(NULL, TEXT("Console Init Failed."), TEXT("INIT ERROR"), (MB_ICONSTOP | MB_OK));
		return FALSE;
	}

	return TRUE;
}

VOID WINAPI app_deinit(VOID)
{
	console_deinit();
	return;
}

VOID WINAPI print_cmd_list(VOID)
{
	__string str = TEXT("Valid Arguments:\r\n\r\n");

	str += TEXT("\"bitcrush\": Audio Bit Crush\r\n");
	str += TEXT("\"reverse\": Audio Reverse\r\n");
	str += TEXT("\"channelswap\": Audio Channel Swap\r\n");
	str += TEXT("\"channelsubtract\": Audio Channel Subtract\r\n\r\n");

	console_stdout_write(str.c_str());
	return;
}

VOID WINAPI bitcrush_proc(VOID)
{
	INT bitcrush = 0;

	p_audio = new AudioBitCrush();

	console_stdout_write(TEXT("Enter Input File Directory: "));
	console_stdin_readcmd(textbuf, TEXTBUF_SIZE_CHARS);

	p_audio->filein_dir = textbuf;

	console_stdout_write(TEXT("Enter Output File Directory: "));
	console_stdin_readcmd(textbuf, TEXTBUF_SIZE_CHARS);

	p_audio->fileout_dir = textbuf;

	if(!p_audio->initialize())
	{
		console_stdout_write(TEXT("AudioClass Init Failed\r\nError Msg: "));
		console_stdout_write(p_audio->getLastErrorMessage().c_str());
		console_wait_keypress(TEXT("\r\n"));

		delete ((AudioBitCrush*) p_audio);
		p_audio = nullptr;
		return;
	}

	console_stdout_write(TEXT("Enter bit crush value: "));
	console_stdin_readcmd(textbuf, TEXTBUF_SIZE_CHARS);

	tstr = textbuf;

	try
	{
		bitcrush = std::stoi(tstr);
	}
	catch(...)
	{
		console_wait_keypress(TEXT("Invalid value entered\r\n"));
		delete ((AudioBitCrush*) p_audio);
		p_audio = nullptr;
		return;
	}

	if(bitcrush < 0 || bitcrush > 255)
	{
		console_wait_keypress(TEXT("Invalid value entered\r\n"));
		delete ((AudioBitCrush*) p_audio);
		p_audio = nullptr;
		return;
	}

	if(!((AudioBitCrush*) p_audio)->setCutoff((BYTE) bitcrush))
	{
		console_stdout_write(TEXT("Error: "));
		console_stdout_write(p_audio->getLastErrorMessage().c_str());
		console_wait_keypress(TEXT("\r\n"));
		delete ((AudioBitCrush*) p_audio);
		p_audio = nullptr;
		return;
	}

	console_stdout_write(TEXT("Running DSP...\r\n"));

	if(p_audio->runDSP()) console_stdout_write(TEXT("DSP Finished\r\n"));
	else
	{
		console_stdout_write(TEXT("DSP Failed\r\nError: "));
		console_stdout_write(p_audio->getLastErrorMessage().c_str());
	}

	console_wait_keypress(TEXT("\r\n"));
	delete ((AudioBitCrush*) p_audio);
	p_audio = nullptr;
	return;
}

VOID WINAPI reverse_proc(VOID)
{
	p_audio = new AudioReverse();

	console_stdout_write(TEXT("Enter Input File Directory: "));
	console_stdin_readcmd(textbuf, TEXTBUF_SIZE_CHARS);

	p_audio->filein_dir = textbuf;

	console_stdout_write(TEXT("Enter Output File Directory: "));
	console_stdin_readcmd(textbuf, TEXTBUF_SIZE_CHARS);

	p_audio->fileout_dir = textbuf;

	if(!p_audio->initialize())
	{
		console_stdout_write(TEXT("AudioClass Init Failed\r\nError Msg: "));
		console_stdout_write(p_audio->getLastErrorMessage().c_str());
		console_wait_keypress(TEXT("\r\n"));

		delete ((AudioReverse*) p_audio);
		p_audio = nullptr;
		return;
	}

	console_stdout_write(TEXT("Running DSP...\r\n"));

	if(p_audio->runDSP()) console_stdout_write(TEXT("DSP Finished\r\n"));
	else
	{
		console_stdout_write(TEXT("DSP Failed\r\nError: "));
		console_stdout_write(p_audio->getLastErrorMessage().c_str());
	}

	console_wait_keypress(TEXT("\r\n"));
	delete ((AudioReverse*) p_audio);
	p_audio = nullptr;
	return;
}

VOID WINAPI channelswap_proc(VOID)
{
	p_audio = new AudioChannelSwap();

	console_stdout_write(TEXT("Enter Input File Directory: "));
	console_stdin_readcmd(textbuf, TEXTBUF_SIZE_CHARS);

	p_audio->filein_dir = textbuf;

	console_stdout_write(TEXT("Enter Output File Directory: "));
	console_stdin_readcmd(textbuf, TEXTBUF_SIZE_CHARS);

	p_audio->fileout_dir = textbuf;

	if(!p_audio->initialize())
	{
		console_stdout_write(TEXT("AudioClass Init Failed\r\nError Msg: "));
		console_stdout_write(p_audio->getLastErrorMessage().c_str());
		console_wait_keypress(TEXT("\r\n"));

		delete ((AudioChannelSwap*) p_audio);
		p_audio = nullptr;
		return;
	}

	console_stdout_write(TEXT("Running DSP...\r\n"));

	if(p_audio->runDSP()) console_stdout_write(TEXT("DSP Finished\r\n"));
	else
	{
		console_stdout_write(TEXT("DSP Failed\r\nError: "));
		console_stdout_write(p_audio->getLastErrorMessage().c_str());
	}

	console_wait_keypress(TEXT("\r\n"));
	delete ((AudioChannelSwap*) p_audio);
	p_audio = nullptr;
	return;
}

VOID WINAPI channelsubtract_proc(VOID)
{
	p_audio = new AudioChannelSubtract();

	console_stdout_write(TEXT("Enter Input File Directory: "));
	console_stdin_readcmd(textbuf, TEXTBUF_SIZE_CHARS);

	p_audio->filein_dir = textbuf;

	console_stdout_write(TEXT("Enter Output File Directory: "));
	console_stdin_readcmd(textbuf, TEXTBUF_SIZE_CHARS);

	p_audio->fileout_dir = textbuf;

	if(!p_audio->initialize())
	{
		console_stdout_write(TEXT("AudioClass Init Failed\r\nError Msg: "));
		console_stdout_write(p_audio->getLastErrorMessage().c_str());
		console_wait_keypress(TEXT("\r\n"));

		delete ((AudioChannelSubtract*) p_audio);
		p_audio = nullptr;
		return;
	}

	console_stdout_write(TEXT("Running DSP...\r\n"));

	if(p_audio->runDSP()) console_stdout_write(TEXT("DSP Finished\r\n"));
	else
	{
		console_stdout_write(TEXT("DSP Failed\r\nError: "));
		console_stdout_write(p_audio->getLastErrorMessage().c_str());
	}

	console_wait_keypress(TEXT("\r\n"));
	delete ((AudioChannelSubtract*) p_audio);
	p_audio = nullptr;
	return;
}
