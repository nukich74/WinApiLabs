#pragma once
#include <algorithm>
#include <iostream>
#include <heapmanager.h>
#include <exception>
#include <list>

void CHeapManager::deleteBlock( const void* value, std::pair< int, MemoryStatus > vertex )
{
	std::pair< item, item > p = freeBlocksTree.equal_range( vertex );
	for( auto it = p.first; it != p.second; it++ ) {
		if( value == it->second ) {
			freeBlocksTree.erase( it );
			break;
		}
	}
}

void CHeapManager::buildSystemInfo() {
	::GetSystemInfo( &sysInfo );
}

bool CMemoryKeyComparator::operator()( const std::pair< int, MemoryStatus >& value1, const std::pair< int, MemoryStatus >& value2 ) 
{
	return ( value1.first < value2.first ) || ( ( value1.first == value2.first ) && ( value1.second < value2.second ) );
}

void CHeapManager::Init( int minSize, int maxSize, int pageSize )
{
	minSize += ( pageSize - ( minSize % pageSize ) );
	maxSize += ( pageSize - ( maxSize % pageSize ) );
	this->pagesCount = minSize / pageSize;

	void* reservePtr = ::VirtualAlloc( NULL, maxSize, MEM_RESERVE, PAGE_READWRITE );
	if( !reservePtr ) {
		throw std::exception( "Error while reserve memory" );
	}
	void* basePtr = ::VirtualAlloc( reservePtr, minSize, MEM_COMMIT, PAGE_READWRITE );
	if( !basePtr ) {
		throw std::exception( "Error while commit memory" );
	} else {
		this->basePtr = basePtr;
		this->reservePtr = reservePtr;
	}
	freeBlocksTree.insert( std::make_pair( std::make_pair( minSize, MemoryStatus::MS_COMMIT ), basePtr ) );
	freeBlocksTree.insert( std::make_pair( std::make_pair( maxSize - minSize, MemoryStatus::MS_RELEASE ), 
		static_cast< void* >( static_cast< char* >( basePtr ) + minSize ) ) );
	freePointersSet.emplace( basePtr, std::make_pair( minSize, MemoryStatus::MS_COMMIT ) );
	freePointersSet.emplace( static_cast< void* >( static_cast< char* >( basePtr ) + minSize ), std::make_pair( maxSize - minSize, MemoryStatus::MS_RELEASE ) );
}

void* CHeapManager::Alloc( int size ) 
{
	size += 4;
	int pageSize = this->sysInfo.dwPageSize;
	std::multimap< std::pair< int, MemoryStatus >, void*, CMemoryKeyComparator >::iterator findRes = freeBlocksTree.lower_bound( std::make_pair( size, MemoryStatus::MS_COMMIT ) );
	//0. �������� �� ���������� �����-���� ��������� ������.
	if( findRes == freeBlocksTree.end() ) {
		throw std::exception( "No free memory to use" );
	}

	std::map< void*, std::pair< int, MemoryStatus > >::iterator ptrIter = freePointersSet.find( findRes->second );
		
	//1. ���� ������ ������	reserved, �� ����� ������� split && commit ��� ����� �������.
	if( findRes->first.second == MemoryStatus::MS_RELEASE ) {
		int toCommitSize = ( pageSize - ( size % pageSize ) ) + size;
		::VirtualAlloc( findRes->second, toCommitSize, MEM_COMMIT, PAGE_READWRITE );
		void* beginPtr = findRes->second;
		void* secondPartPtr = static_cast<void*>( static_cast<char*>( beginPtr ) + toCommitSize );
		int secondPartSize = findRes->first.first;
		freeBlocksTree.erase( findRes );
		freePointersSet.erase( ptrIter );
	
		freePointersSet.emplace( beginPtr, std::make_pair( toCommitSize, MemoryStatus::MS_COMMIT ) );
		freePointersSet.emplace( secondPartPtr, std::make_pair( secondPartSize, MemoryStatus::MS_RELEASE ) );
		
		freeBlocksTree.insert( std::make_pair( std::make_pair( toCommitSize, MemoryStatus::MS_COMMIT ), beginPtr ) );
		freeBlocksTree.insert( std::make_pair( std::make_pair( secondPartSize, MemoryStatus::MS_RELEASE ), secondPartPtr ) );
	}			

	//2.������ �� ����� �����, ��� ������ ������ ��� ��������. �������� ������ ��������������� ��.
	findRes = freeBlocksTree.lower_bound( std::make_pair( size, MemoryStatus::MS_COMMIT ) );
	//������� ������� ��������� �� ������ � ������� ������.
	int* ansPtr = static_cast<int*>( findRes->second );
	*ansPtr = size;
	void* returnPtr = static_cast<void*>( reinterpret_cast<char*>( ansPtr ) + 4 );
	int freeSize = findRes->first.first - size;
	usedBlocksTree.insert( std::make_pair( findRes->second, size ) );
	freePointersSet.erase( findRes->second );
	freeBlocksTree.erase( findRes );
	if( freeSize != 0 ) {
		char* newPtr = static_cast<char*>( returnPtr ) + size - 4;
		freeBlocksTree.insert( std::make_pair( std::make_pair( freeSize, MemoryStatus::MS_COMMIT ), static_cast<void*>( newPtr ) ) );
		freePointersSet.emplace( static_cast<void*>( newPtr ), std::make_pair( freeSize, MemoryStatus::MS_COMMIT ) );
	}
	return returnPtr;
}
 

