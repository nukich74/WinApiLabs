#ifdef MYDLL_EXPORTS
#define MYDLL_API __declspec(dllexport) 
#else
#define MYDLL_API __declspec(dllimport) 
#endif

namespace WordUtils
{
	MYDLL_API int WordCounter( const wchar_t* textPtr );
}