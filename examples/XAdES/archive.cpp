// archive.cpp

#include <ulib/url.h>
#include <ulib/date.h>
#include <ulib/file_config.h>
#include <ulib/utility/base64.h>
#include <ulib/utility/services.h>
#include <ulib/utility/string_ext.h>
#include <ulib/xml/libxml2/schema.h>

#ifdef HAVE_SSL_TS
#  include <ulib/ssl/timestamp.h>
#endif

#undef  PACKAGE
#define PACKAGE "archive"

#define ARGS "[ ARCHIVE_TIMESTAMP SCHEMA ]"

#define U_OPTIONS \
"purpose \"XAdES signature (L)\"\n" \
"option c config 1 \"path of configuration file\" \"\"\n"

#include <ulib/application.h>

#define U_ARCHIVE_TIMESTAMP (const char*)(argv[optind+0])
#define U_SCHEMA            (const char*)(argv[optind+1])

// ArchiveTimeStamp

#define U_XADES_ARCHIVE_TIMESTAMP_TEMPLATE \
"          <xadesv141:ArchiveTimeStamp xmlns:xadesv141=\"http://uri.etsi.org/01903/v1.4.1#\">\r\n" \
"%.*s" \
"          </xadesv141:ArchiveTimeStamp>\r\n" \
"        </xades:UnsignedSignatureProperties>"

