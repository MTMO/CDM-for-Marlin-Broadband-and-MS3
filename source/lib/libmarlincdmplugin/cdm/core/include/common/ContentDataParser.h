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


#ifndef __MARLIN_CONTENT_DATA_PARSER_H__
#define __MARLIN_CONTENT_DATA_PARSER_H__

#include <map>

#include "MarlinCommonTypes.h"

namespace marlincdm {

class ContentDataParser {
public:
  ContentDataParser(const mcdm_data_t data);
  ContentDataParser(const uint8_t* data , size_t len);
  virtual ~ContentDataParser();

  map<int, mcdm_data_t>& getDataMap();

  mcdm_data_t& getData(int index);

  mcdm_data_t& getData();

private:
  mcdm_data_t nullData;
  mcdm_data_t mData;
  map<int, mcdm_data_t> mDataMap;

  void parse();
};
} // namespace marlincdm

#endif /* __MARLIN_CONTENT_DATA_PARSER_H__ */
