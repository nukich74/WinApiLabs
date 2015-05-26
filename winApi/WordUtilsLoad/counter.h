#ifdef MYDLL_EXPORTS
#define MYDLL_API __declspec(dllexport) 
#else
#define MYDLL_API __declspec(dllimport) 
#endif

//MYDLL_API int WordCounterHex( const wchar_t* textPtr );
