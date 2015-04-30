/*
 * (c) 2015 - Copyright Marlin Trust Management Organization
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MESSAGE_CREATOR_H__
#define __MESSAGE_CREATOR_H__

#include "MarlinCommonTypes.h"
#include "CommonHeaderParser.h"
#include "MarlinAgentHandlerType.h"

typedef string REQUEST_KEY_TYPE_NAME;
typedef string REQUEST_VALUE_TYPE_STRING;
typedef int32_t REQUEST_VALUE_TYPE_INTEGER;

static const REQUEST_KEY_TYPE_NAME KEY_REQUEST_DATA = "properties";

// properties
static const REQUEST_KEY_TYPE_NAME KEY_REQUEST_VERSION = "version";
static const REQUEST_KEY_TYPE_NAME KEY_REMAINING_STEPS = "remaining_steps";
static const REQUEST_KEY_TYPE_NAME KEY_MASSAGE_TYPE = "message_type";

// http requet
static const REQUEST_KEY_TYPE_NAME HTTP_REQUEST = "httpRequest";
static const REQUEST_KEY_TYPE_NAME METHOD = "method";
static const REQUEST_KEY_TYPE_NAME URL = "url";
static const REQUEST_KEY_TYPE_NAME REQUEST_HEADERS = "headers";
static const REQUEST_KEY_TYPE_NAME HEADER_NAME = "name";
static const REQUEST_KEY_TYPE_NAME HEADER_VALUE = "value";
static const REQUEST_KEY_TYPE_NAME REQUEST_BODY = "body";

// complete notification
static const REQUEST_KEY_TYPE_NAME COMPLETE_NOTIFICATION = "completeNotification";
static const REQUEST_KEY_TYPE_NAME RESULT_CODE = "resultCode";
static const REQUEST_KEY_TYPE_NAME REASON = "reason";
static const REQUEST_KEY_TYPE_NAME ACTION_NAME = "actionName";
static const REQUEST_KEY_TYPE_NAME ERROR_CODE = "code";

typedef REQUEST_VALUE_TYPE_STRING EME_ACTION_NAME;
static const EME_ACTION_NAME ACTION_TYPE_ATKN_PROCESS("ActionTokenProcessing");
static const EME_ACTION_NAME SAS_TYPE_ATKN_PROCESS("SASProcessing");

typedef REQUEST_VALUE_TYPE_INTEGER EME_RESULT_CODE;
static const EME_RESULT_CODE SUCCESS(0);
static const EME_RESULT_CODE FAIL(1);

// license
static const REQUEST_KEY_TYPE_NAME LICENSE_DATA = "license";

// property
static const REQUEST_KEY_TYPE_NAME PROPERTY_DATA = "prop";
static const REQUEST_KEY_TYPE_NAME PROPERTY_EME_QUERY_RESPONSE = "queryResponse";
static const REQUEST_KEY_TYPE_NAME PROPERTY_EME_RESPONSE_VALUE = "value";

// ltp
static const REQUEST_KEY_TYPE_NAME LTP_DATA = "ltp";
static const REQUEST_KEY_TYPE_NAME LTP_MESSAGE = "message";

// MASSAGE TYPE
typedef REQUEST_VALUE_TYPE_STRING REQUEST_MASSAGE_TYPE;
static const REQUEST_MASSAGE_TYPE MASSAGE_TYPE_NONE("none");
static const REQUEST_MASSAGE_TYPE MASSAGE_TYPE_REQUEST("httpRequest");
static const REQUEST_MASSAGE_TYPE MASSAGE_TYPE_LTP("ltp");
static const REQUEST_MASSAGE_TYPE MASSAGE_TYPE_LICENSE("license");
static const REQUEST_MASSAGE_TYPE MASSAGE_TYPE_PROPERTY("prop");

// HTTP METHOD
typedef REQUEST_VALUE_TYPE_STRING METHOD_TYPE;
static const METHOD_TYPE GET("GET");
static const METHOD_TYPE POST("POST");

bool createDefaultMessage(const uint32_t i_remaining_steps,
                          string* o_message);

bool createPropertyResponse(const string i_property, string* o_message);

bool createPropertyMessage(const MH_string_t* i_property, string* o_message);

bool createPropertyMessage(const string i_property, string* o_message);

bool createLicenseMessage(const MH_string_t* i_license,
                          const uint32_t i_remaining_steps,
                          string* o_message);

bool createHttpRequestMessage(const MH_httpRequest_t i_request,
                              const uint32_t i_remaining_steps,
                              string* o_message,
                              string* o_url);

bool createHttpRequestMessageForEME(const MH_httpRequest_t i_request,
                                    const uint32_t i_remaining_steps,
                                    string* o_message,
                                    string* o_url);

bool createCompleteMessage(const string i_actionName,
                           const uint32_t i_action_result,
                           const uint32_t i_result_status,
                           const uint32_t i_remaining_steps,
                           string* o_message);

bool createLTPMessage(const MH_string_t* i_license, string* o_message);

#endif /* __MESSAGE_CREATOR_H__ */
