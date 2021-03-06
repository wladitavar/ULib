// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    memory_pool.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_MEMORY_POOL_H
#define ULIB_MEMORY_POOL_H 1

// ---------------------------------------------------------------------------------------------------------------
// U_MAX_SIZE_PREALLOCATE: richiesta massima soddisfatta con metodo di preallocazione altrimenti chiamata a malloc

#define U_MAX_SIZE_PREALLOCATE 4096U

// U_STACK_TYPE_* 'tipi' stack per cui la richiesta viene gestita tramite preallocazione

/* NO DEBUG (64 bit)
-------------------------
   1 sizeof(UMagic)
   1 sizeof(UNotifier)
   8 sizeof(UCrl)
   8 sizeof(UPKCS10)
   8 sizeof(UString) <==
   8 sizeof(UCertificate)
-------------------------
   U_STACK_TYPE_0

  12 sizeof(UProcess)
  16 sizeof(UTimeDate)
  16 sizeof(ULock)
  16 sizeof(UTimer)
  16 sizeof(UPKCS7)
  16 sizeof(UTimeVal)
  16 sizeof(USemaphore)
  16 sizeof(UXMLParser)
  16 sizeof(UVector<UString>)
  24 sizeof(UStringRep) <==
  24 sizeof(USOAPObject)
-------------------------
   U_STACK_TYPE_1

  32 sizeof(UCache)
  32 sizeof(ULDAPEntry)
  32 sizeof(UQueryNode)
  32 sizeof(USOAPFault)
  32 sizeof(UTokenizer)
  32 sizeof(UHashMapNode)                                                      
  32 sizeof(UXMLAttribute)
  32 sizeof(UTree<UString>)
  40 sizeof(Url)
  40 sizeof(ULDAP)
  40 sizeof(UHashMap<UString>)
  48 sizeof(UCURL)
  48 sizeof(UDialog)
  48 sizeof(UMimeHeader)
  48 sizeof(UMimeEntity)
  48 sizeof(UQueryParser)
  48 sizeof(USOAPEncoder)
  48 sizeof(UXMLElement)
  48 sizeof(USOAPGenericMethod)
  56 sizeof(UOptions)
  56 sizeof(UPlugIn<void*>)
  56 sizeof(UHTTP::UFileCacheData) <==
-------------------------
   U_STACK_TYPE_2

  64 sizeof(UPCRE)
  64 sizeof(UCommand)
  64 sizeof(UApplication)
  72 sizeof(UMimePKCS7)
  72 sizeof(UClientImage<UTCPSocket>)
  80 sizeof(UZIP)
  88 sizeof(UIPAddress)
  88 sizeof(UMimeMultipartMsg)
  96 sizeof(UMimeMessage)
 120 sizeof(UServer<UTCPSocket>)
 128 sizeof(URDBServer)
 128 sizeof(USOAPParser)
 128 sizeof(UMimeMultipart)
-------------------------
   U_STACK_TYPE_3

 184 sizeof(ULog)
 184 sizeof(UFile)
 192 sizeof(URDBClient<UTCPSocket>)
 216 sizeof(UBison)
 216 sizeof(UFlexer)
 216 sizeof(USocket)
 216 sizeof(UTCPSocket)
 216 sizeof(UUDPSocket)
 240 sizeof(USSLSocket)
 240 sizeof(UModNoCatPeer: 32bit) <==
-------------------------
   U_STACK_TYPE_4

 248 sizeof(UHttpClient<UTCPSocket>)
 256 sizeof(UFileConfig)
-------------------------
   U_STACK_TYPE_5

 264 sizeof(USSHSocket)
 296 sizeof(UCDB)
 296 sizeof(USmtpClient)
 304 sizeof(USOAPClient<UTCPSocket>)
 496 sizeof(UFtpClient)
 512 sizeof(URDB)
-------------------------
   U_STACK_TYPE_6

 1024
-------------------------
   U_STACK_TYPE_7

 2048
-------------------------
   U_STACK_TYPE_8

 4096
-------------------------
   U_STACK_TYPE_9
*/

// NB: con U_NUM_ENTRY_MEM_BLOCK == 32 sono necessari i tipi stack
//     multipli di 2 a partire da 128 per i blocchi puntatori per 32bit arch...

