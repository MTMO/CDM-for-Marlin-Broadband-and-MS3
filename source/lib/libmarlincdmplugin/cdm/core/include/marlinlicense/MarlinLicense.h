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

#ifndef __MARLIN_LICENSE_H__
#define __MARLIN_LICENSE_H__

#include "IMarlinLicense.h"

namespace marlincdm {

class MarlinLicense : public IMarlinLicense {
public:

    MarlinLicense(){};
    virtual ~MarlinLicense(){};

    virtual mcdm_status_t processServiceToken(string* request,
                                              string* uri,
                                              uint32_t& remaining_steps)
    {
        return ERROR_UNKNOWN;
    };

    virtual mcdm_status_t processServiceToken(mcdm_data_t atkn,
                                              map<string, string>* optionalParameters,
                                              string* request,
                                              string* url)
    {
        return ERROR_UNKNOWN;
    };

    virtual mcdm_status_t processRightsResponse(mcdm_data_t& response,
                                                bool err,
                                                uint32_t& remaining_steps,
                                                string* message,
                                                mcdm_data_t* license,
                                                mcdm_data_t* service_info,
                                                bool& is_license_exist)
    {
        return ERROR_UNKNOWN;
    };

private:

};  //class
};  //namespace

#endif /* __MARLIN_LICENSE_H__ */
