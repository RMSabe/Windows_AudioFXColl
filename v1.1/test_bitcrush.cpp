/*
	Test code to test AudioBitCrush class

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "globaldef.h"

#include <iostream>
#include <string>

#include "AudioBaseClass.hpp"
#include "AudioBitCrush.hpp"

UINT cutoff = 0u;
CHAR *filein_dir = nullptr;
CHAR *fileout_dir = nullptr;

AudioBitCrush *pAudio = nullptr;

INT main(INT argc, CHAR **argv)
{
	if(argc < 4)
	{
		std::cout << "Error: missing arguments\r\nThis test routine requires 3 arguments:\r\n<input file directory> <output file directory> <bit cutoff>\r\nThey must be in that order\r\n";
		system("pause");
		return 1;
	}

	filein_dir = argv[1];
	fileout_dir = argv[2];

	try
	{
		cutoff = (UINT) std::stoi(argv[3]);
	}
	catch(...)
	{
		std::cout << "Error: value entered for bit cutoff is invalid\r\n";
		return 1;
	}

	pAudio = new AudioBitCrush(filein_dir, fileout_dir);

	if(!pAudio->initialize())
	{
		std::cout << "Error occurred: " << pAudio->getLastErrorMessage() << std::endl;
		delete pAudio;
		return 1;
	}

	if(!pAudio->setCutoff(cutoff))
	{
		std::cout << "Error setting cutoff\r\n";
		std::cout << pAudio->getLastErrorMessage() << std::endl;
		delete pAudio;
		return 1;
	}

	std::cout << "DSP started...\r\n";

	if(!pAudio->runDSP())
	{
		std::cout << "DSP failed\r\n";
		std::cout << "Error: " << pAudio->getLastErrorMessage() << std::endl;
		delete pAudio;
		return 1;
	}

	std::cout << "DSP finished\r\n";
	delete pAudio;
	return 0;
}
