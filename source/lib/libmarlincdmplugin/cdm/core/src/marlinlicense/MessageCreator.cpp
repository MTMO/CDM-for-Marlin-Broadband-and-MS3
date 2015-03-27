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

#define LOG_TAG "MessageCreator"
#include "MarlinLog.h"

#include "MessageCreator.h"
#include "MarlinCdmUtils.h"
#include "json/json.h"

using namespace marlincdm;

bool createDefaultMessage(const uint32_t i_remaining_steps,
                          string* o_message)
{
    MARLINLOG_ENTER();
    bool ret = false;

    Json::Value root, properties;
    Json::StyledWriter writer;

    if (o_message == NULL) {
        goto EXIT;
    }

    // version
    properties[KEY_REQUEST_VERSION] = CURRENT_VERSION;

    // remaining steps
    properties[KEY_REMAINING_STEPS] = i_remaining_steps;

    // massage type
    properties[KEY_MASSAGE_TYPE] = MASSAGE_TYPE_NONE;

    root[KEY_REQUEST_DATA] = properties;
    o_message->assign(writer.write(root));
    ret = true;

EXIT:
    MARLINLOG_EXIT();
    return ret;
}

bool createPropertyResponse(const string i_property,
                            string* o_message)
{
    MARLINLOG_ENTER();
    bool ret = true;

    Json::Value root, properties;
    Json::StyledWriter writer;

    if (o_message == NULL) {
        LOGE("Input parameter is invalid.");
        goto EXIT;
    }

    // property
    properties[PROPERTY_EME_QUERY_RESPONSE][PROPERTY_EME_RESPONSE_VALUE] = i_property;

    root[KEY_REQUEST_DATA] = properties;
    o_message->assign(writer.write(root));
    ret = true;

EXIT:
    MARLINLOG_EXIT();
    return ret;
}

bool createPropertyMessage(const MH_string_t* i_property,
                           string* o_message)
{
    MARLINLOG_ENTER();
    bool ret = true;

    Json::Value root, properties;
    Json::StyledWriter writer;

    if (i_property == NULL || i_property->bufstr == NULL || i_property->length <= 0) {
        goto EXIT;
    }
    if (o_message == NULL) {
        LOGE("Input parameter is invalid.");
        goto EXIT;
    }

    // version
    properties[KEY_REQUEST_VERSION] = CURRENT_VERSION;

    // remaining steps
    properties[KEY_REMAINING_STEPS] = 0;

    // massage type
    properties[KEY_MASSAGE_TYPE] = MASSAGE_TYPE_PROPERTY;

    // property
    properties[PROPERTY_DATA] = string(reinterpret_cast<const char*>(i_property->bufstr), i_property->length);

    root[KEY_REQUEST_DATA] = properties;
    o_message->assign(writer.write(root));
    ret = true;

EXIT:
    MARLINLOG_EXIT();
    return ret;
}

bool createPropertyMessage(const string i_property, string* o_message)
{
    MARLINLOG_ENTER();
    bool ret = false;
    Json::Value root, properties;
    Json::StyledWriter writer;

    if (o_message == NULL) {
        LOGE("Input parameter is invalid.");
        goto EXIT;
    }

    // version
    properties[KEY_REQUEST_VERSION] = CURRENT_VERSION;

    // remaining steps
    properties[KEY_REMAINING_STEPS] = 0;

    // massage type
    properties[KEY_MASSAGE_TYPE] = MASSAGE_TYPE_PROPERTY;

    // registered state
    properties[PROPERTY_DATA] = i_property;

    root[KEY_REQUEST_DATA] = properties;
    o_message->assign(writer.write(root));
    ret = true;

EXIT:
    MARLINLOG_EXIT();
    return ret;
}

bool createLicenseMessage(const MH_string_t* i_license,
                          const uint32_t i_remaining_steps,
                          string* o_message)
{
    MARLINLOG_ENTER();
    bool ret = false;

    Json::Value root, properties, license;
    Json::StyledWriter writer;

    if (i_license == NULL || i_license->bufstr == NULL || i_license->length <= 0) {
        goto EXIT;
    }
    if (o_message == NULL) {
        LOGE("Input parameter is invalid.");
        goto EXIT;
    }

    // version
    properties[KEY_REQUEST_VERSION] = CURRENT_VERSION;

    // remaining steps
    properties[KEY_REMAINING_STEPS] = i_remaining_steps;

    // massage type
    properties[KEY_MASSAGE_TYPE] = MASSAGE_TYPE_LICENSE;

    // license
    properties[LICENSE_DATA] = string(reinterpret_cast<const char*>(i_license->bufstr), i_license->length);

    root[KEY_REQUEST_DATA] = properties;
    o_message->assign(writer.write(root));
    ret = true;

EXIT:
    MARLINLOG_EXIT();
    return ret;
}