#define U_STACK_TYPE_0     8U
#define U_STACK_TYPE_1    24U
#define U_STACK_TYPE_2    56U
#define U_STACK_TYPE_3   128U
#define U_STACK_TYPE_4   240U
#define U_STACK_TYPE_5   256U
#define U_STACK_TYPE_6   512U
#define U_STACK_TYPE_7  1024U
#define U_STACK_TYPE_8  2048U
#define U_STACK_TYPE_9  U_MAX_SIZE_PREALLOCATE

// U_NUM_STACK_TYPE: numero 'tipi' stack per cui la richiesta viene gestita tramite preallocazione

#define U_NUM_STACK_TYPE 10

/* Implements a simple stack allocator */

#define U_SIZE_TO_STACK_INDEX(sz)  ((sz) <= U_STACK_TYPE_0 ? 0 : \
                                    (sz) <= U_STACK_TYPE_1 ? 1 : \
                                    (sz) <= U_STACK_TYPE_2 ? 2 : \
                                    (sz) <= U_STACK_TYPE_3 ? 3 : \
                                    (sz) <= U_STACK_TYPE_4 ? 4 : \
                                    (sz) <= U_STACK_TYPE_5 ? 5 : \
                                    (sz) <= U_STACK_TYPE_6 ? 6 : \
                                    (sz) <= U_STACK_TYPE_7 ? 7 : \
                                    (sz) <= U_STACK_TYPE_8 ? 8 : 9)

struct U_EXPORT UMemoryPool {

#if defined(DEBUG) || defined(U_TEST)
   static sig_atomic_t index_stack_busy; // Segnala operazione in corso su stack (per check rientranza)
#endif

// static uint32_t findStackIndex(uint32_t sz);

   // allocazione area di memoria <= U_MAX_SIZE_PREALLOCATE
   // ritorna area di memoria preallocata su stack predefiniti
   // (non garantito azzerata a meno di azzerare le deallocazioni...)

   static void* pop(            int stack_index);
   static void  push(void* ptr, int stack_index);

   static void* _malloc(size_t sz);
   static void _free(void* ptr, size_t sz);

   // special for class UString

   static void* _malloc_str(           size_t sz, uint32_t& capacity);
   static void    _free_str(void* ptr, size_t sz);

#ifdef U_OVERLOAD_NEW_DELETE
   // funzioni allocazione e deallocazione sostitutive a new() e delete()

   static void  _delete(void* ptr);
   static void* _new(int stack_index);
#endif

#ifdef DEBUG
   static void printInfo();
#endif
};

#ifdef U_OVERLOAD_NEW_DELETE // A macro for requiring the use of our versions of the C++ operators.

// The inlined new e delete operator (used by applications)

#include <new>

extern "C++" {
   inline void* operator new(std::size_t sz) throw(std::bad_alloc)
      {
      U_TRACE(0, "::new(%lu)", sz)

      U_INTERNAL_ASSERT_MAJOR(sz, 0)

      void* ptr = (sz <= U_MAX_SIZE_PREALLOCATE
                     ? UMemoryPool::_new(U_SIZE_TO_STACK_INDEX(sz + sizeof(int)))
                     : U_SYSCALL(malloc, "%u", sz));

      U_RETURN(ptr);
      }

   inline void* operator new(std::size_t sz, const std::nothrow_t& nothrow) throw()
      {
      U_TRACE(0, "::new(%lu,%p)", sz, &nothrow)

      return ::operator new(sz);
      }

   inline void* operator new[](std::size_t sz) throw (std::bad_alloc)
      {
      U_TRACE(0, "::new[](%lu)", sz)

      return ::operator new(sz);
      }

   inline void* operator new[](std::size_t sz, const std::nothrow_t& nothrow) throw()
      {
      U_TRACE(0, "::new[](%lu,%p)", sz, &nothrow)

      return ::operator new(sz);
      }

   inline void operator delete(void* ptr) throw()
      {
      U_TRACE(0, "::delete(%p)", ptr)

      UMemoryPool::_delete(ptr);
      }

   inline void operator delete(void* ptr, const std::nothrow_t& nothrow) throw()
      {
      U_TRACE(0, "::delete(%p,%p)", ptr, &nothrow)

      UMemoryPool::_delete(ptr);
      }

   inline void operator delete[](void* ptr) throw()
      {
      U_TRACE(0, "::delete[](%p)", ptr)

      UMemoryPool::_delete(ptr);
      }

   inline void operator delete[](void* ptr, const std::nothrow_t& nothrow) throw()
      {
      U_TRACE(0, "::delete[](%p,%p)", ptr, &nothrow)

      UMemoryPool::_delete(ptr);
      }
}
#endif

#endif
