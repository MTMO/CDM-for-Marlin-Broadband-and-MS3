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

#ifndef __MARLIN_MS3_FUNCTION_H__
#define __MARLIN_MS3_FUNCTION_H__

#include "IMarlinFunction.h"

namespace marlincdm {

class MS3Function : public IMarlinFunction {
protected:

public:

    MS3Function();
    MS3Function(const mcdm_SessionId_t& session_id);
    virtual ~MS3Function();

    virtual mcdm_status_t initUnit(const string content_data, string* message);

    virtual mcdm_status_t closeUnit(string* message);

    virtual mcdm_status_t processServiceToken(const string* mimeType,
                                              const mcdm_key_type* keyType,
                                              const string sas_data,
                                              map<string, string>* optionalParameters,
                                              string* request,
                                              string* url);

    virtual mcdm_status_t processServiceToken(const mcdm_key_type* keyType,
                                              const string init_data,
                                              map<string, string>* optionalParameters,
                                              string* request,
                                              string* url);

    virtual mcdm_status_t processServiceToken(const string* mimeType,
                                              const mcdm_key_type* keyType,
                                              const string init_data,
                                              string* request,
                                              string* url);

    virtual mcdm_status_t processServiceToken(const mcdm_key_type* keyType,
                                              const string suri,
                                              string* request,
                                              string* url);

    virtual mcdm_status_t processServiceToken(const string data,
                                              string* request,
                                              string* url);

    virtual mcdm_status_t processRightsResponse(const string& response,
                                                string* request);

private:

};  //class
};  //namespace

#endif /* __MARLIN_MS3_FUNCTION_H__ */
