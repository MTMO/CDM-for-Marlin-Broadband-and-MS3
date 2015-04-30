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

#ifndef __MARLIN_FUNCTION_H__
#define __MARLIN_FUNCTION_H__

#include <map>

#include "IMarlinFunction.h"

namespace marlincdm {

class MarlinFunction : public IMarlinFunction {
    protected:

    public:
    MarlinFunction(){};
    virtual ~MarlinFunction(){};

    virtual mcdm_status_t processServiceToken(const string* mimeType,
                                              const mcdm_key_type* keyType,
                                              const string atkn_data,
                                              map<string, string>* optionalParameters,
                                              string* request,
                                              string* url)
    {
        mcdm_status_t status = ERROR_UNKNOWN;
        return status;
    };

    virtual mcdm_status_t processServiceToken(const mcdm_key_type* keyType,
                                              const string atkn_data,
                                              map<string, string>* optionalParameters,
                                              string* request,
                                              string* url)
    {
        mcdm_status_t status = ERROR_UNKNOWN;
        return status;
    };

    virtual mcdm_status_t processServiceToken(const string* mimeType,
                                              const mcdm_key_type* keyType,
                                              const string atkn_data,
                                              string* request,
                                              string* url)
    {
        mcdm_status_t status = ERROR_UNKNOWN;
        return status;
    };

    virtual mcdm_status_t processServiceToken(const mcdm_key_type* keyType,
                                              const string atkn_data,
                                              string* request,
                                              string* url)
    {
        mcdm_status_t status = ERROR_UNKNOWN;
        return status;
    };

    virtual mcdm_status_t processServiceToken(const string atkn_data,
                                              string* request,
                                              string* url)
    {
        mcdm_status_t status = ERROR_UNKNOWN;
        return status;
    };

    virtual mcdm_status_t processRightsResponse(const string& response,
                                                string* request)
    {
        mcdm_status_t status = ERROR_UNKNOWN;
        return status;
    };

}; //class
}; //namespace

#endif /* __MARLIN_FUNCTION_H__ */
