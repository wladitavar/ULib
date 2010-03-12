// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    context.cpp - xml Digital SIGnature with libxml2 
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/base64.h>

#include "xpath.h"
#include "context.h"

#ifdef LIBXML_FTP_ENABLED 
#  include <libxml/nanoftp.h>
#endif
#ifdef LIBXML_HTTP_ENABLED
#  include <libxml/nanohttp.h>
#endif

UDSIGContext*     UDSIGContext::pthis;
UVector<UString>* UTransformCtx::enabledTransforms;

void UTransformCtx::registerDefault()
{
   U_TRACE(0, "UTransformCtx::registerDefault()")

   U_INTERNAL_ASSERT_POINTER(enabledTransforms)

   enabledTransforms->push(UString(UTranformBase64::_name));               // "base64"
   enabledTransforms->push(UString(UTranformBase64::_href));

   enabledTransforms->push(UString(UTranformInclC14N::_name));             // "c14n"
   enabledTransforms->push(UString(UTranformInclC14N::_href));

   enabledTransforms->push(UString(UTranformXPointer::_name));             // "xpointer"
   enabledTransforms->push(UString(UTranformXPointer::_href));

   /*
   enabledTransforms->push(UString(UTranformEnveloped::_name));            // "enveloped-signature"
   enabledTransforms->push(UString(UTranformInclC14NWithComment::_name));  // "c14n-with-comments"
   enabledTransforms->push(UString(UTranformInclC14N11::_name));           // "c14n11"
   enabledTransforms->push(UString(UTranformInclC14N11WithComment::_name));// "c14n11-with-comments"
   enabledTransforms->push(UString(UTranformExclC14N::_name));             // "exc-c14n"
   enabledTransforms->push(UString(UTranformExclC14NWithComment::_name));  // "exc-c14n-with-comments"
   enabledTransforms->push(UString(UTranformXPath::_name));                // "xpath"
   enabledTransforms->push(UString(UTranformXPath2::_name));               // "xpath2"
   enabledTransforms->push(UString(UTranformXslt::_name));                 // "xslt"
   */

   enabledTransforms->push(UString(UTranformSha1::_name));                 // "sha1"
   enabledTransforms->push(UString(UTranformSha1::_href));  

   enabledTransforms->push(UString(UTranformRsaMd5::_name));               // "rsa-md5"
   enabledTransforms->push(UString(UTranformRsaMd5::_href));

   enabledTransforms->push(UString(UTranformRsaSha1::_name));              // "rsa-sha1"
   enabledTransforms->push(UString(UTranformRsaSha1::_href));

   /*
   enabledTransforms->push(UString(UTranform::_name));                     // "aes128-cbc"
   enabledTransforms->push(UString(UTranform::_name));                     // "aes192-cbc"
   enabledTransforms->push(UString(UTranform::_name));                     // "aes256-cbc"
   enabledTransforms->push(UString(UTranform::_name));                     // "kw-aes128"
   enabledTransforms->push(UString(UTranform::_name));                     // "kw-aes192"
   enabledTransforms->push(UString(UTranform::_name));                     // "kw-aes256"
   enabledTransforms->push(UString(UTranform::_name));                     // "tripledes-cbc"
   enabledTransforms->push(UString(UTranform::_name));                     // "kw-tripledes"
   enabledTransforms->push(UString(UTranform::_name));                     // "dsa-sha1"
   enabledTransforms->push(UString(UTranform::_name));                     // "hmac-md5"
   enabledTransforms->push(UString(UTranform::_name));                     // "hmac-ripemd160"
   enabledTransforms->push(UString(UTranform::_name));                     // "hmac-sha1"
   enabledTransforms->push(UString(UTranform::_name));                     // "hmac-sha224"
   enabledTransforms->push(UString(UTranform::_name));                     // "hmac-sha256"
   enabledTransforms->push(UString(UTranform::_name));                     // "hmac-sha384"
   enabledTransforms->push(UString(UTranform::_name));                     // "hmac-sha512"
   enabledTransforms->push(UString(UTranform::_name));                     // "md5"
   enabledTransforms->push(UString(UTranform::_name));                     // "ripemd160"
   enabledTransforms->push(UString(UTranform::_name));                     // "rsa-ripemd160"
   enabledTransforms->push(UString(UTranform::_name));                     // "rsa-sha224"
   enabledTransforms->push(UString(UTranform::_name));                     // "rsa-sha256"
   enabledTransforms->push(UString(UTranform::_name));                     // "rsa-sha384"
   enabledTransforms->push(UString(UTranform::_name));                     // "rsa-sha512"
   enabledTransforms->push(UString(UTranform::_name));                     // "rsa-1_5"
   enabledTransforms->push(UString(UTranform::_name));                     // "rsa-oaep-mgf1p"
   enabledTransforms->push(UString(UTranform::_name));                     // "sha224"
   enabledTransforms->push(UString(UTranform::_name));                     // "sha256"
   enabledTransforms->push(UString(UTranform::_name));                     // "sha384"
   enabledTransforms->push(UString(UTranform::_name));                     // "sha512"
   */
}

