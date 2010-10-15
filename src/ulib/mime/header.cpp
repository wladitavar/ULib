// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    header.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/url.h>
#include <ulib/mime/header.h>
#include <ulib/utility/uhttp.h>
#include <ulib/container/vector.h>

UString* UMimeHeader::str_name;
UString* UMimeHeader::str_ascii;
UString* UMimeHeader::str_charset;
UString* UMimeHeader::str_msg_rfc;
UString* UMimeHeader::str_boundary;
UString* UMimeHeader::str_filename;
UString* UMimeHeader::str_txt_xml;
UString* UMimeHeader::str_txt_plain;
UString* UMimeHeader::str_mime_version;
UString* UMimeHeader::str_content_transfer_encoding;

void UMimeHeader::str_allocate()
{
   U_TRACE(0, "UMimeHeader::str_allocate()")

   U_INTERNAL_ASSERT_EQUALS(str_name,0)
   U_INTERNAL_ASSERT_EQUALS(str_ascii,0)
   U_INTERNAL_ASSERT_EQUALS(str_charset,0)
   U_INTERNAL_ASSERT_EQUALS(str_msg_rfc,0)
   U_INTERNAL_ASSERT_EQUALS(str_boundary,0)
   U_INTERNAL_ASSERT_EQUALS(str_filename,0)
   U_INTERNAL_ASSERT_EQUALS(str_txt_xml,0)
   U_INTERNAL_ASSERT_EQUALS(str_txt_plain,0)
   U_INTERNAL_ASSERT_EQUALS(str_mime_version,0)
   U_INTERNAL_ASSERT_EQUALS(str_content_transfer_encoding,0)

   static ustringrep stringrep_storage[] = {
      { U_STRINGREP_FROM_CONSTANT("name") },
      { U_STRINGREP_FROM_CONSTANT("charset") },
      { U_STRINGREP_FROM_CONSTANT("us-ascii") },
      { U_STRINGREP_FROM_CONSTANT("boundary") },
      { U_STRINGREP_FROM_CONSTANT("filename") },
      { U_STRINGREP_FROM_CONSTANT("text/xml") },
      { U_STRINGREP_FROM_CONSTANT("text/plain") },
      { U_STRINGREP_FROM_CONSTANT("MIME-Version") },
      { U_STRINGREP_FROM_CONSTANT("message/rfc822") },
      { U_STRINGREP_FROM_CONSTANT("Content-Transfer-Encoding") }
   };

   U_NEW_ULIB_OBJECT(str_name,                      U_STRING_FROM_STRINGREP_STORAGE(0));
   U_NEW_ULIB_OBJECT(str_charset,                   U_STRING_FROM_STRINGREP_STORAGE(1));
   U_NEW_ULIB_OBJECT(str_ascii,                     U_STRING_FROM_STRINGREP_STORAGE(2));
   U_NEW_ULIB_OBJECT(str_boundary,                  U_STRING_FROM_STRINGREP_STORAGE(3));
   U_NEW_ULIB_OBJECT(str_filename,                  U_STRING_FROM_STRINGREP_STORAGE(4));
   U_NEW_ULIB_OBJECT(str_txt_xml,                   U_STRING_FROM_STRINGREP_STORAGE(5));
   U_NEW_ULIB_OBJECT(str_txt_plain,                 U_STRING_FROM_STRINGREP_STORAGE(6));
   U_NEW_ULIB_OBJECT(str_mime_version,              U_STRING_FROM_STRINGREP_STORAGE(7));
   U_NEW_ULIB_OBJECT(str_msg_rfc,                   U_STRING_FROM_STRINGREP_STORAGE(8));
   U_NEW_ULIB_OBJECT(str_content_transfer_encoding, U_STRING_FROM_STRINGREP_STORAGE(9));
}

//=============================================================================
// parsing an UString for MIME headers.
// 
// Mime-type headers comprise a keyword followed by ":" followed
// by whitespace and the value. If a line begins with witespace it is
// a continuation of the preceeding line.
//
// MIME headers are delimited by an empty line.
//=============================================================================

