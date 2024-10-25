// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: unetanalysis.177000.178000.proto

#ifndef PROTOBUF_unetanalysis_2e177000_2e178000_2eproto__INCLUDED
#define PROTOBUF_unetanalysis_2e177000_2e178000_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2004000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2004001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>
#include "ucloud.pb.h"
// @@protoc_insertion_point(includes)

namespace ucloud {
namespace unetanalysis {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_unetanalysis_2e177000_2e178000_2eproto();
void protobuf_AssignDesc_unetanalysis_2e177000_2e178000_2eproto();
void protobuf_ShutdownFile_unetanalysis_2e177000_2e178000_2eproto();

class StatsData;
class RecordStatsRequest;

enum MessageType {
  BEGINNING_ID = 177000,
  RECORD_STATS_REQUEST = 177001,
  ENDING_ID = 178000
};
bool MessageType_IsValid(int value);
const MessageType MessageType_MIN = BEGINNING_ID;
const MessageType MessageType_MAX = ENDING_ID;
const int MessageType_ARRAYSIZE = MessageType_MAX + 1;

const ::google::protobuf::EnumDescriptor* MessageType_descriptor();
inline const ::std::string& MessageType_Name(MessageType value) {
  return ::google::protobuf::internal::NameOfEnum(
    MessageType_descriptor(), value);
}
inline bool MessageType_Parse(
    const ::std::string& name, MessageType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<MessageType>(
    MessageType_descriptor(), name, value);
}
// ===================================================================

class StatsData : public ::google::protobuf::Message {
 public:
  StatsData();
  virtual ~StatsData();
  
  StatsData(const StatsData& from);
  
  inline StatsData& operator=(const StatsData& from) {
    CopyFrom(from);
    return *this;
  }
  
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }
  
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }
  
  static const ::google::protobuf::Descriptor* descriptor();
  static const StatsData& default_instance();
  
  void Swap(StatsData* other);
  
  // implements Message ----------------------------------------------
  
  StatsData* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const StatsData& from);
  void MergeFrom(const StatsData& from);
  void Clear();
  bool IsInitialized() const;
  
  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  
  ::google::protobuf::Metadata GetMetadata() const;
  
  // nested types ----------------------------------------------------
  
  // accessors -------------------------------------------------------
  
  // required string uuid = 10;
  inline bool has_uuid() const;
  inline void clear_uuid();
  static const int kUuidFieldNumber = 10;
  inline const ::std::string& uuid() const;
  inline void set_uuid(const ::std::string& value);
  inline void set_uuid(const char* value);
  inline void set_uuid(const char* value, size_t size);
  inline ::std::string* mutable_uuid();
  inline ::std::string* release_uuid();
  
  // required uint32 item_id = 20;
  inline bool has_item_id() const;
  inline void clear_item_id();
  static const int kItemIdFieldNumber = 20;
  inline ::google::protobuf::uint32 item_id() const;
  inline void set_item_id(::google::protobuf::uint32 value);
  
  // required uint32 region_id = 30;
  inline bool has_region_id() const;
  inline void clear_region_id();
  static const int kRegionIdFieldNumber = 30;
  inline ::google::protobuf::uint32 region_id() const;
  inline void set_region_id(::google::protobuf::uint32 value);
  
  // required uint32 isp_id = 40;
  inline bool has_isp_id() const;
  inline void clear_isp_id();
  static const int kIspIdFieldNumber = 40;
  inline ::google::protobuf::uint32 isp_id() const;
  inline void set_isp_id(::google::protobuf::uint32 value);
  
  // required uint64 value = 50;
  inline bool has_value() const;
  inline void clear_value();
  static const int kValueFieldNumber = 50;
  inline ::google::protobuf::uint64 value() const;
  inline void set_value(::google::protobuf::uint64 value);
  
  // required uint32 time = 60;
  inline bool has_time() const;
  inline void clear_time();
  static const int kTimeFieldNumber = 60;
  inline ::google::protobuf::uint32 time() const;
  inline void set_time(::google::protobuf::uint32 value);
  
  // @@protoc_insertion_point(class_scope:ucloud.unetanalysis.StatsData)
 private:
  inline void set_has_uuid();
  inline void clear_has_uuid();
  inline void set_has_item_id();
  inline void clear_has_item_id();
  inline void set_has_region_id();
  inline void clear_has_region_id();
  inline void set_has_isp_id();
  inline void clear_has_isp_id();
  inline void set_has_value();
  inline void clear_has_value();
  inline void set_has_time();
  inline void clear_has_time();
  
  ::google::protobuf::UnknownFieldSet _unknown_fields_;
  
  ::std::string* uuid_;
  ::google::protobuf::uint32 item_id_;
  ::google::protobuf::uint32 region_id_;
  ::google::protobuf::uint64 value_;
  ::google::protobuf::uint32 isp_id_;
  ::google::protobuf::uint32 time_;
  
  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(6 + 31) / 32];
  
  friend void  protobuf_AddDesc_unetanalysis_2e177000_2e178000_2eproto();
  friend void protobuf_AssignDesc_unetanalysis_2e177000_2e178000_2eproto();
  friend void protobuf_ShutdownFile_unetanalysis_2e177000_2e178000_2eproto();
  
  void InitAsDefaultInstance();
  static StatsData* default_instance_;
};
// -------------------------------------------------------------------