UBaseTransform* UTransformCtx::findByHref(const char* href)
{
   U_TRACE(0, "UTransformCtx::findByHref(%S)", href)

   /* check with enabled transforms list */

   uint32_t i = enabledTransforms->find(href, u_strlen(href));

   if (i == U_NOT_FOUND) U_RETURN_POINTER(0, UBaseTransform);

   UBaseTransform* ptr;

   switch (i)
      {
      case 1: // "base64"
         {
         ptr = U_NEW(UTranformBase64());
         }
      break;

      case 3: // "c14n"
         {
         ptr = U_NEW(UTranformInclC14N());
         }
      break;

      case 5: // "xpointer"
         {
         ptr = U_NEW(UTranformXPointer());
         }
      break;

      case 7: // "sha1"
         {
         ptr = U_NEW(UTranformSha1());
         }
      break;

      case 9: // "rsa-md5"
         {
         ptr = U_NEW(UTranformRsaMd5());
         }
      break;

      case 11: // "rsa-sha1"
         {
         ptr = U_NEW(UTranformRsaSha1());
         }
      break;

      default:
         {
         ptr = 0;
         }
      break;
      }

   U_INTERNAL_ASSERT_POINTER(ptr)
   U_INTERNAL_ASSERT_EQUALS(strcmp(ptr->href(), href), 0)

   U_RETURN_POINTER(ptr, UBaseTransform);
}

/**
 * @node:  the pointer to the transform's node.
 * @usage: the transform usage (signature, encryption, ...).
 *
 * Reads transform from the @node as follows:
 *
 *    1) reads "Algorithm" attribute;
 *
 *    2) checks the lists of known and allowed transforms;
 *
 *    3) calls transform's read transform node method.
 *
 * Returns: pointer to newly created transform or NULL if an error occurs.
 */

UBaseTransform* UTransformCtx::nodeRead(xmlNodePtr node, int usage)
{
   U_TRACE(0, "UTransformCtx::nodeRead(%p,%d)", node, usage)

   U_INTERNAL_ASSERT_POINTER(node)
   U_INTERNAL_ASSERT_POINTER(UDSIGContext::pthis)
   U_INTERNAL_ASSERT_EQUALS(UDSIGContext::pthis->status, UReferenceCtx::UNKNOWN)

   const char* href = UXML2Node::getProp(node, "Algorithm");

   if (href)
      {
      UBaseTransform* id = findByHref(href);

      if (id)
         {
         if ((id->usage() & usage) != 0 &&
              id->readNode(node))
            {
            id->hereNode = node;

            U_RETURN_POINTER(id, UBaseTransform);
            }

         delete id;
         }
      }

   U_RETURN_POINTER(0, UBaseTransform);
}

// Reads transforms from the <dsig:Transform/> children of the @node and 
// appends them to the current transforms chain in ctx object.

