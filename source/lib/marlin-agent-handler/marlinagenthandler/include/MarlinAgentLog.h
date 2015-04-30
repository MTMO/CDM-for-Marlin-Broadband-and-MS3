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

#ifndef __MARLIN_AGENT_LOG_H__
#define __MARLIN_AGENT_LOG_H__

#if !defined(AGENT_LOG_ENABLE)

#define LOG_ENTER()
#define LOG_EXIT()
#define DBGMSG(...)
#define ERRMSG(...)
#define DUMP(in_len,in_buf)

#else // AGENT_LOG_ENABLE
#define LOG_ENTER() \
    do { \
        printf( \
                "==Enter== %s\n",__func__); \
    } while (0);

#define LOG_EXIT() \
    do { \
        printf( \
                "==Enter== %s\n",__func__); \
    } while (0);

#define DBGMSG(...)   printf(__VA_ARGS__)
#define ERRMSG(...)   printf(__VA_ARGS__)

#define DUMP(in_len,in_buf) \
    do { \
        u_int32_t i, j, x; \
        char buf[80]; \
        for (i = 0; i < in_len; i += 16) { \
            x = 0; \
            for (j = 0; j < 16 && i + j < in_len; j++) { \
                static const char l_hextab[] = "0123456789ABCDEF"; \
                u_int8_t c = in_buf[i+j]; \
                buf[x]   = ':'; \
                buf[x+1] = l_hextab[c>>4]; \
                buf[x+2] = l_hextab[c&15]; \
                x += 3; \
            } \
            while ( j++ < 16 ) { \
                buf[x ++] = ':'; \
                buf[x ++] = '-'; \
                buf[x ++] = '-'; \
            } \
            buf[x ++] = ' '; \
            for (j = 0; j < 16 && i + j < in_len; j++) { \
                u_int8_t c = in_buf[i+j]; \
                buf[x ++] = (c >= 0x20 && c < 0x7f) ? c : '.'; \
            } \
            buf[x ++] = '\n'; \
            buf[x] = 0; \
            printf("%s", buf); \
        } \
    } while (0);
#endif // AGENT_LOG_ENABLE
#endif /* __MARLIN_AGENT_LOG_H__ */
