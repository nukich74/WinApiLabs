#include "stdafx.h"
#include <sstream>

namespace WordUtils
{
		int WordUtils::WordCounter( const wchar_t* textPtr ) 
		{
			int count = 0;
			std::wstring tempString;
			std::wstringstream wss( textPtr );
			while( wss >> tempString ) {
				count++;
			}
			return count;
		}
}