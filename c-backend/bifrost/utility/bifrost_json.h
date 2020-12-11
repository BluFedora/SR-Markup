/******************************************************************************/
/*!
* @file   bf_json.h
* @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
* @brief
*   Basic Json parser with an Event (SAX) API.
*   Has some extensions to make wrting json easier.
*   Just search for '@JsonSpecExtention' in the source file.
*
* @version 0.0.1
* @date    2020-03-14
*
* @copyright Copyright (c) 2020
*/
/******************************************************************************/
#ifndef BF_JSON_H
#define BF_JSON_H

#include <stdbool.h> /* bool, true, false */
#include <stddef.h>  /* size_t   */
#include <stdint.h>  /* uint32_t */

#if __cplusplus
extern "C" {
#endif

/* Customizable Constants */

enum
{
  k_bfJsonUserStorageSize = 64,
  k_bfJsonStringBlockSize = 256,
};

/* String View */

typedef struct
{
  const char* string;
  size_t      length;

} bfJsonString;

static inline bfJsonString bfJsonString_fromCstr(const char* null_terminated_string)
{
  bfJsonString result;

  result.string = null_terminated_string;
  result.length = 0;

  while (null_terminated_string[result.length])
  {
    ++result.length;
  }

  return result;
}

static inline bfJsonString bfJsonString_fromRange(const char* str_begin, const char* str_end)
{
  bfJsonString result;

  result.string = str_begin;
  result.length = str_end - str_begin;

  return result;
}

/* Reader API (String -> Object) */

typedef enum
{
  BF_JSON_EVENT_BEGIN_DOCUMENT,
  BF_JSON_EVENT_END_DOCUMENT,
  BF_JSON_EVENT_BEGIN_ARRAY,
  BF_JSON_EVENT_END_ARRAY,
  BF_JSON_EVENT_BEGIN_OBJECT,
  BF_JSON_EVENT_END_OBJECT,
  BF_JSON_EVENT_KEY,
  BF_JSON_EVENT_VALUE,
  BF_JSON_EVENT_PARSE_ERROR,

} bfJsonEvent;

typedef enum
{
  BF_JSON_VALUE_STRING,
  BF_JSON_VALUE_NUMBER,
  BF_JSON_VALUE_BOOLEAN,
  BF_JSON_VALUE_NULL,

} bfJsonType;

struct bfJsonParserContext_t;
typedef struct bfJsonParserContext_t bfJsonParserContext;

typedef void (*bfJsonFn)(bfJsonParserContext* ctx, bfJsonEvent event, void* user_data);

void         bfJsonParser_fromString(char* source, size_t source_length, bfJsonFn callback, void* user_data);
const char*  bfJsonParser_errorMessage(const bfJsonParserContext* ctx);
bfJsonType   bfJsonParser_valueType(const bfJsonParserContext* ctx);
bool         bfJsonParser_valueIs(const bfJsonParserContext* ctx, bfJsonType type);
bfJsonString bfJsonParser_asString(const bfJsonParserContext* ctx);
double       bfJsonParser_asNumber(const bfJsonParserContext* ctx);
bool         bfJsonParser_asBoolean(const bfJsonParserContext* ctx);
void*        bfJsonParser_userStorage(const bfJsonParserContext* ctx);
void*        bfJsonParser_parentUserStorage(const bfJsonParserContext* ctx);

/* Writer API (Object -> String) */

struct bfJsonWriter_t;
typedef struct bfJsonWriter_t bfJsonWriter;

struct bfJsonStringBlock_t;
typedef struct bfJsonStringBlock_t bfJsonStringBlock;

typedef void* (*bfJsonAllocFn)(size_t size, void* user_data);
typedef void (*bfJsonFreeFn)(void* ptr, void* user_data);
typedef void (*bfJsonWriterForEachFn)(const bfJsonStringBlock* block, void* user_data);

bfJsonWriter* bfJsonWriter_new(bfJsonAllocFn alloc_fn, void* user_data);
bfJsonWriter* bfJsonWriter_newCRTAlloc(void);
size_t        bfJsonWriter_length(const bfJsonWriter* self);
void          bfJsonWriter_beginArray(bfJsonWriter* self);
void          bfJsonWriter_endArray(bfJsonWriter* self);
void          bfJsonWriter_beginObject(bfJsonWriter* self);
void          bfJsonWriter_key(bfJsonWriter* self, bfJsonString key);
void          bfJsonWriter_valueString(bfJsonWriter* self, bfJsonString value);
void          bfJsonWriter_valueNumber(bfJsonWriter* self, double value);
void          bfJsonWriter_valueBoolean(bfJsonWriter* self, bool value);
void          bfJsonWriter_valueNull(bfJsonWriter* self);
void          bfJsonWriter_next(bfJsonWriter* self);
void          bfJsonWriter_indent(bfJsonWriter* self, int num_spaces);
void          bfJsonWriter_write(bfJsonWriter* self, const char* str, size_t length);
void          bfJsonWriter_endObject(bfJsonWriter* self);
void          bfJsonWriter_forEachBlock(const bfJsonWriter* self, bfJsonWriterForEachFn fn, void* user_data);
void          bfJsonWriter_delete(bfJsonWriter* self, bfJsonFreeFn free_fn);
void          bfJsonWriter_deleteCRT(bfJsonWriter* self);
bfJsonString  bfJsonStringBlock_string(const bfJsonStringBlock* block);

#if __cplusplus
}
#endif

#endif /* BF_JSON_H */
