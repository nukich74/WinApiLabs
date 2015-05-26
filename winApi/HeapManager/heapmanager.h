#pragma once
#include <set>
#include <Windows.h>
#include <map>

enum MemoryStatus
{
	MS_COMMIT,
	MS_RELEASE
};

class CMemoryKeyComparator {
public:
	bool operator()( const std::pair< int, MemoryStatus >&, const std::pair< int, MemoryStatus >& );
};

class CHeapManager {
public:
	static CHeapManager& CreateInstance( const int minSize, const int maxSize )
	{
		static bool isInit = false;
		static CHeapManager manager;
		if( isInit == false ) {	
			manager.buildSystemInfo();
			manager.Init( minSize, maxSize, manager.sysInfo.dwPageSize );
			isInit = true;
		}	
		return manager;
	}

	void Init( int minSize, int maxSize, int pageSize );
	void* Alloc( int size );
	void Free( void* ptr );
	void Init( int minSize, int maxSize );
private:
	int pagesCount;
	struct ::_SYSTEM_INFO sysInfo;
	std::map< void*, std::pair< int, MemoryStatus > > freePointersSet;
	std::multimap< std::pair< int, MemoryStatus >, void*, CMemoryKeyComparator > freeBlocksTree; //ptr, size, MS_***
	std::map< void*, int > usedBlocksTree;
	void* basePtr;
	void* reservePtr;

	CHeapManager() {}
	CHeapManager& operator=( const CHeapManager& value ) {}
	CHeapManager( const CHeapManager& value ) {}
	~CHeapManager();
	void buildSystemInfo();
	void deleteBlock( const void* value, std::pair< int, MemoryStatus > vertex );
};

typedef std::multimap< std::pair< int, MemoryStatus >, void*, CMemoryKeyComparator* >::iterator item;