class RecordStatsRequest : public ::google::protobuf::Message {
 public:
  RecordStatsRequest();
  virtual ~RecordStatsRequest();
  
  RecordStatsRequest(const RecordStatsRequest& from);
  
  inline RecordStatsRequest& operator=(const RecordStatsRequest& from) {
    CopyFrom(from);
    return *this;
  }
  
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }
  
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }
  
  static const ::google::protobuf::Descriptor* descriptor();
  static const RecordStatsRequest& default_instance();
  
  void Swap(RecordStatsRequest* other);
  
  // implements Message ----------------------------------------------
  
  RecordStatsRequest* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const RecordStatsRequest& from);
  void MergeFrom(const RecordStatsRequest& from);
  void Clear();
  bool IsInitialized() const;
  
  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  
  ::google::protobuf::Metadata GetMetadata() const;
  
  // nested types ----------------------------------------------------
  
  // accessors -------------------------------------------------------
  
  // repeated .ucloud.unetanalysis.StatsData stats_data_list = 10;
  inline int stats_data_list_size() const;
  inline void clear_stats_data_list();
  static const int kStatsDataListFieldNumber = 10;
  inline const ::ucloud::unetanalysis::StatsData& stats_data_list(int index) const;
  inline ::ucloud::unetanalysis::StatsData* mutable_stats_data_list(int index);
  inline ::ucloud::unetanalysis::StatsData* add_stats_data_list();
  inline const ::google::protobuf::RepeatedPtrField< ::ucloud::unetanalysis::StatsData >&
      stats_data_list() const;
  inline ::google::protobuf::RepeatedPtrField< ::ucloud::unetanalysis::StatsData >*
      mutable_stats_data_list();
  
  // @@protoc_insertion_point(class_scope:ucloud.unetanalysis.RecordStatsRequest)
 private:
  
  ::google::protobuf::UnknownFieldSet _unknown_fields_;
  
  ::google::protobuf::RepeatedPtrField< ::ucloud::unetanalysis::StatsData > stats_data_list_;
  
  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];
  
  friend void  protobuf_AddDesc_unetanalysis_2e177000_2e178000_2eproto();
  friend void protobuf_AssignDesc_unetanalysis_2e177000_2e178000_2eproto();
  friend void protobuf_ShutdownFile_unetanalysis_2e177000_2e178000_2eproto();
  
  void InitAsDefaultInstance();
  static RecordStatsRequest* default_instance_;
};
// ===================================================================

static const int kRecordStatsRequestFieldNumber = 177001;
extern ::google::protobuf::internal::ExtensionIdentifier< ::ucloud::Body,
    ::google::protobuf::internal::MessageTypeTraits< ::ucloud::unetanalysis::RecordStatsRequest >, 11, false >
  record_stats_request;

// ===================================================================

// StatsData

