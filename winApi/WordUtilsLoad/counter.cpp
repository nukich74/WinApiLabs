#include "stdafx.h"
#include <sstream>
#include <iomanip>
#include <string>

extern "C" __declspec(dllexport) int WordCounterHex( const wchar_t* textPtr ) 
{
	int count = 0;
	std::wstring tempString;
	std::wstringstream wss( textPtr );
	while( wss >> tempString ) {
		count++;
	}
	return count;
}
