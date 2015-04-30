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

#ifndef __MARLIN_BB_FUNCTION_H__
#define __MARLIN_BB_FUNCTION_H__

#include "IMarlinFunction.h"
#include "IMarlinMediaFormat.h"

namespace marlincdm {

class MBBFunction : public IMarlinFunction {

public:

    MBBFunction();
    MBBFunction(const mcdm_SessionId_t& session_id);
    virtual ~MBBFunction();

    virtual mcdm_status_t initUnit(const string content_data, string* message);

    virtual mcdm_status_t closeUnit(string* message);

    virtual mcdm_status_t processServiceToken(const string* mimeType,
                                              const mcdm_key_type* keyType,
                                              const string atkn_data,
                                              map<string, string>* optionalParameters,
                                              string* request,
                                              string* url);

    virtual mcdm_status_t processServiceToken(const mcdm_key_type* keyType,
                                              const string atkn_data,
                                              map<string, string>* optionalParameters,
                                              string* request,
                                              string* url);

    virtual mcdm_status_t processServiceToken(const string* mimeType,
                                              const mcdm_key_type* keyType,
                                              const string atkn_data,
                                              string* request,
                                              string* url);

    virtual mcdm_status_t processServiceToken(const mcdm_key_type* keyType,
                                              const string pssh,
                                              string* request,
                                              string* url);

    virtual mcdm_status_t processServiceToken(const string atkn,
                                              string* request,
                                              string* url);

    virtual mcdm_status_t processRightsResponse(const string& response,
                                                string* request);

    virtual mcdm_status_t removeKeys(const string& content_data, string* message);

    virtual mcdm_status_t removeKeys(string* message);

    virtual mcdm_status_t restoreKeys(const string& content_data, string* message);

    virtual mcdm_status_t queryKeyStatus(map<string, string>& infoMap);

    virtual mcdm_status_t queryKeyStatus(const string& name, string& value);

    mcdm_status_t processLTPMessage(const string* mimeType,
                                    const mcdm_key_type* keyType,
                                    const string ltp_data,
                                    string* request,
                                    string* url);

    mcdm_status_t processLTPMessage(const mcdm_key_type* keyType,
                                    const string ltp_data,
                                    string* request,
                                    string* url);

    mcdm_status_t getRegisteredStatus(string& service_id,
                                      string& account_id,
                                      string& subscription_id,
                                      string& state);

private:

};  //class
};  //namespace

#endif /* __MARLIN_BB_FUNCTION_H__ */