// required string uuid = 10;
inline bool StatsData::has_uuid() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void StatsData::set_has_uuid() {
  _has_bits_[0] |= 0x00000001u;
}
inline void StatsData::clear_has_uuid() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void StatsData::clear_uuid() {
  if (uuid_ != &::google::protobuf::internal::kEmptyString) {
    uuid_->clear();
  }
  clear_has_uuid();
}
inline const ::std::string& StatsData::uuid() const {
  return *uuid_;
}
inline void StatsData::set_uuid(const ::std::string& value) {
  set_has_uuid();
  if (uuid_ == &::google::protobuf::internal::kEmptyString) {
    uuid_ = new ::std::string;
  }
  uuid_->assign(value);
}
inline void StatsData::set_uuid(const char* value) {
  set_has_uuid();
  if (uuid_ == &::google::protobuf::internal::kEmptyString) {
    uuid_ = new ::std::string;
  }
  uuid_->assign(value);
}
inline void StatsData::set_uuid(const char* value, size_t size) {
  set_has_uuid();
  if (uuid_ == &::google::protobuf::internal::kEmptyString) {
    uuid_ = new ::std::string;
  }
  uuid_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* StatsData::mutable_uuid() {
  set_has_uuid();
  if (uuid_ == &::google::protobuf::internal::kEmptyString) {
    uuid_ = new ::std::string;
  }
  return uuid_;
}
inline ::std::string* StatsData::release_uuid() {
  clear_has_uuid();
  if (uuid_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = uuid_;
    uuid_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}

// required uint32 item_id = 20;
inline bool StatsData::has_item_id() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void StatsData::set_has_item_id() {
  _has_bits_[0] |= 0x00000002u;
}
inline void StatsData::clear_has_item_id() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void StatsData::clear_item_id() {
  item_id_ = 0u;
  clear_has_item_id();
}
inline ::google::protobuf::uint32 StatsData::item_id() const {
  return item_id_;
}
inline void StatsData::set_item_id(::google::protobuf::uint32 value) {
  set_has_item_id();
  item_id_ = value;
}

// required uint32 region_id = 30;
inline bool StatsData::has_region_id() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void StatsData::set_has_region_id() {
  _has_bits_[0] |= 0x00000004u;
}
inline void StatsData::clear_has_region_id() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void StatsData::clear_region_id() {
  region_id_ = 0u;
  clear_has_region_id();
}
inline ::google::protobuf::uint32 StatsData::region_id() const {
  return region_id_;
}
inline void StatsData::set_region_id(::google::protobuf::uint32 value) {
  set_has_region_id();
  region_id_ = value;
}

// required uint32 isp_id = 40;
inline bool StatsData::has_isp_id() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void StatsData::set_has_isp_id() {
  _has_bits_[0] |= 0x00000008u;
}
inline void StatsData::clear_has_isp_id() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void StatsData::clear_isp_id() {
  isp_id_ = 0u;
  clear_has_isp_id();
}
inline ::google::protobuf::uint32 StatsData::isp_id() const {
  return isp_id_;
}
inline void StatsData::set_isp_id(::google::protobuf::uint32 value) {
  set_has_isp_id();
  isp_id_ = value;
}

// required uint64 value = 50;
inline bool StatsData::has_value() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void StatsData::set_has_value() {
  _has_bits_[0] |= 0x00000010u;
}
inline void StatsData::clear_has_value() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void StatsData::clear_value() {
  value_ = GOOGLE_ULONGLONG(0);
  clear_has_value();
}
inline ::google::protobuf::uint64 StatsData::value() const {
  return value_;
}
inline void StatsData::set_value(::google::protobuf::uint64 value) {
  set_has_value();
  value_ = value;
}

// required uint32 time = 60;
inline bool StatsData::has_time() const {
  return (_has_bits_[0] & 0x00000020u) != 0;
}
inline void StatsData::set_has_time() {
  _has_bits_[0] |= 0x00000020u;
}
inline void StatsData::clear_has_time() {
  _has_bits_[0] &= ~0x00000020u;
}
inline void StatsData::clear_time() {
  time_ = 0u;
  clear_has_time();
}
inline ::google::protobuf::uint32 StatsData::time() const {
  return time_;
}
inline void StatsData::set_time(::google::protobuf::uint32 value) {
  set_has_time();
  time_ = value;
}

// -------------------------------------------------------------------

// RecordStatsRequest

// repeated .ucloud.unetanalysis.StatsData stats_data_list = 10;
inline int RecordStatsRequest::stats_data_list_size() const {
  return stats_data_list_.size();
}
inline void RecordStatsRequest::clear_stats_data_list() {
  stats_data_list_.Clear();
}
inline const ::ucloud::unetanalysis::StatsData& RecordStatsRequest::stats_data_list(int index) const {
  return stats_data_list_.Get(index);
}
inline ::ucloud::unetanalysis::StatsData* RecordStatsRequest::mutable_stats_data_list(int index) {
  return stats_data_list_.Mutable(index);
}
inline ::ucloud::unetanalysis::StatsData* RecordStatsRequest::add_stats_data_list() {
  return stats_data_list_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::ucloud::unetanalysis::StatsData >&
RecordStatsRequest::stats_data_list() const {
  return stats_data_list_;
}
inline ::google::protobuf::RepeatedPtrField< ::ucloud::unetanalysis::StatsData >*
RecordStatsRequest::mutable_stats_data_list() {
  return &stats_data_list_;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace unetanalysis
}  // namespace ucloud

#ifndef SWIG
namespace google {
namespace protobuf {

template <>
inline const EnumDescriptor* GetEnumDescriptor< ucloud::unetanalysis::MessageType>() {
  return ucloud::unetanalysis::MessageType_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_unetanalysis_2e177000_2e178000_2eproto__INCLUDED