uint32_t UMimeHeader::parse(const UString& buffer)
{
   U_TRACE(0+256, "UMimeHeader::parse(%.*S)", U_STRING_TO_TRACE(buffer))

   header = buffer;
   char* ptr = buffer.data();
   uint32_t n = table.size(), pos, pos1 = 0, pos2, end = buffer.size();

   U_INTERNAL_DUMP("n = %u, end = %u", n, end)

   bool cr;
   UString key, value;

   // Until we reach the data part of the response we know we are dealing with iso-8859-1 characters

   if (*ptr == '\n') U_RETURN(0); // line empty: we have reached the MIME headers delimiter

   while (pos1 < end)
      {
      U_INTERNAL_DUMP("buffer = %.*S", 80, buffer.c_pointer(pos1))

      pos2 = buffer.find('\n', pos1);

      if (pos2 == U_NOT_FOUND) pos2 = end; // we have reached the MIME headers end without line empty...

      cr = (ptr[--pos2] == '\r');

      U_INTERNAL_DUMP("pos1 = %u, pos2 = %u cr = %b", pos1, pos2, cr)

      if (cr == false) ++pos2;

      if (pos2 == pos1) break; // line empty: we have reached the MIME headers delimiter

      // Check for continuation of preceding header

      if (u_isspace(ptr[pos1]))
         {
         do { ++pos1; } while (pos1 < end && u_isspace(ptr[pos1]));

         U_INTERNAL_ASSERT_MINOR(pos1,pos2)

         if (value.empty() == false)
            {
            value.append(ptr + pos1, pos2 - pos1);

            table.replaceAfterFind(value);
            }
         }
      else
         {
         pos = buffer.find(':', pos1);

         U_INTERNAL_DUMP("pos = %u, pos2 = %u", pos, pos2)

         if (pos >= pos2) break;
   
      // U_INTERNAL_ASSERT_MINOR(pos,pos2)

         key = buffer.substr(pos1, pos - pos1);

         do { ++pos; } while (pos < end && u_isspace(ptr[pos]));

         value = buffer.substr(pos, pos2 - pos);

         table.insert(key, value);
         }

      pos1 = pos2 + 1 + cr;
      }

   U_RETURN(table.size() - n);
}

// inlining failed in call to 'UMimeHeader::removeHeader(UString const&)': call is unlikely and code size would grow

void UMimeHeader::removeHeader(const UString& key)
{
   U_TRACE(0, "UMimeHeader::removeHeader(%.*S)", U_STRING_TO_TRACE(key))

   if (containsHeader(key)) table.eraseAfterFind();
}

uint32_t UMimeHeader::getAttributeFromKeyValue(const UString& key_value, UVector<UString>& name_value)
{
   U_TRACE(0, "UMimeHeader::getAttributeFromKeyValue(%.*S,%p)", U_STRING_TO_TRACE(key_value), &name_value)

   uint32_t n = 0;

   if (key_value.empty() == false)
      {
      // Content-Disposition: form-data; name="input_file"; filename="/tmp/4dcd39e8-2a84-4242-b7bc-ca74922d26e1"

      uint32_t pos = key_value.find(';');

      if (pos != U_NOT_FOUND)
         {
         // NB: da notare che il risultato dipende da una substring che muore...

         n = name_value.split(key_value.substr(pos + 1), " =;"); // NB: non posso usare anche '"'...

         U_INTERNAL_ASSERT( n >= 2)
         U_INTERNAL_ASSERT((n  & 1) == 0) // pari...
         }
      }

   U_RETURN(n);
}

UString UMimeHeader::getValueAttributeFromKeyValue(const UString& name_attr, UVector<UString>& name_value, bool ignore_case)
{
   U_TRACE(0, "UMimeHeader::getValueAttributeFromKeyValue(%.*S,%p,%b)", U_STRING_TO_TRACE(name_attr), &name_value, ignore_case)

   UString value_name;

   for (int32_t i = 0, n = name_value.size(); i < n; ++i)
      {
      if (name_value[i++].equal(name_attr, ignore_case))
         {
         value_name = name_value[i];

         if (value_name.isQuoted()) value_name.unQuote();

         break;
         }
      }

   U_INTERNAL_DUMP("value_name = %.*S", U_STRING_TO_TRACE(value_name))

   U_RETURN_STRING(value_name);
}

UString UMimeHeader::getValueAttributeFromKeyValue(const UString& key_value, const UString& name_attr, bool ignore_case)
{
   U_TRACE(0, "UMimeHeader::getValueAttributeFromKeyValue(%.*S,%.*S,%b)", U_STRING_TO_TRACE(key_value),
                                                                          U_STRING_TO_TRACE(name_attr), ignore_case)

   UString value_name;
   UVector<UString> name_value;

   if (getAttributeFromKeyValue(key_value, name_value)) value_name = getValueAttributeFromKeyValue(name_attr, name_value, ignore_case);

   U_RETURN_STRING(value_name);
}

