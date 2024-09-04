/*
	Audio FX Collection version 1.1 for Windows

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef STRINGDEF_HPP
#define STRINGDEF_HPP

#include "globaldef.h"
#include <string>

#ifdef UNICODE
typedef std::wstring __string;
#define __TOSTRING(value) std::to_wstring(value)
#else
typedef std::string __string;
#define __TOSTRING(value) std::to_string(value)
#endif

#endif //STRINGDEF_HPP