bool createHttpRequestMessage(const MH_httpRequest_t i_request,
                              const uint32_t i_remaining_steps,
                              string* o_message,
                              string* o_url)
{
    MARLINLOG_ENTER();
    bool ret = false;
    MH_string_t* in_request = i_request.request;
    MH_header_list_t header_list = i_request.headers;
    char* url = i_request.url;
    uint8_t method = i_request.method;
    map<string, string>::const_iterator itr;

    Json::Value root, properties,
                http_request, headers, header;
    Json::StyledWriter writer;

    if (o_message == NULL || o_url == NULL) {
        LOGE("Input parameter is invalid.");
        goto EXIT;
    }

    // version
    properties[KEY_REQUEST_VERSION] = CURRENT_VERSION;

    // remaining steps
    properties[KEY_REMAINING_STEPS] = i_remaining_steps;

    // massage type
    properties[KEY_MASSAGE_TYPE] = MASSAGE_TYPE_REQUEST;

    // method in properties of http request
    if (method == MH_GET) {
        http_request[METHOD] = GET;
    } else if (method == MH_POST) {
        http_request[METHOD] = POST;
    } else {
        goto EXIT;
    }

    // url in properties of http request
    http_request[URL] = string(url);
    *o_url = string(url);

    // headers in properties of http request
    itr = header_list.begin();
    for (int i = 0; itr != header_list.end(); itr++, i++) {
        header[HEADER_NAME] = itr->first;
        header[HEADER_VALUE] = itr->second;
        http_request[REQUEST_HEADERS][static_cast<unsigned int>(i)] = header;
    }

    // body for http request
    if (in_request != NULL && in_request->bufstr != NULL && in_request->length > 0) {
        http_request[REQUEST_BODY] = string(reinterpret_cast<const char*>(in_request->bufstr), in_request->length);
    }

    properties[HTTP_REQUEST] = http_request;
    root[KEY_REQUEST_DATA] = properties;
    o_message->assign(writer.write(root));
    ret = true;

EXIT:
    MARLINLOG_EXIT();
    return ret;
}

bool createHttpRequestMessageForEME(const MH_httpRequest_t i_request,
                                    const uint32_t i_remaining_steps,
                                    string* o_message,
                                    string* o_url)
{
    MARLINLOG_ENTER();
    bool ret = false;
    MH_string_t* in_request = i_request.request;
    MH_header_list_t header_list = i_request.headers;
    char* url = i_request.url;
    uint8_t method = i_request.method;
    map<string, string>::const_iterator itr;

    Json::Value root, properties,
                http_request, headers, header;
    Json::StyledWriter writer;

    if (o_message == NULL || o_url == NULL) {
        LOGE("Input parameter is invalid.");
        goto EXIT;
    }

    // method in properties of http request
    if (method == MH_GET) {
        http_request[METHOD] = GET;
    } else if (method == MH_POST) {
        http_request[METHOD] = POST;
    } else {
        goto EXIT;
    }

    // url in properties of http request
    http_request[URL] = string(url);
    *o_url = string(url);

    // headers in properties of http request
    itr = header_list.begin();
    for (int i = 0; itr != header_list.end(); itr++, i++) {
        header[HEADER_NAME] = itr->first;
        header[HEADER_VALUE] = itr->second;
        http_request[REQUEST_HEADERS][static_cast<unsigned int>(i)] = header;
    }

    // body for http request
    if (in_request != NULL && in_request->bufstr != NULL && in_request->length > 0) {
        http_request[REQUEST_BODY] = string(reinterpret_cast<const char*>(in_request->bufstr), in_request->length);
    }

    properties[HTTP_REQUEST] = http_request;
    root[KEY_REQUEST_DATA] = properties;
    o_message->assign(writer.write(root));
    ret = true;

EXIT:
    MARLINLOG_EXIT();
    return ret;
}

bool createCompleteMessage(const string i_actionName,
                           const uint32_t i_action_result,
                           const uint32_t i_result_status,
                           const uint32_t i_remaining_steps,
                           string* o_message)
{
    MARLINLOG_ENTER();
    bool ret = false;
    Json::Value root, properties,
                      complete_message, reason;
    Json::StyledWriter writer;

    if (o_message == NULL) {
        LOGE("Input parameter is invalid.");
        goto EXIT;
    }

    complete_message[RESULT_CODE] = i_result_status;
    if (i_result_status != SUCCESS) {
        reason[ACTION_NAME] = i_actionName;
        reason[ERROR_CODE] = i_action_result;
        complete_message[REASON] = complete_message;
    }

    properties[COMPLETE_NOTIFICATION] = complete_message;
    root[KEY_REQUEST_DATA] = properties;
    o_message->assign(writer.write(root));
    ret = true;

EXIT:
    MARLINLOG_EXIT();
    return ret;
}

bool createLTPMessage(const MH_string_t* i_message, string* o_message)
{
    MARLINLOG_ENTER();
    bool ret = false;

    Json::Value root, properties, message;
    Json::StyledWriter writer;
    string ltp;

    if (i_message == NULL || i_message->bufstr == NULL || i_message->length <= 0) {
        goto EXIT;
    }
    if (o_message == NULL) {
        LOGE("Input parameter is invalid.");
        goto EXIT;
    }

    // version
    properties[KEY_REQUEST_VERSION] = CURRENT_VERSION;

    // massage type
    properties[KEY_MASSAGE_TYPE] = MASSAGE_TYPE_LTP;

    // ltp message
    binToStr(i_message->bufstr, i_message->length, ltp);
    properties[LTP_DATA][LTP_MESSAGE] = ltp;

    root[KEY_REQUEST_DATA] = properties;
    o_message->assign(writer.write(root));
    ret = true;

EXIT:
    MARLINLOG_EXIT();
    return ret;
}
