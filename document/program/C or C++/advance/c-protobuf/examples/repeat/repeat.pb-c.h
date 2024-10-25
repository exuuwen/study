/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: repeat.proto */

#ifndef PROTOBUF_C_repeat_2eproto__INCLUDED
#define PROTOBUF_C_repeat_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1001001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _Data Data;
typedef struct _DataList DataList;


/* --- enums --- */


/* --- messages --- */

struct  _Data
{
  ProtobufCMessage base;
  /*
   * 分析目标 一般谁eip地址，如果是该机房总量uuid为特定值'all'
   */
  int32_t data;
};
#define DATA__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&data__descriptor) \
    , 0 }


struct  _DataList
{
  ProtobufCMessage base;
  /*
   * 用于分析的元数据
   */
  size_t n_data_list;
  Data **data_list;
};
#define DATA_LIST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&data_list__descriptor) \
    , 0,NULL }


/* Data methods */
void   data__init
                     (Data         *message);
size_t data__get_packed_size
                     (const Data   *message);
size_t data__pack
                     (const Data   *message,
                      uint8_t             *out);
size_t data__pack_to_buffer
                     (const Data   *message,
                      ProtobufCBuffer     *buffer);
Data *
       data__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   data__free_unpacked
                     (Data *message,
                      ProtobufCAllocator *allocator);
/* DataList methods */
void   data_list__init
                     (DataList         *message);
size_t data_list__get_packed_size
                     (const DataList   *message);
size_t data_list__pack
                     (const DataList   *message,
                      uint8_t             *out);
size_t data_list__pack_to_buffer
                     (const DataList   *message,
                      ProtobufCBuffer     *buffer);
DataList *
       data_list__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   data_list__free_unpacked
                     (DataList *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Data_Closure)
                 (const Data *message,
                  void *closure_data);
typedef void (*DataList_Closure)
                 (const DataList *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor data__descriptor;
extern const ProtobufCMessageDescriptor data_list__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_repeat_2eproto__INCLUDED */
