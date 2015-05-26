#include <utility>
#include <map>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <assert.h>
#include <memory>
#include "object.hpp"
#include <heapmanager.h>
#include <ctime>
#include <ratio>
#include <chrono>

static size_t g_mem_usage = 0;
CHeapManager& manager = CHeapManager::CreateInstance( 30000, 100000);

void* operator new( size_t size ) {
	void *p = malloc( size + sizeof( size_t ) );
//	void* p = manager.Alloc( size );
	*( size_t* )p = size;
	g_mem_usage += size;
//	std::cout << " creating "<< (size_t*)p + 1 << '\n';
	return ( size_t* )p + 1;
}

void operator delete( void *p ) {
//	std::cout << "deleting " << p << '\n';
	size_t* smart_point = ( size_t* ) p;
	g_mem_usage -= *--smart_point;
	free( smart_point );
//	manager.Free( p );
}


class BaseObject: public GarbageCollector::Object {
private:
	int* i;
public:
	BaseObject() {
		i = new int;
	}
	std::vector< GarbageCollector::Object* > AllPointers() {
		return std::vector< GarbageCollector::Object* >();
	}
	~BaseObject() {
		delete i;
	}
};

class BadObject: public GarbageCollector::Object {
public:
	BaseObject *tmp;
	BadObject() {
		tmp = new BaseObject();
	}
	std::vector< GarbageCollector::Object* > AllPointers() {
		std::vector< GarbageCollector::Object* > l;
		l.push_back(tmp);
		return l;
	}
	~BadObject() {
	 
	}
};

class VeryBadObject: public GarbageCollector::Object {
public:
	std::vector< GarbageCollector::Object* > AllPointers() {
		std::vector< GarbageCollector::Object* > tmp;
		tmp.push_back(obj);
		return tmp;
	}
	
	VeryBadObject *obj;
	
	~VeryBadObject() {

	}
};

class NotBadObject:public GarbageCollector::Object {
private:
	VeryBadObject *obj1;
	VeryBadObject *obj2;
public:
	std::vector< GarbageCollector::Object* > AllPointers() {
		std::vector< GarbageCollector::Object* > tmp;
		tmp.push_back(obj1);
		tmp.push_back(obj2);
		return tmp;
	}
	NotBadObject() {
		obj1 = new VeryBadObject();
		obj2 = new VeryBadObject();
		obj1->obj = obj2;
		obj2->obj = obj1;
	}
};

void test1() {
	BaseObject tmp;
	std::cout << g_mem_usage << '\n';
	GarbageCollector::CollectAllGarbage();
}

void test2() {
	NotBadObject obj;
	std::cout << g_mem_usage << '\n';
	GarbageCollector::CollectAllGarbage();
	std::cout << g_mem_usage << '\n';
}

void coverageTest() {
	void* val1 = manager.Alloc( sizeof( int ) );
	void* val2 = manager.Alloc( sizeof( int ) );
	void* val3 = manager.Alloc( sizeof( int ) );
	manager.Free( val3 );
	manager.Free( val1 );
	manager.Free( val2 );
}

void CHeapManagerPerfomanceTest() 
{
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	std::vector< int* > ptrArray( 10000 );
	for( int i = 0; i < 10000; i++ ) {
		ptrArray[i] = reinterpret_cast< int* >( manager.Alloc( sizeof( int ) ) );
		*ptrArray[i] = i;
	}
	std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
	std::chrono::duration<double> durTime = std::chrono::duration_cast< std::chrono::duration<double> >( t2 - t1 );
	std::cout << "Test Alloc() time: " << durTime.count() << std::endl;
	for( int i = 0; i < 10000; i++ ) {
		manager.Free( ptrArray[i] );
	}
	std::chrono::steady_clock::time_point t3 = std::chrono::steady_clock::now();
	durTime = std::chrono::duration_cast< std::chrono::duration<double> >( t3 - t2 );
	std::cout << "Test Free() time: " << durTime.count()  << std::endl;
}

void baseAllocatorPerfomanceTest() 
{
	std::cout << "Operator new and delete perfomance test:" << std::endl;
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	std::vector< int* > ptrArray( 10000 );
	for( int i = 0; i < 10000; i++ ) {
		ptrArray[i] = new int;
		*ptrArray[i] = i;
	}
	std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
	std::chrono::duration<double> durTime = std::chrono::duration_cast< std::chrono::duration<double> >( t2 - t1 );
	std::cout << "Test Alloc() time: " << durTime.count() << std::endl;
	for( int i = 0; i < 10000; i++ ) {
		delete ptrArray[i];
	}
	std::chrono::steady_clock::time_point t3 = std::chrono::steady_clock::now();
	durTime = std::chrono::duration_cast< std::chrono::duration<double> >( t3 - t2 );
	std::cout << "Test Free() time: " << durTime.count()  << std::endl;
}


int main() {
	//std::cout << "GC test1:\n";
	//test1();
	//std::cout << "GC Memory after test1: " << g_mem_usage << '\n';
	//
	//std::cout << "GC test2:\n";
	//test2();
	//std::cout << "GC Memory after test2: " << g_mem_usage << '\n';

	//GarbageCollector::CollectAllGarbage();
	//std::cout << "GC Memory after global CollectAllGarbage()"  << g_mem_usage << "\n";

	//GarbageCollector::FreeMemory();
	//std::cout << g_mem_usage << '\n';
	try {
		coverageTest();
		CHeapManagerPerfomanceTest();
		baseAllocatorPerfomanceTest();
	} catch ( std::exception& e ) {
		std::cout << e.what();
	}
	return 0;
}
