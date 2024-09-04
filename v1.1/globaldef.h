/*
	Audio FX Collection version 1.1 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef GLOBALDEF_H
#define GLOBALDEF_H

//Text formatting macro. Define UNICODE for 16bit unicode text format. Undefine it for ANSI text format.
//Using 16bit unicode is recommended for Windows GUI applications.
//Using ANSI is recommended for Console applications.

#ifdef UNICODE
#undef UNICODE
#endif

#include <windows.h>

#endif //GLOBALDEF_H
