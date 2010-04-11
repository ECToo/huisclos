#ifndef __ASSERT_SWIG__
#define __ASSERT_SWIG__

// FIXME: Modify assert() behavior based on a token def: should throw(), but not abort().  Need __file, __line, &c.
//#ifdef SWIG
//#define NDEBUG
#include <cassert>
//#include <iostream>

//// Write to stdout & then throw
//inline void assert_fail_throw (__const char *__assertion, __const char *__file, unsigned int __line [>, __const char *__function<])
//{
	//using namespace std;
	//cout << "*** Assert failed: " << __assertion << " in " << __file << " at line " << __line << [>" in " << __function << <]"." << endl;
	//throw "General assertion failure.";
//}

/*
#undef assert

# define assert(expr)							\
  ((expr)								\
   ? __ASSERT_VOID_CAST (0)						\
   : assert_fail_throw(__STRING(expr), __FILE__, __LINE__, __ASSERT_FUNCTION))
*/


#endif