class Application : public UApplication {
public:

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      vec.clear();
      }

   UString getTimeStampToken(const UString& data, const UString& url)
      {
      U_TRACE(5, "Application::getTimeStampToken(%.*S,%.*S)", U_STRING_TO_TRACE(data), U_STRING_TO_TRACE(url))

      U_ASSERT(url.empty() == false)

      Url TSA(url);
      UString token;

#  ifdef HAVE_SSL_TS
      UString request = UTimeStamp::createQuery(U_HASH_SHA1, data, 0, false, false);

      UTimeStamp ts(request, TSA);

      if (ts.UPKCS7::isValid()) token = ts.UPKCS7::getEncoded("BASE64");
#  endif

      U_RETURN_STRING(token);
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage options

      if (UApplication::isOptions()) cfg_str = opt['c'];

      // manage file configuration

      if (cfg_str.empty()) cfg_str = U_STRING_FROM_CONSTANT("XAdES.ini");

      // ----------------------------------------------------------------------------------------------------------------------------------
      // XAdES signature - configuration parameters
      // ----------------------------------------------------------------------------------------------------------------------------------
      // DigestAlgorithm   md2 | md5 | sha | sha1 | sha224 | sha256 | sha384 | sha512 | mdc2 | ripmed160
      //
      // SigningTime this property contains the time at which the signer claims to have performed the signing process (yes/no)
      // ClaimedRole this property contains claimed or certified roles assumed by the signer in creating the signature
      //
      // this property contains the indication of the purported place where the signer claims to have produced the signature
      // -------------------------------------------------------------------------------------------------------------------
      // ProductionPlaceCity
      // ProductionPlaceStateOrProvince
      // ProductionPlacePostalCode
      // ProductionPlaceCountryName
      // -------------------------------------------------------------------------------------------------------------------
      //
      // DataObjectFormatMimeType   this property identifies the format of a signed data object (when electronic signatures
      //                            are not exchanged in a restricted context) to enable the presentation to the verifier or
      //                            use by the verifier (text, sound or video) in exactly the same way as intended by the signer
      //
      // CAStore
      //
      // ArchiveTimeStamp           the time-stamp token within this property covers the archive validation data
      // SignatureTimeStamp         the time-stamp token within this property covers the digital signature value element
      //
      // Schema                     the pathname XML Schema of XAdES
      // ----------------------------------------------------------------------------------------------------------------------------------

      (void) cfg.open(cfg_str);

      (void) UServices::read(STDIN_FILENO, x);

      if (x.empty()) U_ERROR("cannot read data from <stdin>...", 0);

      content.setBuffer(x.size());

      if (UBase64::decode(x, content) == false) U_ERROR("decoding data read failed...", 0);

      archive_timestamp = ( U_ARCHIVE_TIMESTAMP == 0 ||
                           *U_ARCHIVE_TIMESTAMP == '\0'
                              ? cfg[U_STRING_FROM_CONSTANT("XAdES-L.ArchiveTimeStamp")]
                              : UString(U_ARCHIVE_TIMESTAMP));

      if (archive_timestamp.empty()) U_ERROR("error on archive timestamp: empty", 0);

      schema = ( U_SCHEMA == 0 ||
                *U_SCHEMA == '\0'
                  ? cfg[U_STRING_FROM_CONSTANT("XAdES-Verify.Schema")]
                  : UString(U_SCHEMA));

      if (schema.empty()) U_ERROR("error on XAdES schema: empty", 0);

      UXML2Schema XAdES_schema(UFile::contentOf(schema));

      UXML2Document document(content);

      if (XAdES_schema.validate(document) == false) U_ERROR("error on input data: not XAdES", 0);

      // manage arguments...

      /*
      The input to the computation of the digest value MUST be built as follows:

      1) Initialize the final octet stream as an empty octet stream.

      2) Take all the ds:Reference elements in their order of appearance within ds:SignedInfo referencing
         whatever the signer wants to sign including the SignedProperties element. Process each one as indicated below:

         - Process the retrieved ds:Reference element according to the reference processing model of XMLDSIG.

         - If the result is a XML node set, canonicalize it. If ds:Canonicalization is present, the algorithm
           indicated by this element is used. If not, the standard canonicalization method specified by XMLDSIG
           is used.

         - Concatenate the resulting octets to the final octet stream.
      */

      uint32_t i, n = UXML2Document::getElement(content, vec, U_CONSTANT_TO_PARAM("ds:Reference"));

      for (i = 0; i < n; ++i)
         {
         x = vec[i];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      /*
      3) Take the following XMLDSIG elements in the order they are listed below, canonicalize each one and
         concatenate each resulting octet stream to the final octet stream:

         - The ds:SignedInfo element.
         - The ds:SignatureValue element.
         - The ds:KeyInfo element, if present.
      */

      if (UXML2Document::getElement(content, vec, U_CONSTANT_TO_PARAM("ds:SignedInfo")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      if (UXML2Document::getElement(content, vec, U_CONSTANT_TO_PARAM("ds:SignatureValue")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      if (UXML2Document::getElement(content, vec, U_CONSTANT_TO_PARAM("ds:KeyInfo")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      /*
      4) Take the unsigned signature properties that appear before the current xadesv141:ArchiveTimeStamp
         in the order they appear within the xades:UnsignedSignatureProperties, canonicalize each one and
         concatenate each resulting octet stream to the final octet stream. While concatenating the following
         rules apply:

         - The xades:CertificateValues property MUST be added if it is not already present and the
           ds:KeyInfo element does not contain the full set of certificates used to validate the electronic signature.

         - The xades:RevocationValues property MUST be added if it is not already present and the
           ds:KeyInfo element does not contain the revocation information that has to be shipped with the electronic signature.

         - The xades:AttrAuthoritiesCertValues property MUST be added if not already present and the following
           conditions are true: there exist an attribute certificate in the signature AND a number of certificates
           that have been used in its validation do not appear in CertificateValues. Its content will satisfy with
           the rules specified in clause 7.6.3.

         - The xades:AttributeRevocationValues property MUST be added if not already present and there the following
           conditions are true: there exist an attribute certificate AND some revocation data that have been used in
           its validation do not appear in RevocationValues. Its content will satisfy with the rules specified in clause 7.6.4.
      */

      if (UXML2Document::getElement(content, vec, U_CONSTANT_TO_PARAM("xades:CompleteCertificateRefs")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      if (UXML2Document::getElement(content, vec, U_CONSTANT_TO_PARAM("xades:CertificateValues")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      if (UXML2Document::getElement(content, vec, U_CONSTANT_TO_PARAM("xades:CompleteRevocationRefs")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      if (UXML2Document::getElement(content, vec, U_CONSTANT_TO_PARAM("xades:RevocationValues")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      if (UXML2Document::getElement(content, vec, U_CONSTANT_TO_PARAM("xades:SignatureTimeStamp")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      /*
      5) Take all the ds:Object elements except the one containing xades:QualifyingProperties element. Canonicalize each one
         and concatenate each resulting octet stream to the final octet stream. If ds:Canonicalization is present, the algorithm
         indicated by this element is used. If not, the standard canonicalization method specified by XMLDSIG is used.
      */

      n = UXML2Document::getElement(content, vec, U_CONSTANT_TO_PARAM("ds:Object"));

      for (i = 0; i < n; ++i)
         {
         x = vec[i];

         if (x.find("xades:QualifyingProperties") == U_NOT_FOUND) to_digest += UXML2Document::xmlC14N(x);
         }

      u_line_terminator_len = 2;
      u_base64_max_columns  = U_OPENSSL_BASE64_MAX_COLUMN;

      UString archiveTimeStamp(U_CAPACITY), token = getTimeStampToken(to_digest, archive_timestamp);

      archiveTimeStamp.snprintf(U_XADES_ARCHIVE_TIMESTAMP_TEMPLATE, U_STRING_TO_TRACE(token));

      output = UStringExt::substitute(content, U_STRING_FROM_CONSTANT("        </xades:UnsignedSignatureProperties>"), archiveTimeStamp);

      cout.write(U_STRING_TO_PARAM(output));
      }

private:
   UFileConfig cfg;
   UVector<UString> vec;
   UString cfg_str, content, to_digest, x, archive_timestamp, schema, output;
};

U_MAIN(Application)
