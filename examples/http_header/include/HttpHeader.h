// HttpHeader.h

#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H 1

#include <HttpField.h>

#ifndef NO_ULIB
template <>
inline void u_construct(HttpField* f)
{
   U_TRACE(0, "u_construct<HttpField>(%p)", f)
}

template <>
inline void u_construct(HttpField* f, unsigned n)
{
   U_TRACE(0, "u_construct<HttpField>(%p,%u)", f, n)
}

template <>
inline void u_destroy(HttpField* f)
{
   U_TRACE(0, "u_destroy<HttpField>(%p)", f)
}

template <>
inline void u_destroy(HttpField** ptr, unsigned n)
{
   U_TRACE(0, "u_destroy<HttpField>(%p,%u)", ptr, n)
}
#endif

class HttpHeader {
public:
   //// Check for memory error
   U_MEMORY_TEST

   /// = Allocator e Deallocator.
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   HttpHeader()
      {
      U_TRACE_REGISTER_OBJECT(5, HttpHeader, "")
      }

   /** Destructor of the class.
   */
   ~HttpHeader()
      {
      U_TRACE_UNREGISTER_OBJECT(0, HttpHeader)
      }

   /**
   * @param name_ Field name to search in headers
   * return: the number of field founded in headers
   */
   unsigned count(const UString& name_) __pure;

   /**
   * @param header_ Header to add to headers
   * @param index_  Index of header to replace or 0 for addition without replace
   */
   HttpField* add(HttpField* field, unsigned index_ = 0);

   /**
   * @param name_  Field name to search in headers
   * @param index_ Index of header to get
   */
   HttpField* find(const UString& name_, unsigned index_ = 0) __pure;

   /**
   * @param name_  Field to delete in headers
   * @param index_ Index of header to delete
   */
   HttpField* del(const UString& name_, unsigned index_ = 0);

   /**
   * @param field Field to delete in headers
   */
   HttpField* del(HttpField* field);

   /**
   * @param field_ String where to save header as string
   */
   void stringify(UString& field);

   void clear();

   /// DEBUG

#ifdef DEBUG
   const char* dump(bool reset) const;
#endif

protected:
   UVector<HttpField*> header;
};

#endif
