/*
	Test code to test AudioChannelSubtract class

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "globaldef.h"

#include <iostream>
#include <string>

#include "AudioBaseClass.hpp"
#include "AudioChannelSubtract.hpp"

CHAR *filein_dir = nullptr;
CHAR *fileout_dir = nullptr;

AudioChannelSubtract *pAudio = nullptr;

INT main(INT argc, CHAR **argv)
{
	if(argc < 3)
	{
		std::cout << "Error: missing arguments\r\nThis test routine requires 2 arguments:\r\n<input file directory> <output file directory>\r\nThey must be in that order\r\n";
		system("pause");
		return 1;
	}

	filein_dir = argv[1];
	fileout_dir = argv[2];

	pAudio = new AudioChannelSubtract(filein_dir, fileout_dir);

	if(!pAudio->initialize())
	{
		std::cout << "Error occurred: " << pAudio->getLastErrorMessage() << std::endl;
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