bool UTransformCtx::nodesListRead(xmlNodePtr node, int usage)
{
   U_TRACE(0, "UTransformCtx::nodesListRead(%p,%d)", node, usage)

   xmlNodePtr cur = UXML2Node::getNextSibling(node->children);

   if (UXML2Node::checkNodeName(cur, (const xmlChar*)"Transforms",
                                     (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      UBaseTransform* id = nodeRead(cur, usage);

      if (id == 0) U_RETURN(false);

      chain.push(id);

      cur = UXML2Node::getNextSibling(cur->next);
      }

   if (cur) U_RETURN(false);

   U_RETURN(true);
}

UDSIGContext::UDSIGContext()
{
   U_TRACE_REGISTER_OBJECT(0, UDSIGContext, "", 0)

   U_INTERNAL_ASSERT_EQUALS(pthis, 0)

   id                   = 0;
   pthis                = this;
   status               = UReferenceCtx::UNKNOWN;  // the <dsig:Signature/> processing status.
   operation            = 0;                       // the operation: sign or verify.
   signMethod           = 0;                       // the pointer to signature transform.
   c14nMethod           = 0;                       // the pointer to c14n transform.
   keyInfoNode          = 0;                       // the pointer to <dsig:keyInfo/> node.
   signValueNode        = 0;                       // the pointer to <dsig:SignatureValue/> node.
   signedInfoNode       = 0;                       // the pointer to <dsig:signedInfo/> node.
   enabledReferenceUris = 0;                       // the URI types allowed for <dsig:Reference/> node.

   // Registers the default transforms compiled-in handlers.

   U_INTERNAL_ASSERT_EQUALS(UTransformCtx::enabledTransforms, 0)

   UTransformCtx::enabledTransforms = U_NEW(UVector<UString>);

   UTransformCtx::registerDefault();

   // Registers the default compiled-in I/O handlers.

   U_INTERNAL_ASSERT_EQUALS(UTranformInputURI::allIOCallbacks, 0)

   UTranformInputURI::allIOCallbacks = U_NEW(UVector<UIOCallback*>);

#ifdef LIBXML_FTP_ENABLED       
   U_SYSCALL_VOID_NO_PARAM(xmlNanoFTPInit);

   UTranformInputURI::allIOCallbacks->push(U_NEW(UIOCallback(xmlIOFTPMatch, xmlIOFTPOpen, xmlIOFTPRead, xmlIOFTPClose)));
#endif

#ifdef LIBXML_HTTP_ENABLED
   U_SYSCALL_VOID_NO_PARAM(xmlNanoHTTPInit);

   UTranformInputURI::allIOCallbacks->push(U_NEW(UIOCallback(xmlIOHTTPMatch, xmlIOHTTPOpen, xmlIOHTTPRead, xmlIOHTTPClose)));
#endif

   UTranformInputURI::allIOCallbacks->push(U_NEW(UIOCallback(xmlFileMatch, xmlFileOpen, xmlFileRead, xmlFileClose)));
}

UDSIGContext::~UDSIGContext()
{
   U_TRACE_UNREGISTER_OBJECT(0, UDSIGContext)

   delete UTransformCtx::enabledTransforms;

#ifdef LIBXML_FTP_ENABLED       
   U_SYSCALL_VOID_NO_PARAM(xmlNanoFTPCleanup);
#endif
#ifdef LIBXML_HTTP_ENABLED
   U_SYSCALL_VOID_NO_PARAM(xmlNanoHTTPCleanup);
#endif

   delete UTranformInputURI::allIOCallbacks;
}

/**
 * The Manifest Element (http://www.w3.org/TR/xmldsig-core/#sec-Manifest)
 *
 * The Manifest element provides a list of References. The difference from 
 * the list in SignedInfo is that it is application defined which, if any, of 
 * the digests are actually checked against the objects referenced and what to 
 * do if the object is inaccessible or the digest compare fails. If a Manifest 
 * is pointed to from SignedInfo, the digest over the Manifest itself will be 
 * checked by the core result validation behavior. The digests within such 
 * a Manifest are checked at the application's discretion. If a Manifest is 
 * referenced from another Manifest, even the overall digest of this two level 
 * deep Manifest might not be checked.
 *     
 * Schema Definition:
 *     
 * <element name="Manifest" type="ds:ManifestType"/> 
 * <complexType name="ManifestType">
 *   <sequence>
 *     <element ref="ds:Reference" maxOccurs="unbounded"/> 
 *   </sequence> 
 *   <attribute name="Id" type="ID" use="optional"/> 
 *  </complexType>
 * 
 * DTD:
 *
 * <!ELEMENT Manifest (Reference+)  >
 * <!ATTLIST Manifest Id ID  #IMPLIED >
 */

bool UDSIGContext::processManifestNode(xmlNodePtr node)
{
   U_TRACE(0, "UDSIGContext::processManifestNode(%p)", node)

   UReferenceCtx* ref;

   xmlNodePtr cur = UXML2Node::getNextSibling(node->children);

   while (UXML2Node::checkNodeName(cur, (const xmlChar*)"Reference",
                                        (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      /* create reference */

      ref = U_NEW(UReferenceCtx(UReferenceCtx::MANIFEST));

      /* add to the list */

      manifestReferences.push(ref);

      /* process */

      if (ref->processNode(cur) == false) goto end;

      cur = UXML2Node::getNextSibling(cur->next);
      }

   // if there is something left than it's an error

   if (cur) goto end;

   U_RETURN(true);

end:
   U_RETURN(false);
}

/**
 * The Object Element (http://www.w3.org/TR/xmldsig-core/#sec-Object)
 * 
 * Object is an optional element that may occur one or more times. When 
 * present, this element may contain any data. The Object element may include 
 * optional MIME type, ID, and encoding attributes.
 *     
 * Schema Definition:
 *     
 * <element name="Object" type="ds:ObjectType"/> 
 * <complexType name="ObjectType" mixed="true">
 *   <sequence minOccurs="0" maxOccurs="unbounded">
 *     <any namespace="##any" processContents="lax"/>
 *   </sequence>
 *   <attribute name="Id" type="ID" use="optional"/> 
 *   <attribute name="MimeType" type="string" use="optional"/>
 *   <attribute name="Encoding" type="anyURI" use="optional"/> 
 * </complexType>
 * 
 * DTD:
 * 
 * <!ELEMENT Object (#PCDATA|Signature|SignatureProperties|Manifest %Object.ANY;)* >
 * <!ATTLIST Object  Id  ID  #IMPLIED 
 *                   MimeType    CDATA   #IMPLIED 
 *                   Encoding    CDATA   #IMPLIED >
 */

bool UDSIGContext::processObjectNode(xmlNodePtr node)
{
   U_TRACE(0, "UDSIGContext::processObjectNode(%p)", node)

   /* we care about Manifest nodes only; ignore everything else */

   xmlNodePtr cur = UXML2Node::getNextSibling(node->children);

   while (UXML2Node::checkNodeName(cur, (const xmlChar*)"Manifest",
                                        (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      if (processManifestNode(cur) == false) goto end;

      cur = UXML2Node::getNextSibling(cur->next);
      }

   U_RETURN(true);

end:
   U_RETURN(false);
}

bool UDSIGContext::processKeyInfoNode()
{
   U_TRACE(0, "UDSIGContext::processKeyInfoNode()")

   U_INTERNAL_ASSERT_POINTER(signMethod)

   /* set key requirements

   ret = xmlSecTransformSetKeyReq(dsigCtx->signMethod, &(dsigCtx->keyInfoReadCtx.keyReq));

   // ignore <dsig:KeyInfo /> if there is the key is already set
   // TODO: throw an error if key is set and node != NULL?

   if ((dsigCtx->signKey == NULL) &&
       (dsigCtx->keyInfoReadCtx.keysMngr != NULL) &&
       (dsigCtx->keyInfoReadCtx.keysMngr->getKey != NULL))
      {  
      dsigCtx->signKey = (dsigCtx->keyInfoReadCtx.keysMngr->getKey)(node, &(dsigCtx->keyInfoReadCtx));
      }

   // check that we have exactly what we want

   if ((dsigCtx->signKey == NULL) ||
       (!xmlSecKeyMatch(dsigCtx->signKey, NULL, &(dsigCtx->keyInfoReadCtx.keyReq))))
      {
      goto end;
      }

   // set the key to the transform

   ret = xmlSecTransformSetKey(dsigCtx->signMethod, dsigCtx->signKey);

   // if we are signing document, update <dsig:KeyInfo/> node

   if (node &&
       operation == UBaseTransform::SIGN)
      {  
      ret = xmlSecKeyInfoNodeWrite(node, dsigCtx->signKey, &(dsigCtx->keyInfoWriteCtx));
      }
   */

   U_RETURN(true);

end:
   U_RETURN(false);
}

/**
 * The Signature element (http://www.w3.org/TR/xmldsig-core/#sec-Signature)
 *
 * The Signature element is the root element of an XML Signature. 
 * Implementation MUST generate laxly schema valid [XML-schema] Signature 
 * elements as specified by the following schema:
 * The way in which the SignedInfo element is presented to the 
 * canonicalization method is dependent on that method. The following 
 * applies to algorithms which process XML as nodes or characters:
 *
 *  - XML based canonicalization implementations MUST be provided with 
 *  a [XPath] node-set originally formed from the document containing 
 *  the SignedInfo and currently indicating the SignedInfo, its descendants,
 *  and the attribute and namespace nodes of SignedInfo and its descendant 
 *  elements.
 *
 *  - Text based canonicalization algorithms (such as CRLF and charset 
 *  normalization) should be provided with the UTF-8 octets that represent 
 *  the well-formed SignedInfo element, from the first character to the 
 *  last character of the XML representation, inclusive. This includes 
 *  the entire text of the start and end tags of the SignedInfo element 
 *  as well as all descendant markup and character data (i.e., the text) 
 *  between those tags. Use of text based canonicalization of SignedInfo 
 *  is NOT RECOMMENDED.         
 *
 *  =================================
 *  we do not support any non XML based C14N 
 *
 * Schema Definition:
 *
 *  <element name="Signature" type="ds:SignatureType"/>
 *  <complexType name="SignatureType">
 *  <sequence> 
 *     <element ref="ds:SignedInfo"/> 
 *     <element ref="ds:SignatureValue"/> 
 *     <element ref="ds:KeyInfo" minOccurs="0"/> 
 *     <element ref="ds:Object" minOccurs="0" maxOccurs="unbounded"/> 
 *     </sequence> <attribute name="Id" type="ID" use="optional"/>
 *  </complexType>
 *    
 * DTD:
 *    
 *  <!ELEMENT Signature (SignedInfo, SignatureValue, KeyInfo?, Object*)  >
 *  <!ATTLIST Signature  
 *      xmlns   CDATA   #FIXED 'http://www.w3.org/2000/09/xmldsig#'
 *      Id      ID  #IMPLIED >
 *
 */

bool UDSIGContext::processSignatureNode(xmlNodePtr signature)
{
   U_TRACE(0, "UDSIGContext::processSignatureNode(%p)", signature)

   U_INTERNAL_ASSERT_POINTER(signature)
   U_INTERNAL_ASSERT_EQUALS(signMethod, 0)
   U_INTERNAL_ASSERT_EQUALS(c14nMethod, 0)
   U_INTERNAL_ASSERT_EQUALS(signValueNode, 0)
   U_INTERNAL_ASSERT_EQUALS(status, UReferenceCtx::UNKNOWN)
   U_INTERNAL_ASSERT(operation == UBaseTransform::VERIFY || operation == UBaseTransform::SIGN)

   xmlNodePtr cur;

   /* read node data */

   id = UXML2Node::getProp(signature, "Id");

   // first node is required SignedInfo

   signedInfoNode = UXML2Node::getNextSibling(signature->children);

   if (UXML2Node::checkNodeName(signedInfoNode, (const xmlChar*)"SignedInfo",
                                                (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#") == false) goto end;

   // next node is required SignatureValue

   signValueNode = UXML2Node::getNextSibling(signedInfoNode->next);

   if (UXML2Node::checkNodeName(signValueNode, (const xmlChar*)"SignatureValue",
                                               (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#") == false) goto end;

   // next node is optional KeyInfo

   keyInfoNode = UXML2Node::getNextSibling(signValueNode->next);

   if (UXML2Node::checkNodeName(keyInfoNode, (const xmlChar*)"KeyInfo",
                                             (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      cur = UXML2Node::getNextSibling(keyInfoNode->next);
      }
   else
      {
      cur = keyInfoNode;
      }

   // next nodes are optional Object nodes

   while (UXML2Node::checkNodeName(cur, (const xmlChar*)"Object",
                                        (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      /* read manifests from objects */

      if (processObjectNode(cur) == false) goto end;

      cur = UXML2Node::getNextSibling(cur->next);
      }

   // if there is something left than it's an error

   if (cur) goto end;

   // now validated all the references and prepare transform

   if (processSignedInfoNode() == false) goto end;

   /* references processing might change the status */

   if (status != UReferenceCtx::UNKNOWN) U_RETURN(true);

   /* as the result, we should have sign and c14n methods set */

   U_INTERNAL_ASSERT_POINTER(c14nMethod)
   U_INTERNAL_ASSERT_POINTER(signMethod)

   if (processKeyInfoNode() == false) goto end;

   /*
   // as the result, we should have a key

   U_INTERNAL_ASSERT_POINTER(signKey)

   // if we need to write result to xml node then we need base64 encode result

   if (operation == UBaseTransform::SIGN)
      {  
      xmlSecTransformPtr base64Encode;

      // we need to add base64 encode transform

      base64Encode = xmlSecTransformCtxCreateAndAppend(&(dsigCtx->transformCtx), xmlSecTransformBase64Id);

      base64Encode->operation = xmlSecTransformOperationEncode;
      }

   int firstType = xmlSecTransformGetDataType(dsigCtx->transformCtx.first, xmlSecTransformModePush, &(dsigCtx->transformCtx));

   if ((firstType & UTransformCtx::XML) != 0)
      {
      U_INTERNAL_ASSERT_POINTER(signedInfoNode)

      xmlSecNodeSetPtr nodeset = xmlSecNodeSetGetChildren(signedInfoNode->doc, signedInfoNode, 1, 0);

      // calculate the signature

      ret = xmlSecTransformCtxXmlExecute(&(dsigCtx->transformCtx), nodeset);

      xmlSecNodeSetDestroy(nodeset);
      }
   */

   U_RETURN(true);

end:
   U_RETURN(false);
}

/**
 * @transform: the pointer to transform.
 * @node:      the pointer to node.
 *
 * Gets the @node content, base64 decodes it and calls #xmlSecTransformVerify
 * function to verify binary results.
 *
 * Returns: true on success or false if an error occurs.
 */

bool UTransformCtx::verifyNodeContent(UBaseTransform* transform, xmlNodePtr node)
{
   U_TRACE(0, "UTransformCtx::verifyNodeContent(%p,%p)", transform, node)

   U_INTERNAL_ASSERT_POINTER(node)
   U_INTERNAL_ASSERT_POINTER(transform)

   const char* content = (const char*) UXML2Node::getContent(node);

   uint32_t size = u_strlen(content);

   transform->inBuf.reserve(size);

   if (UBase64::decode(content, size, transform->inBuf) == false) goto end;

   /*
    * Verifies the data with transform's processing results
    * (for digest, HMAC and signature transforms). The verification 
    * result is stored in the #status member of #xmlSecTransform object.
    */

   if (transform->verify() == false) goto end;

   U_RETURN(true);

end:
   U_RETURN(false);
}

/**
 * Executes transforms chain in ctx.
 *
 * Returns: true on success or false otherwise.
 */

bool UTransformCtx::execute()
{
   U_TRACE(0, "UTransformCtx::execute()")

   U_INTERNAL_ASSERT_EQUALS(status, 0)

   if (       uri &&
       strlen(uri))
      {
      UBaseTransform* uriTransform = U_NEW(UTranformInputURI(uri));

      chain.insert(0, uriTransform);

      // Process binary data from the URI using transforms chain in ctx

      /* 
      // we do not need to do something special for this transform

      ret = xmlSecTransformCtxPrepare(ctx, xmlSecTransformDataTypeUnknown);

      // Now we have a choice: we either can push from first transform or pop 
      // from last. Our C14N transforms prefers push, so push data!

      ret = xmlSecTransformPump(uriTransform, uriTransform->next, ctx);     

      status = xmlSecTransformStatusFinished;
      */
      }
   else
      {
      /*
      xmlSecNodeSetPtr nodes;

      if (       xptrExpr &&
          strlen(xptrExpr))
         {
         // our xpointer transform takes care of providing correct nodes set

         nodes = xmlSecNodeSetCreate(doc, NULL, xmlSecNodeSetNormal);
         }
      else
         {
         // we do not want to have comments for empty URI

         nodes = xmlSecNodeSetGetChildren(doc, NULL, 0, 0);
         }

      ret = xmlSecTransformCtxXmlExecute(ctx, nodes);

      // TODO: don't destroy nodes here

      xmlSecNodeSetDestroy(nodes);
      */
      }

   U_RETURN(true);

end:
   U_RETURN(false);
}

/**
 * Validates signature in the document.
 *
 * The verification result is returned in #status member of the object.
 *
 * Returns: true on success
 */

bool UDSIGContext::verify(UXML2Document& document)
{
   U_TRACE(0, "UDSIGContext::verify(%p)", &document)

   operation = UBaseTransform::VERIFY;

   // find signature node (the Signature element is the root element of an XML Signature)

   xmlNodePtr signature = document.findNode(document.getRootNode(),
                                            (const xmlChar*)"Signature",
                                            (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#");

   /* read signature info */

   if (signature == 0 ||
       processSignatureNode(signature) == false) goto end;

   U_INTERNAL_ASSERT_POINTER(signMethod)
   U_INTERNAL_ASSERT_POINTER(signValueNode)

   /* references processing might change the status */

   if (status != UReferenceCtx::UNKNOWN) U_RETURN(true);

   /* verify SignatureValue node content */

   if (transformCtx.verifyNodeContent(signMethod, signValueNode) == false) goto end;

   /* set status and we are done */

   status = (signMethod->status == UBaseTransform::OK ? UReferenceCtx::SUCCEEDED : UReferenceCtx::INVALID);

   U_RETURN(true);

end:
   U_RETURN(false);
}

/**
 * @node: the pointer to <dsig:Reference/> node.

 * The Reference Element (http://www.w3.org/TR/xmldsig-core/#sec-Reference)
 * 
 * Reference is an element that may occur one or more times. It specifies 
 * a digest algorithm and digest value, and optionally an identifier of the 
 * object being signed, the type of the object, and/or a list of transforms 
 * to be applied prior to digesting. The identification (URI) and transforms 
 * describe how the digested content (i.e., the input to the digest method) 
 * was created. The Type attribute facilitates the processing of referenced 
 * data. For example, while this specification makes no requirements over 
 * external data, an application may wish to signal that the referent is a 
 * Manifest. An optional ID attribute permits a Reference to be referenced 
 * from elsewhere.
 *
 * Returns: true on succes or false value otherwise.
 */

bool UReferenceCtx::processNode(xmlNodePtr node)
{
   U_TRACE(0, "UReferenceCtx::processNode(%p)", node)

   U_INTERNAL_ASSERT_POINTER(node)
   U_INTERNAL_ASSERT_EQUALS(digestMethod, 0)

   xmlNodePtr cur, digestValueNode;

   // read attributes first

   id   = UXML2Node::getProp(node, "Id");
   uri  = UXML2Node::getProp(node, "URI");
   type = UXML2Node::getProp(node, "Type");

   /* set start URI (and check that it is enabled!) */

   if (transformCtx.setURI(uri, node) == false) goto end;

   // first is optional Transforms node

   cur = UXML2Node::getNextSibling(node->children);

   if (UXML2Node::checkNodeName(cur, (const xmlChar*)"Transforms",
                                     (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      // Reads transforms from the <dsig:Transform/> children of the @node and 
      // appends them to the current transforms chain in @ctx object.

      if (transformCtx.nodesListRead(cur, UBaseTransform::DSIG) == false) goto end;

      cur = UXML2Node::getNextSibling(cur->next);
      }

   // next node is required DigestMethod

   if (UXML2Node::checkNodeName(cur, (const xmlChar*)"DigestMethod",
                                     (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      digestMethod = UTransformCtx::nodeRead(cur, UBaseTransform::DIGEST);

      cur = UXML2Node::getNextSibling(cur->next);
      }

   if (digestMethod == 0) goto end;

   digestMethod->operation = UDSIGContext::pthis->operation;

   // last node is required DigestValue

   if (UXML2Node::checkNodeName(cur, (const xmlChar*)"DigestValue",
                                     (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      digestValueNode = cur;

      cur = UXML2Node::getNextSibling(cur->next);
      }

   // if there is something left than it's an error

   if (cur) goto end;

   // if we need to write result to xml node then we need base64 encode result

   if (UDSIGContext::pthis->operation == UBaseTransform::SIGN)
      {
      /* we need to add base64 encode transform

      xmlSecTransformPtr base64Encode = xmlSecTransformCtxCreateAndAppend(transformCtx, xmlSecTransformBase64Id);

      base64Encode->operation = xmlSecTransformOperationEncode;
      */
      }

   // finally get transforms results

   if (transformCtx.execute() == 0) goto end;

   /*
   dsigRefCtx->result = transformCtx->result;

   if (UDSIGContext::pthis->operation == UBaseTransform::SIGN)
      {
      if (digest_result.empty() ||
          xmlSecBufferGetData(digest_result) == false) goto end;

      // write signed data to xml

      xmlNodeSetContentLen(digestValueNode, xmlSecBufferGetData(dsigRefCtx->result), xmlSecBufferGetSize(dsigRefCtx->result));

      // set success status and we are done

      status = SUCCEEDED;
      }
   else
      {
      // verify SignatureValue node content

      ret = xmlSecTransformVerifyNodeContent(dsigRefCtx->digestMethod, digestValueNode, transformCtx);

      // set status and we are done

      status = (digestMethod->status == UBaseTransform::OK ? SUCCEEDED : INVALID);
      }
   */

   status = SUCCEEDED;

   U_RETURN(true);

end:
   U_RETURN(false);
}

/** 
 * The SignedInfo Element (http://www.w3.org/TR/xmldsig-core/#sec-SignedInfo)
 * 
 * The structure of SignedInfo includes the canonicalization algorithm, 
 * a result algorithm, and one or more references. The SignedInfo element 
 * may contain an optional ID attribute that will allow it to be referenced by 
 * other signatures and objects.
 *
 * SignedInfo does not include explicit result or digest properties (such as
 * calculation time, cryptographic device serial number, etc.). If an 
 * application needs to associate properties with the result or digest, 
 * it may include such information in a SignatureProperties element within 
 * an Object element.
 *
 * Schema Definition:
 *
 *  <element name="SignedInfo" type="ds:SignedInfoType"/> 
 *  <complexType name="SignedInfoType">
 *    <sequence> 
 *      <element ref="ds:CanonicalizationMethod"/>
 *      <element ref="ds:SignatureMethod"/> 
 *      <element ref="ds:Reference" maxOccurs="unbounded"/> 
 *    </sequence> 
 *    <attribute name="Id" type="ID" use="optional"/> 
 *  </complexType>
 *    
 * DTD:
 *    
 *  <!ELEMENT SignedInfo (CanonicalizationMethod, SignatureMethod,  Reference+) >
 *  <!ATTLIST SignedInfo  Id   ID      #IMPLIED>
 * 
 */

bool UDSIGContext::processSignedInfoNode()
{
   U_TRACE(0, "UDSIGContext::processSignedInfoNode()")

   U_INTERNAL_ASSERT_EQUALS(signMethod, 0)
   U_INTERNAL_ASSERT_EQUALS(c14nMethod, 0)
   U_ASSERT_EQUALS(pthis->signedInfoReferences.size(), 0)
   U_INTERNAL_ASSERT_EQUALS(status, UReferenceCtx::UNKNOWN)
   U_INTERNAL_ASSERT(operation == UBaseTransform::VERIFY || operation == UBaseTransform::SIGN)

   // first node is required CanonicalizationMethod

   xmlNodePtr cur = UXML2Node::getNextSibling(signedInfoNode->children);

   if (UXML2Node::checkNodeName(cur, (const xmlChar*)"CanonicalizationMethod",
                                     (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#") == false) goto end;

   c14nMethod = UTransformCtx::nodeRead(cur, UBaseTransform::C14N);

   if (c14nMethod == 0) goto end;

   transformCtx.chain.push(c14nMethod);

   // next node is required SignatureMethod

   cur = UXML2Node::getNextSibling(cur->next);

   if (UXML2Node::checkNodeName(cur, (const xmlChar*)"SignatureMethod",
                                     (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#") == false) goto end;

   signMethod = UTransformCtx::nodeRead(cur, UBaseTransform::SIGNATURE);

   if (signMethod == 0) goto end;

   transformCtx.chain.push(signMethod);

   signMethod->operation = UDSIGContext::pthis->operation;

   // calculate references

   UReferenceCtx* ref;

   cur = UXML2Node::getNextSibling(cur->next);

   while (UXML2Node::checkNodeName(cur, (const xmlChar*)"Reference",
                                        (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      /* create reference */

      ref = U_NEW(UReferenceCtx(UReferenceCtx::SIGNED_INFO));

      /* add to the list */

      signedInfoReferences.push(ref);

      /* process */

      if (ref->processNode(cur) == false) goto end;

      /* bail out if next Reference processing failed */

      if (ref->status != UReferenceCtx::SUCCEEDED)
         {
         status = UReferenceCtx::INVALID;

         goto end;
         }

      cur = UXML2Node::getNextSibling(cur->next);
      }

   /* check that we have at least one Reference */

   if (signedInfoReferences.empty()) goto end;

   // if there is something left than it's an error

   if (cur) goto end;

   U_RETURN(true);

end:
   U_RETURN(false);
}

/* Parses uri and adds xpointer transforms if required.
 *
 * The following examples demonstrate what the URI attribute identifies
 * and how it is dereferenced 
 *
 * (http://www.w3.org/TR/xmldsig-core/#sec-ReferenceProcessingModel):
 *
 * - URI="http://example.com/bar.xml"
 *
 * identifies the octets that represent the external resource 
 * 'http://example.com/bar.xml', that is probably an XML document given 
 * its file extension.
 *
 * - URI="http://example.com/bar.xml#chapter1"
 *
 * identifies the element with ID attribute value 'chapter1' of the 
 * external XML resource 'http://example.com/bar.xml', provided as an 
 * octet stream. Again, for the sake of interoperability, the element 
 * identified as 'chapter1' should be obtained using an XPath transform 
 * rather than a URI fragment (barename XPointer resolution in external 
 * resources is not REQUIRED in this specification). 
 *
 * - URI=""
 *
 * identifies the node-set (minus any comment nodes) of the XML resource 
 * containing the signature
 *
 * - URI="#chapter1"
 *
 * identifies a node-set containing the element with ID attribute value 
 * 'chapter1' of the XML resource containing the signature. XML Signature 
 * (and its applications) modify this node-set to include the element plus 
 * all descendents including namespaces and attributes -- but not comments.
 */

bool UTransformCtx::setURI(const char* uri, xmlNodePtr node)
{
   U_TRACE(0, "UTransformCtx::setURI(%S,%p)", uri, node)

   /* check uri */

   int uriType = 0;

   if (       uri  == 0 ||
       strlen(uri) == 0)
      {
      uriType = EMPTY;
      }
   else if (uri[0] == '#')
      {
      uriType = SAME_DOCUMENT;
      }
   else if (U_MEMCMP(uri, "file://") == 0)
      {
      uriType = TYPE_LOCAL;
      }
   else
      {
      uriType = TYPE_REMOTE;
      }

   if ((uriType & enabledUris) == 0) U_RETURN(false);

   /* is it an empty uri? */

   if (uriType == EMPTY) U_RETURN(true);

   /* do we have barename or full xpointer? */

   const char* xptr = strchr(uri, '#');

   if (xptr == NULL) U_RETURN(true);

   if (U_MEMCMP(uri, "#xpointer(/)") == 0)
      {
      xptrExpr = uri;

      U_RETURN(true);
      }

   xptrExpr  = strdup(xptr);
   this->uri = strndup(uri, xptr - uri);

   /* do we have barename or full xpointer? */

   int nodeSetType = UNodeSet::TREE;

   if (U_STRNEQ(xptr, "#xmlns(") ||
       U_STRNEQ(xptr, "#xpointer("))
      {
      ++xptr;
      }
   else
      {
      /* we need to add "xpointer(id('..')) because otherwise we have problems with numeric ("111" and so on) and other "strange" ids */

      static char buf[128];

      (void) u_snprintf(buf, sizeof(buf), "xpointer(id(\'%s\'))", xptr + 1);

      xptr = buf;

      nodeSetType = UNodeSet::TREE_WITHOUT_COMMENTS;
      }

   U_INTERNAL_DUMP("this->uri = %S xptr = %S", this->uri, xptr)

   // we need to create XPointer transform to execute expr

   UTranformXPointer* transform = U_NEW(UTranformXPointer());

   if (transform->setExpr(xptr, nodeSetType, node))
      {
      chain.insert(0, transform);

      U_RETURN(true);
      }

   U_RETURN(false);
}

// DEBUG

#ifdef DEBUG
#  include <ulib/internal/objectIO.h>

const char* UTransformCtx::dump(bool reset) const
{
   *UObjectIO::os << "status      " << status << '\n'
                  << "enabledUris " << enabledUris;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UReferenceCtx::dump(bool reset) const
{
   *UObjectIO::os << "status    " << status << '\n'
                  << "origin    " << origin;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UDSIGContext::dump(bool reset) const
{
   *UObjectIO::os << "status    " << status << '\n'
                  << "operation " << operation;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

#endif