void CHeapManager::Free( void* ptr )
{
	std::map< void*, std::pair< int, MemoryStatus > >::iterator leftBoundIter, rightBoundIter, blockIter;
	void* freePtr = static_cast<void*>( static_cast<char*>( ptr ) - 4 );
	int size = *( static_cast<int*>( freePtr ) );
	std::map< void*, int >::iterator leftUsedIter, rightUsedIter, findRes = usedBlocksTree.find( freePtr );
	if( findRes == usedBlocksTree.end() ) {
		throw std::exception( "No such allocated ptr in ptr base" );
	} else {
//		::ZeroMemory( ptr, size - 4 ); !!!!!!!!!!!!!!!!!!!!!!!!!! ��������!!!
		auto posIter = freeBlocksTree.insert( std::make_pair( std::make_pair( size, MemoryStatus::MS_COMMIT ), freePtr ) );
		usedBlocksTree.erase( findRes );
		freePointersSet.emplace( freePtr, std::make_pair( size, MemoryStatus::MS_COMMIT ) );
		//��� ��� ���������� ������ ������
		//������� �������� ������� ������� ������
		//int checkPageNumbeBeg = ( static_cast<char*>( freePtr ) - static_cast<char*>( basePtr ) ) / sysInfo.dwPageSize;
		//int toCheckPagesCount = size / sysInfo.dwPageSize + 1;
		//char* firstPageToCheckBegin = static_cast<char*>( basePtr ) + checkPageNumbeBeg * sysInfo.dwPageSize;
		//char* endSearchPtr = static_cast<char*>( freePtr ) + size - 1; //��������� �� ��������� ����
		//bool isSomeMemUsed = false;
		//leftUsedIter = usedBlocksTree.find( static_cast<void*>( firstPageToCheckBegin ) );
		//rightUsedIter = std::upper_bound( leftUsedIter, usedBlocksTree.end(), static_cast<void*>( endSearchPtr ) );
		//int count = 0;
		//for( auto it = leftUsedIter; it != rightUsedIter; ++it ) {
		//	count++;
		//}
		//if( count > 0 ) {
		//	isSomeMemUsed = true;
		//}

		//������������ ���������: ���� �������� ����� ��������� �� ���������, �� ������ ��
		auto blockIter = freePointersSet.find( freePtr );
		auto rightBlockIter = blockIter;
		rightBlockIter++;
		if( rightBlockIter != freePointersSet.end() && ( rightBlockIter->second.second == blockIter->second.second ) && 
			( static_cast<char*>( blockIter->first ) + size == static_cast<char*>( rightBlockIter->first ) ) ) {
			auto vertex1ToDelete = *rightBlockIter;
			auto vertex2ToDelete = *posIter;
			freePointersSet.erase( rightBlockIter->first );
			deleteBlock( vertex1ToDelete.first, vertex1ToDelete.second );
			deleteBlock( vertex2ToDelete.second, vertex2ToDelete.first );			
			posIter = freeBlocksTree.emplace( std::make_pair( vertex1ToDelete.second.first + size, blockIter->second.second ), blockIter->first );
			freePointersSet.erase( blockIter );
			blockIter = freePointersSet.insert( std::make_pair( posIter->second, posIter->first ) ).first;
		}
		void* newBeginPtr = blockIter->first;
		int newSize = blockIter->second.first;
		if( blockIter != freePointersSet.begin() ) {
			auto leftBlockIter = blockIter;
			leftBlockIter--;
			if( ( leftBlockIter->second.second == blockIter->second.second ) && 
				( static_cast<char*>( blockIter->first ) - size == static_cast<char*>(  leftBlockIter->first ) ) ) {
				newBeginPtr = leftBlockIter->first;
				newSize += leftBlockIter->second.first;
				auto vertex1ToDelete = *leftBlockIter;
				auto vertex2ToDelete = *posIter;
				freePointersSet.erase( blockIter );
				freePointersSet.erase( leftBlockIter );
				deleteBlock( vertex1ToDelete.first, vertex1ToDelete.second );
				deleteBlock( vertex2ToDelete.second, vertex2ToDelete.first );			
				//��� ����� �������� decommit ��� ������
				posIter = freeBlocksTree.emplace( std::make_pair( newSize, MemoryStatus::MS_COMMIT ), newBeginPtr );
				blockIter = freePointersSet.insert( std::make_pair( posIter->second, posIter->first ) ).first;
			}
		}
	}
}

CHeapManager::~CHeapManager()
{
	for( auto it = usedBlocksTree.begin(); it != usedBlocksTree.end(); it++ ) {
		std::cout << it->first << " " << it->second << "\n";	
	}
	::VirtualFree( basePtr, 0, MEM_DECOMMIT );
}
