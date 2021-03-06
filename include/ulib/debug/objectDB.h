// ============================================================================
//
// = LIBRARY
//    ulibdbg - c++ library
//
// = FILENAME
//    objectDB.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIBDBG_OBJECTDB_H
#define ULIBDBG_OBJECTDB_H 1

#include <ulib/base/base.h>

/* =======================================================================================================================
A mechanism that allow all objects to be registered with a central in-memory "database" that can dump the state of all
live objects. The macros which allow easy registration and removal of objects to be dumped (U_REGISTER_OBJECT and
U_UNREGISTER_OBJECT) are turned into no-ops by compiling with the DEBUG macro undefined. This allows usage to be removed
in "release mode" builds without changing code. There are several interesting aspects to this design:

1. It uses the External Polymorphism pattern to avoid having to derive all classes from a common base class that has
   virtual methods (this is crucial to avoid unnecessary overhead). In addition, there is no additional space added
   to objects (this is crucial to maintain binary layout compatibility)

2. This mechanism can be conditionally compiled in order to completely disable this feature entirely. Moreover, by
   using macros there are relatively few changes to code

3. This mechanism copes with single-inheritance hierarchies of dumpable classes. In such cases we typically want only
   one dump, corresponding to the most derived instance. Note, however, that this scheme doesn't generalize to work
   with multiple-inheritance or virtual base classes
========================================================================================================================== */

/** class UObjectDumpable

   Base class that defines a uniform interface for all object dumping
*/

class U_NO_EXPORT UObjectDumpable {
public:

   // Info on object

   const char* name_file;
   const char* name_class;
   const char* name_function;
   const void* ptr_object; // Pointer to the object that is being stored
   int         num_line, level;
   uint32_t    cnt, size_object;

   // Constructor

            UObjectDumpable(int lv, const char* name, const void* ptr) : name_class(name), ptr_object(ptr), level(lv) {}
   virtual ~UObjectDumpable()                                                                                         {}

   virtual const char* dump() const = 0; // This pure virtual method must be filled in by a subclass.
};

/** class UObjectDumpable_Adapter

This class inherits the interface of the abstract UObjectDumpable class and is instantiated with the implementation
of the concrete component 'class Concrete'. This design is similar to the Adapter and Decorator patterns from the
'Gang of Four' book. Note that 'class Concrete' need not inherit from a common class since UObjectDumpable
provides the uniform virtual interface!
*/

template <class Concrete> class U_NO_EXPORT UObjectDumpable_Adapter : public UObjectDumpable {
public:

   // Initialization and termination methods

   UObjectDumpable_Adapter(int _level, const char* _name_class, const Concrete* object) : UObjectDumpable(_level, _name_class, object)
      {
      U_INTERNAL_TRACE("UObjectDumpable_Adapter::UObjectDumpable_Adapter(%u,%s,%p)", _level, _name_class, object)

      UObjectDumpable::size_object = (sizeof(Concrete) > sizeof(void*) ? (uint32_t)(sizeof(Concrete) - sizeof(void*)) : 1U); // - U_MEMORY_TEST...

      U_INTERNAL_PRINT("this = %p", this)
      }

   ~UObjectDumpable_Adapter()
      {
      U_INTERNAL_TRACE("UObjectDumpable_Adapter::~UObjectDumpable_Adapter()", 0)

      U_INTERNAL_PRINT("this = %p", this)
      }

   // Concrete dump method (simply delegates to the 'dump()' method of class Concrete)

   virtual const char* dump() const __pure { return ((const Concrete*)ptr_object)->dump(true); }
};

/** class UObjectDB

This is the object database (ODB) that keeps track of all live objects
*/

class U_NO_EXPORT UObjectDB {
public:

   static int  U_EXPORT fd, level_active;
   static bool U_EXPORT flag_new_object;
   static bool U_EXPORT flag_ulib_object;

   static void close();
   static void initFork();
   static void init(bool flag, bool info);

   static void U_EXPORT   registerObject(UObjectDumpable* dumper); // Add the 'dumper' to the list of registered objects
   static void U_EXPORT unregisterObject(const void* ptr_object);  // Use 'ptr_object' to locate and remove the
                                                                   // associated 'dumper' from the list of
                                                                   // registered objects

   // Iterates through the entire set of registered objects and dumps their state

   static void     dumpObjects();
   static void     dumpObject(const UObjectDumpable* dumper);
   static uint32_t dumpObject(char* buffer, uint32_t n, bPFpcpv check_object);

private:
   static char*    file_ptr;
   static char*    file_mem;
   static char*    file_limit;
   static uint32_t file_size;

   static uint32_t n;
   static const UObjectDumpable** vec_obj_live;

   static char buffer1[64];
   static char buffer2[256];

   static char* lbuf;
   static char* lend;
   static iovec liov[7];
   static bPFpcpv checkObject;

   static void _write(const struct iovec* iov, int n) U_NO_EXPORT;
   static bool addObjLive(const UObjectDumpable* dumper) U_NO_EXPORT;
   static bool printObjLive(const UObjectDumpable* dumper) U_NO_EXPORT;
   static int  compareDumper(const void* dumper1, const void* dumper2) __pure U_NO_EXPORT;
};

#endif