bool UMimeHeader::getNames(const UString& cdisposition, UString& name, UString& filename)
{
   U_TRACE(0, "UMimeHeader::getNames(%.*S,%p,%p)", U_STRING_TO_TRACE(cdisposition), &name, &filename)

   bool result = false;
   UVector<UString> name_value;

   // Content-Disposition: form-data; name="input_file"; filename="/tmp/4dcd39e8-2a84-4242-b7bc-ca74922d26e1"

   if (getAttributeFromKeyValue(cdisposition, name_value))
      {
      for (int32_t i = 0, n = name_value.size(); i < n; i += 2)
         {
         if (name_value[i].equal(*str_name, false))
            {
            name = name_value[i+1];

            U_INTERNAL_DUMP("name_value[%d] = %.*S", i+1, U_STRING_TO_TRACE(name))

            if (name.isQuoted()) name.unQuote();

            U_INTERNAL_DUMP("name = %.*S", U_STRING_TO_TRACE(name))
            }
         else if (name_value[i].equal(*str_filename, false))
            {
            filename = name_value[i+1];

            U_INTERNAL_DUMP("name_value[%d] = %.*S", i+1, U_STRING_TO_TRACE(filename))

            if (filename.empty() == false)
               {
               // This is hairy: Netscape and IE don't encode the filenames
               // The RFC says they should be encoded, so I will assume they are

               uint32_t len = filename.size();

               UString filename_decoded(len);

               Url::decode(filename.data(), len, filename_decoded);

               filename = filename_decoded;

               if (filename.isQuoted()) filename.unQuote();

               U_INTERNAL_DUMP("filename = %.*S", U_STRING_TO_TRACE(filename))

               result = true;
               }
            }
         }
      }

   U_RETURN(result);
}

UString UMimeHeader::getCharSet(const UString& content_type)
{
   U_TRACE(0, "UMimeHeader::getCharSet(%.*S)", U_STRING_TO_TRACE(content_type))

   U_ASSERT(content_type.empty() == false)

   UString charset = getValueAttributeFromKeyValue(content_type, *str_charset, false);

   // Per some RFC, encoding is us-ascii if it's not specifief in header.

   if (charset.empty()) charset = *str_ascii;

   U_INTERNAL_DUMP("charset = %.*S", U_STRING_TO_TRACE(charset))

   U_RETURN_STRING(charset);
}

UString UMimeHeader::shortContentType(const UString& content_type)
{
   U_TRACE(0, "UMimeHeader::shortContentType(%.*S)", U_STRING_TO_TRACE(content_type))

   U_ASSERT(content_type.empty() == false)

   uint32_t pos = content_type.find(';');

   if (pos != U_NOT_FOUND)
      {
      UString result = content_type.substr(0U, pos);

      U_RETURN_STRING(result);
      }

   U_RETURN_STRING(content_type);
}

void UMimeHeader::writeHeaders(UString& buffer)
{
   U_TRACE(0, "UMimeHeader::writeHeaders(%.*S)", U_STRING_TO_TRACE(buffer))

   UStringRep* key;
   UStringRep* value;
   UString tmp(U_CAPACITY);

   if (table.first())
      {
      do {
         key   = table.key();
         value = table.elem();

         tmp.snprintf("%.*s: %.*s\r\n", key->size(), key->data(), value->size(), value->data());

         buffer += tmp;
         }
      while (table.next());
      }

   U_INTERNAL_DUMP("buffer = %.*S", U_STRING_TO_TRACE(buffer))
}

UString UMimeHeader::getHeaders()
{
   U_TRACE(0, "UMimeHeader::getHeaders()")

   if (empty() == false)
      {
      UString buffer(U_CAPACITY);

      writeHeaders(buffer);

      U_RETURN_STRING(buffer);
      }

   U_RETURN_STRING(UString::getStringNull());
}

// read from socket

bool UMimeHeader::readHeader(USocket* socket, UString& data)
{
   U_TRACE(0, "UMimeHeader::readHeader(%p,%p)", socket, &data)

   U_ASSERT(empty() == true)

   UHTTP::resetHTTPInfo();

   bool result = (UHTTP::readHTTPHeader(socket, data)
                     ? parse(U_HTTP_HEADER(data)) > 0
                     : false);

   U_RETURN(result);
}

// STREAM

U_EXPORT ostream& operator<<(ostream& os, UMimeHeader& h)
{
   U_TRACE(0+256, "UMimeHeader::operator<<(%p,%p)", &os, &h)

   UStringRep* key;
   UStringRep* value;

   if (h.table.first())
      {
      do {
         key   = h.table.key();
         value = h.table.elem();

         os.write(key->data(), key->size());
         os.write(": ", 2);
         os.write(value->data(), value->size());
         os.write(U_CONSTANT_TO_PARAM(U_CRLF));
         }
      while (h.table.next());
      }

   os.write(U_CONSTANT_TO_PARAM(U_CRLF));

   return os;
}

// DEBUG

#ifdef DEBUG
#  include <ulib/internal/objectIO.h>

const char* UMimeHeader::dump(bool reset) const
{
   *UObjectIO::os << "table     (UHashMap " << (void*)&table  << ")\n"
                  << "header    (UString  " << (void*)&header << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

#endif
