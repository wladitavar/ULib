#include <ulib/all.h>

#define USP_FORM_NAME(n)  (UHTTP::getFormValue(*UClientImage_Base::_value,(0+(n*2))),*UClientImage_Base::_value)

#define USP_FORM_VALUE(n) (UHTTP::getFormValue(*UClientImage_Base::_value,(1+(n*2))),*UClientImage_Base::_value)

#define USP_PUTS(string) (void)UClientImage_Base::wbuffer->append(string)

#define USP_PRINTF(fmt,args...) (UClientImage_Base::_buffer->snprintf(fmt , ##args),USP_PUTS(*UClientImage_Base::_buffer))

#define USP_PRINTF_XML(fmt,args...) (UClientImage_Base::_buffer->snprintf(fmt , ##args),UXMLEscape::encode(*UClientImage_Base::_buffer,*UClientImage_Base::_encoded),USP_PUTS(*UClientImage_Base::_encoded))

#define USP_PRINTF_FORM_NAME(fmt,n) (USP_FORM_NAME(n),USP_PRINTF(fmt,U_STRING_TO_TRACE(*UClientImage_Base::_value)))

#define USP_PRINTF_FORM_VALUE(fmt,n) (USP_FORM_VALUE(n),USP_PRINTF(fmt,U_STRING_TO_TRACE(*UClientImage_Base::_value)))

#define USP_PRINTF_XML_FORM_NAME(fmt,n) ((void)USP_FORM_NAME(n),USP_PRINTF_XML(fmt,U_STRING_TO_TRACE(*UClientImage_Base::_value)))

#define USP_PRINTF_XML_FORM_VALUE(fmt,n) ((void)USP_FORM_VALUE(n),USP_PRINTF_XML(fmt,U_STRING_TO_TRACE(*UClientImage_Base::_value)))


extern "C" {
extern U_EXPORT int runDynamicPage(UClientImage_Base* client_image);
       U_EXPORT int runDynamicPage(UClientImage_Base* client_image)
{
  U_TRACE(0, "::runDynamicPage(%p)", client_image)

  if (client_image == 0 || client_image == (UClientImage_Base*)-1 || client_image == (UClientImage_Base*)-2) U_RETURN(0);

  U_INTERNAL_ASSERT_POINTER(UClientImage_Base::_value)
  U_INTERNAL_ASSERT_POINTER(UClientImage_Base::_buffer)
  U_INTERNAL_ASSERT_POINTER(UClientImage_Base::request)
  U_INTERNAL_ASSERT_POINTER(UClientImage_Base::rbuffer)
  U_INTERNAL_ASSERT_POINTER(UClientImage_Base::wbuffer)
  U_INTERNAL_ASSERT_POINTER(UClientImage_Base::_encoded)

(void) UClientImage_Base::wbuffer->append(U_CONSTANT_TO_PARAM("Content-Type: application/json\r\nPragma: no-cache\r\nExpires: Thu, 19 Nov 1981 08:52:00 GMT\r\nCache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0\r\n\r\n"));
USP_PUTS(UHTTP::getUploadProgress());
U_RETURN(0); } }
