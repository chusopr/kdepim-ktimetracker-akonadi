/* soapClient.cpp
   Generated by gSOAP 2.7.0d from /build/kde/cvs/pim/kdepim/kresources/groupwise/soap/groupwise.h
   Copyright (C) 2001-2004 Genivia, Inc. All Rights Reserved.
   This software is released under one of the following three licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
   See README.txt for further details.
*/
#include "soapH.h"

SOAP_BEGIN_NAMESPACE(soap)

SOAP_SOURCE_STAMP("@(#) soapClient.cpp ver 2.7.0d 2004-12-03 16:33:23 GMT")


SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__loginRequest(struct soap *soap, const char *URL, const char *action, _ns1__loginRequest *ns1__loginRequest, _ns1__loginResponse *ns1__loginResponse)
{
	struct __ns1__loginRequest soap_tmp___ns1__loginRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "loginRequest";
	soap_tmp___ns1__loginRequest.ns1__loginRequest=ns1__loginRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__loginRequest(soap, &soap_tmp___ns1__loginRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__loginRequest(soap, &soap_tmp___ns1__loginRequest, "-ns1:loginRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__loginRequest(soap, &soap_tmp___ns1__loginRequest, "-ns1:loginRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__loginResponse)
		return soap_closesock(soap);
	ns1__loginResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__loginResponse->soap_get(soap, "ns1:loginResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__getCategoryListRequest(struct soap *soap, const char *URL, const char *action, std::string ns1__getCategoryListRequest, _ns1__getCategoryListResponse *ns1__getCategoryListResponse)
{
	struct __ns1__getCategoryListRequest soap_tmp___ns1__getCategoryListRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "getCategoryListRequest";
	soap_tmp___ns1__getCategoryListRequest.ns1__getCategoryListRequest=ns1__getCategoryListRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__getCategoryListRequest(soap, &soap_tmp___ns1__getCategoryListRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__getCategoryListRequest(soap, &soap_tmp___ns1__getCategoryListRequest, "-ns1:getCategoryListRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__getCategoryListRequest(soap, &soap_tmp___ns1__getCategoryListRequest, "-ns1:getCategoryListRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__getCategoryListResponse)
		return soap_closesock(soap);
	ns1__getCategoryListResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__getCategoryListResponse->soap_get(soap, "ns1:getCategoryListResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__getAddressBookListRequest(struct soap *soap, const char *URL, const char *action, std::string ns1__getAddressBookListRequest, _ns1__getAddressBookListResponse *ns1__getAddressBookListResponse)
{
	struct __ns1__getAddressBookListRequest soap_tmp___ns1__getAddressBookListRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "getAddressBookListRequest";
	soap_tmp___ns1__getAddressBookListRequest.ns1__getAddressBookListRequest=ns1__getAddressBookListRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__getAddressBookListRequest(soap, &soap_tmp___ns1__getAddressBookListRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__getAddressBookListRequest(soap, &soap_tmp___ns1__getAddressBookListRequest, "-ns1:getAddressBookListRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__getAddressBookListRequest(soap, &soap_tmp___ns1__getAddressBookListRequest, "-ns1:getAddressBookListRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__getAddressBookListResponse)
		return soap_closesock(soap);
	ns1__getAddressBookListResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__getAddressBookListResponse->soap_get(soap, "ns1:getAddressBookListResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__getItemRequest(struct soap *soap, const char *URL, const char *action, _ns1__getItemRequest *ns1__getItemRequest, _ns1__getItemResponse *ns1__getItemResponse)
{
	struct __ns1__getItemRequest soap_tmp___ns1__getItemRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "getItemRequest";
	soap_tmp___ns1__getItemRequest.ns1__getItemRequest=ns1__getItemRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__getItemRequest(soap, &soap_tmp___ns1__getItemRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__getItemRequest(soap, &soap_tmp___ns1__getItemRequest, "-ns1:getItemRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__getItemRequest(soap, &soap_tmp___ns1__getItemRequest, "-ns1:getItemRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__getItemResponse)
		return soap_closesock(soap);
	ns1__getItemResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__getItemResponse->soap_get(soap, "ns1:getItemResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__getItemsRequest(struct soap *soap, const char *URL, const char *action, _ns1__getItemsRequest *ns1__getItemsRequest, _ns1__getItemsResponse *ns1__getItemsResponse)
{
	struct __ns1__getItemsRequest soap_tmp___ns1__getItemsRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "getItemsRequest";
	soap_tmp___ns1__getItemsRequest.ns1__getItemsRequest=ns1__getItemsRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__getItemsRequest(soap, &soap_tmp___ns1__getItemsRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__getItemsRequest(soap, &soap_tmp___ns1__getItemsRequest, "-ns1:getItemsRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__getItemsRequest(soap, &soap_tmp___ns1__getItemsRequest, "-ns1:getItemsRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__getItemsResponse)
		return soap_closesock(soap);
	ns1__getItemsResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__getItemsResponse->soap_get(soap, "ns1:getItemsResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__getFolderListRequest(struct soap *soap, const char *URL, const char *action, _ns1__getFolderListRequest *ns1__getFolderListRequest, _ns1__getFolderListResponse *ns1__getFolderListResponse)
{
	struct __ns1__getFolderListRequest soap_tmp___ns1__getFolderListRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "getFolderListRequest";
	soap_tmp___ns1__getFolderListRequest.ns1__getFolderListRequest=ns1__getFolderListRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__getFolderListRequest(soap, &soap_tmp___ns1__getFolderListRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__getFolderListRequest(soap, &soap_tmp___ns1__getFolderListRequest, "-ns1:getFolderListRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__getFolderListRequest(soap, &soap_tmp___ns1__getFolderListRequest, "-ns1:getFolderListRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__getFolderListResponse)
		return soap_closesock(soap);
	ns1__getFolderListResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__getFolderListResponse->soap_get(soap, "ns1:getFolderListResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__getDeltaRequest(struct soap *soap, const char *URL, const char *action, _ns1__getDeltaRequest *ns1__getDeltaRequest, _ns1__getDeltaResponse *ns1__getDeltaResponse)
{
	struct __ns1__getDeltaRequest soap_tmp___ns1__getDeltaRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "getDeltaRequest";
	soap_tmp___ns1__getDeltaRequest.ns1__getDeltaRequest=ns1__getDeltaRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__getDeltaRequest(soap, &soap_tmp___ns1__getDeltaRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__getDeltaRequest(soap, &soap_tmp___ns1__getDeltaRequest, "-ns1:getDeltaRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__getDeltaRequest(soap, &soap_tmp___ns1__getDeltaRequest, "-ns1:getDeltaRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__getDeltaResponse)
		return soap_closesock(soap);
	ns1__getDeltaResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__getDeltaResponse->soap_get(soap, "ns1:getDeltaResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__createItemRequest(struct soap *soap, const char *URL, const char *action, _ns1__createItemRequest *ns1__createItemRequest, _ns1__createItemResponse *ns1__createItemResponse)
{
	struct __ns1__createItemRequest soap_tmp___ns1__createItemRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "createItemRequest";
	soap_tmp___ns1__createItemRequest.ns1__createItemRequest=ns1__createItemRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__createItemRequest(soap, &soap_tmp___ns1__createItemRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__createItemRequest(soap, &soap_tmp___ns1__createItemRequest, "-ns1:createItemRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__createItemRequest(soap, &soap_tmp___ns1__createItemRequest, "-ns1:createItemRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__createItemResponse)
		return soap_closesock(soap);
	ns1__createItemResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__createItemResponse->soap_get(soap, "ns1:createItemResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__sendItemRequest(struct soap *soap, const char *URL, const char *action, _ns1__sendItemRequest *ns1__sendItemRequest, _ns1__sendItemResponse *ns1__sendItemResponse)
{
	struct __ns1__sendItemRequest soap_tmp___ns1__sendItemRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "sendItemRequest";
	soap_tmp___ns1__sendItemRequest.ns1__sendItemRequest=ns1__sendItemRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__sendItemRequest(soap, &soap_tmp___ns1__sendItemRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__sendItemRequest(soap, &soap_tmp___ns1__sendItemRequest, "-ns1:sendItemRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__sendItemRequest(soap, &soap_tmp___ns1__sendItemRequest, "-ns1:sendItemRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__sendItemResponse)
		return soap_closesock(soap);
	ns1__sendItemResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__sendItemResponse->soap_get(soap, "ns1:sendItemResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__modifyItemRequest(struct soap *soap, const char *URL, const char *action, _ns1__modifyItemRequest *ns1__modifyItemRequest, _ns1__modifyItemResponse *ns1__modifyItemResponse)
{
	struct __ns1__modifyItemRequest soap_tmp___ns1__modifyItemRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "modifyItemRequest";
	soap_tmp___ns1__modifyItemRequest.ns1__modifyItemRequest=ns1__modifyItemRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__modifyItemRequest(soap, &soap_tmp___ns1__modifyItemRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__modifyItemRequest(soap, &soap_tmp___ns1__modifyItemRequest, "-ns1:modifyItemRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__modifyItemRequest(soap, &soap_tmp___ns1__modifyItemRequest, "-ns1:modifyItemRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__modifyItemResponse)
		return soap_closesock(soap);
	ns1__modifyItemResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__modifyItemResponse->soap_get(soap, "ns1:modifyItemResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__purgeItemRequest(struct soap *soap, const char *URL, const char *action, _ns1__purgeItemRequest *ns1__purgeItemRequest, _ns1__purgeItemResponse *ns1__purgeItemResponse)
{
	struct __ns1__purgeItemRequest soap_tmp___ns1__purgeItemRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "purgeItemRequest";
	soap_tmp___ns1__purgeItemRequest.ns1__purgeItemRequest=ns1__purgeItemRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__purgeItemRequest(soap, &soap_tmp___ns1__purgeItemRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__purgeItemRequest(soap, &soap_tmp___ns1__purgeItemRequest, "-ns1:purgeItemRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__purgeItemRequest(soap, &soap_tmp___ns1__purgeItemRequest, "-ns1:purgeItemRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__purgeItemResponse)
		return soap_closesock(soap);
	ns1__purgeItemResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__purgeItemResponse->soap_get(soap, "ns1:purgeItemResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__removeItemRequest(struct soap *soap, const char *URL, const char *action, _ns1__removeItemRequest *ns1__removeItemRequest, _ns1__removeItemResponse *ns1__removeItemResponse)
{
	struct __ns1__removeItemRequest soap_tmp___ns1__removeItemRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "removeItemRequest";
	soap_tmp___ns1__removeItemRequest.ns1__removeItemRequest=ns1__removeItemRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__removeItemRequest(soap, &soap_tmp___ns1__removeItemRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__removeItemRequest(soap, &soap_tmp___ns1__removeItemRequest, "-ns1:removeItemRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__removeItemRequest(soap, &soap_tmp___ns1__removeItemRequest, "-ns1:removeItemRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__removeItemResponse)
		return soap_closesock(soap);
	ns1__removeItemResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__removeItemResponse->soap_get(soap, "ns1:removeItemResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__startFreeBusySessionRequest(struct soap *soap, const char *URL, const char *action, _ns1__startFreeBusySessionRequest *ns1__startFreeBusySessionRequest, _ns1__startFreeBusySessionResponse *ns1__startFreeBusySessionResponse)
{
	struct __ns1__startFreeBusySessionRequest soap_tmp___ns1__startFreeBusySessionRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "startFreeBusySessionRequest";
	soap_tmp___ns1__startFreeBusySessionRequest.ns1__startFreeBusySessionRequest=ns1__startFreeBusySessionRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__startFreeBusySessionRequest(soap, &soap_tmp___ns1__startFreeBusySessionRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__startFreeBusySessionRequest(soap, &soap_tmp___ns1__startFreeBusySessionRequest, "-ns1:startFreeBusySessionRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__startFreeBusySessionRequest(soap, &soap_tmp___ns1__startFreeBusySessionRequest, "-ns1:startFreeBusySessionRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__startFreeBusySessionResponse)
		return soap_closesock(soap);
	ns1__startFreeBusySessionResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__startFreeBusySessionResponse->soap_get(soap, "ns1:startFreeBusySessionResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__closeFreeBusySessionRequest(struct soap *soap, const char *URL, const char *action, _ns1__closeFreeBusySessionRequest *ns1__closeFreeBusySessionRequest, _ns1__closeFreeBusySessionResponse *ns1__closeFreeBusySessionResponse)
{
	struct __ns1__closeFreeBusySessionRequest soap_tmp___ns1__closeFreeBusySessionRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "closeFreeBusySessionRequest";
	soap_tmp___ns1__closeFreeBusySessionRequest.ns1__closeFreeBusySessionRequest=ns1__closeFreeBusySessionRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__closeFreeBusySessionRequest(soap, &soap_tmp___ns1__closeFreeBusySessionRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__closeFreeBusySessionRequest(soap, &soap_tmp___ns1__closeFreeBusySessionRequest, "-ns1:closeFreeBusySessionRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__closeFreeBusySessionRequest(soap, &soap_tmp___ns1__closeFreeBusySessionRequest, "-ns1:closeFreeBusySessionRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__closeFreeBusySessionResponse)
		return soap_closesock(soap);
	ns1__closeFreeBusySessionResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__closeFreeBusySessionResponse->soap_get(soap, "ns1:closeFreeBusySessionResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__getFreeBusyRequest(struct soap *soap, const char *URL, const char *action, _ns1__getFreeBusyRequest *ns1__getFreeBusyRequest, _ns1__getFreeBusyResponse *ns1__getFreeBusyResponse)
{
	struct __ns1__getFreeBusyRequest soap_tmp___ns1__getFreeBusyRequest;
	soap->encodingStyle = NULL;
	if (!action)
		action = "getFreeBusyRequest";
	soap_tmp___ns1__getFreeBusyRequest.ns1__getFreeBusyRequest=ns1__getFreeBusyRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__getFreeBusyRequest(soap, &soap_tmp___ns1__getFreeBusyRequest);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put___ns1__getFreeBusyRequest(soap, &soap_tmp___ns1__getFreeBusyRequest, "-ns1:getFreeBusyRequest", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, URL, action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__getFreeBusyRequest(soap, &soap_tmp___ns1__getFreeBusyRequest, "-ns1:getFreeBusyRequest", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__getFreeBusyResponse)
		return soap_closesock(soap);
	ns1__getFreeBusyResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__getFreeBusyResponse->soap_get(soap, "ns1:getFreeBusyResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
#ifndef WITH_LEANER
	 || soap_resolve_attachments(soap)
#endif
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_END_NAMESPACE(soap)

/* end of soapClient.cpp */
