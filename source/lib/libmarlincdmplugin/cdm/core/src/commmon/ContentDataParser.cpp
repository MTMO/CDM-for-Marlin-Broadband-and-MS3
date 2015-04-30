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

#define LOG_TAG "ContentDataParser"
#include "MarlinLog.h"

#include <cstring>
#include "ContentDataParser.h"
#include "MarlinCdmUtils.h"

using namespace marlincdm;

ContentDataParser::ContentDataParser(const mcdm_data_t data)
{
    mDataMap.clear();
    mData.len = 0;
    mData.data = NULL;
    nullData.len = 0;
    nullData.data = NULL;

    if (data.data != NULL && data.len > 0) {
        mData.data = new uint8_t[data.len];
        memcpy(mData.data, data.data, data.len);
        mData.len = data.len;
        parse();
    }
}

ContentDataParser::ContentDataParser(const uint8_t* data , size_t len)
{
    mDataMap.clear();
    mData.len = 0;
    mData.data = NULL;
    nullData.len = 0;
    nullData.data = NULL;

    if (data != NULL && len > 0) {
        mData.data = new uint8_t[len];
        memcpy(mData.data, data, len);
        mData.len = len;
        parse();
    }
}

ContentDataParser::~ContentDataParser()
{
    MARLINLOG_ENTER();
    map<int, mcdm_data_t>::const_iterator itr;
    if (mDataMap.size() > 0) {
        itr = mDataMap.begin();
        for (int i = 0; itr != mDataMap.end(); itr++, i++) {
            uint8_t* data = itr->second.data;
            if (data != NULL) {
                delete data;
                data = NULL;
            }
        }
        mDataMap.clear();
    }
}

map<int, mcdm_data_t>& ContentDataParser::getDataMap()
{
    return mDataMap;
}

mcdm_data_t& ContentDataParser::getData(int index)
{
    if (mDataMap.count(index) != 0) {
        return mDataMap[index];
    }
    return nullData;
}

mcdm_data_t& ContentDataParser::getData()
{
    return mData;
}

void ContentDataParser::parse()
{
    MARLINLOG_ENTER();
    size_t offset = 0;
    for (int index = 0; offset < mData.len; index++) {
        mcdm_data_t data = {0, NULL};
        uint32_t length = 0;

        read32(mData.data + offset, length);
        offset += sizeof(uint32_t);

        LOGD("offset[%d] length[%d] mData.len[%d]", offset, length, mData.len);
        if (offset + length > mData.len) {
            LOGD("Content data format is ILLEGAL");
            map<int, mcdm_data_t>::const_iterator itr;
            if (mDataMap.size() > 0) {
                itr = mDataMap.begin();
                for (int i = 0; itr != mDataMap.end(); itr++, i++) {
                    uint8_t* data = itr->second.data;
                    if (data != NULL) {
                        delete data;
                        data = NULL;
                    }
                }
                mDataMap.clear();
            }
            break;
        }
        data.len = length;
        data.data = new uint8_t[length];
        memcpy(data.data, mData.data + offset, length);
        offset += length;

        mDataMap[index] = data;
    }
    MARLINLOG_EXIT();
}
