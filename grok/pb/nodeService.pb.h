// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: nodeService.proto

#ifndef PROTOBUF_nodeService_2eproto__INCLUDED
#define PROTOBUF_nodeService_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace nodeService {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_nodeService_2eproto();
void protobuf_AssignDesc_nodeService_2eproto();
void protobuf_ShutdownFile_nodeService_2eproto();

class MsgPack;
class ReqNodeRegistor;
class RspNodeRegistor;

enum RegistorResultType {
  eRegister_OK = 0,
  eRegister_ParamErr = 1,
  RegistorResultType_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  RegistorResultType_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool RegistorResultType_IsValid(int value);
const RegistorResultType RegistorResultType_MIN = eRegister_OK;
const RegistorResultType RegistorResultType_MAX = eRegister_ParamErr;
const int RegistorResultType_ARRAYSIZE = RegistorResultType_MAX + 1;

const ::google::protobuf::EnumDescriptor* RegistorResultType_descriptor();
inline const ::std::string& RegistorResultType_Name(RegistorResultType value) {
  return ::google::protobuf::internal::NameOfEnum(
    RegistorResultType_descriptor(), value);
}
inline bool RegistorResultType_Parse(
    const ::std::string& name, RegistorResultType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<RegistorResultType>(
    RegistorResultType_descriptor(), name, value);
}
enum MsgType {
  eMsg_none = 0,
  eMsg_request = 1,
  eMsg_response = 2,
  eMsg_notify = 3,
  MsgType_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  MsgType_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool MsgType_IsValid(int value);
const MsgType MsgType_MIN = eMsg_none;
const MsgType MsgType_MAX = eMsg_notify;
const int MsgType_ARRAYSIZE = MsgType_MAX + 1;

const ::google::protobuf::EnumDescriptor* MsgType_descriptor();
inline const ::std::string& MsgType_Name(MsgType value) {
  return ::google::protobuf::internal::NameOfEnum(
    MsgType_descriptor(), value);
}
inline bool MsgType_Parse(
    const ::std::string& name, MsgType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<MsgType>(
    MsgType_descriptor(), name, value);
}
// ===================================================================

class ReqNodeRegistor : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:nodeService.ReqNodeRegistor) */ {
 public:
  ReqNodeRegistor();
  virtual ~ReqNodeRegistor();

  ReqNodeRegistor(const ReqNodeRegistor& from);

  inline ReqNodeRegistor& operator=(const ReqNodeRegistor& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ReqNodeRegistor& default_instance();

  void Swap(ReqNodeRegistor* other);

  // implements Message ----------------------------------------------

  inline ReqNodeRegistor* New() const { return New(NULL); }

  ReqNodeRegistor* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ReqNodeRegistor& from);
  void MergeFrom(const ReqNodeRegistor& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(ReqNodeRegistor* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // @@protoc_insertion_point(class_scope:nodeService.ReqNodeRegistor)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_nodeService_2eproto();
  friend void protobuf_AssignDesc_nodeService_2eproto();
  friend void protobuf_ShutdownFile_nodeService_2eproto();

  void InitAsDefaultInstance();
  static ReqNodeRegistor* default_instance_;
};
// -------------------------------------------------------------------

class RspNodeRegistor : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:nodeService.RspNodeRegistor) */ {
 public:
  RspNodeRegistor();
  virtual ~RspNodeRegistor();

  RspNodeRegistor(const RspNodeRegistor& from);

  inline RspNodeRegistor& operator=(const RspNodeRegistor& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const RspNodeRegistor& default_instance();

  void Swap(RspNodeRegistor* other);

  // implements Message ----------------------------------------------

  inline RspNodeRegistor* New() const { return New(NULL); }

  RspNodeRegistor* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const RspNodeRegistor& from);
  void MergeFrom(const RspNodeRegistor& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(RspNodeRegistor* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional .nodeService.RegistorResultType status = 1;
  void clear_status();
  static const int kStatusFieldNumber = 1;
  ::nodeService::RegistorResultType status() const;
  void set_status(::nodeService::RegistorResultType value);

  // @@protoc_insertion_point(class_scope:nodeService.RspNodeRegistor)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  int status_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_nodeService_2eproto();
  friend void protobuf_AssignDesc_nodeService_2eproto();
  friend void protobuf_ShutdownFile_nodeService_2eproto();

  void InitAsDefaultInstance();
  static RspNodeRegistor* default_instance_;
};
// -------------------------------------------------------------------

class MsgPack : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:nodeService.MsgPack) */ {
 public:
  MsgPack();
  virtual ~MsgPack();

  MsgPack(const MsgPack& from);

  inline MsgPack& operator=(const MsgPack& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const MsgPack& default_instance();

  void Swap(MsgPack* other);

  // implements Message ----------------------------------------------

  inline MsgPack* New() const { return New(NULL); }

  MsgPack* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const MsgPack& from);
  void MergeFrom(const MsgPack& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(MsgPack* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional string source = 1;
  void clear_source();
  static const int kSourceFieldNumber = 1;
  const ::std::string& source() const;
  void set_source(const ::std::string& value);
  void set_source(const char* value);
  void set_source(const char* value, size_t size);
  ::std::string* mutable_source();
  ::std::string* release_source();
  void set_allocated_source(::std::string* source);

  // optional string dest = 2;
  void clear_dest();
  static const int kDestFieldNumber = 2;
  const ::std::string& dest() const;
  void set_dest(const ::std::string& value);
  void set_dest(const char* value);
  void set_dest(const char* value, size_t size);
  ::std::string* mutable_dest();
  ::std::string* release_dest();
  void set_allocated_dest(::std::string* dest);

  // optional string msgname = 3;
  void clear_msgname();
  static const int kMsgnameFieldNumber = 3;
  const ::std::string& msgname() const;
  void set_msgname(const ::std::string& value);
  void set_msgname(const char* value);
  void set_msgname(const char* value, size_t size);
  ::std::string* mutable_msgname();
  ::std::string* release_msgname();
  void set_allocated_msgname(::std::string* msgname);

  // optional .nodeService.MsgType msgtype = 4;
  void clear_msgtype();
  static const int kMsgtypeFieldNumber = 4;
  ::nodeService::MsgType msgtype() const;
  void set_msgtype(::nodeService::MsgType value);

  // optional uint32 sessionid = 5;
  void clear_sessionid();
  static const int kSessionidFieldNumber = 5;
  ::google::protobuf::uint32 sessionid() const;
  void set_sessionid(::google::protobuf::uint32 value);

  // optional bytes pbdata = 6;
  void clear_pbdata();
  static const int kPbdataFieldNumber = 6;
  const ::std::string& pbdata() const;
  void set_pbdata(const ::std::string& value);
  void set_pbdata(const char* value);
  void set_pbdata(const void* value, size_t size);
  ::std::string* mutable_pbdata();
  ::std::string* release_pbdata();
  void set_allocated_pbdata(::std::string* pbdata);

  // @@protoc_insertion_point(class_scope:nodeService.MsgPack)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr source_;
  ::google::protobuf::internal::ArenaStringPtr dest_;
  ::google::protobuf::internal::ArenaStringPtr msgname_;
  int msgtype_;
  ::google::protobuf::uint32 sessionid_;
  ::google::protobuf::internal::ArenaStringPtr pbdata_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_nodeService_2eproto();
  friend void protobuf_AssignDesc_nodeService_2eproto();
  friend void protobuf_ShutdownFile_nodeService_2eproto();

  void InitAsDefaultInstance();
  static MsgPack* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// ReqNodeRegistor

// -------------------------------------------------------------------

// RspNodeRegistor

// optional .nodeService.RegistorResultType status = 1;
inline void RspNodeRegistor::clear_status() {
  status_ = 0;
}
inline ::nodeService::RegistorResultType RspNodeRegistor::status() const {
  // @@protoc_insertion_point(field_get:nodeService.RspNodeRegistor.status)
  return static_cast< ::nodeService::RegistorResultType >(status_);
}
inline void RspNodeRegistor::set_status(::nodeService::RegistorResultType value) {
  
  status_ = value;
  // @@protoc_insertion_point(field_set:nodeService.RspNodeRegistor.status)
}

// -------------------------------------------------------------------

// MsgPack

// optional string source = 1;
inline void MsgPack::clear_source() {
  source_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& MsgPack::source() const {
  // @@protoc_insertion_point(field_get:nodeService.MsgPack.source)
  return source_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void MsgPack::set_source(const ::std::string& value) {
  
  source_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:nodeService.MsgPack.source)
}
inline void MsgPack::set_source(const char* value) {
  
  source_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:nodeService.MsgPack.source)
}
inline void MsgPack::set_source(const char* value, size_t size) {
  
  source_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:nodeService.MsgPack.source)
}
inline ::std::string* MsgPack::mutable_source() {
  
  // @@protoc_insertion_point(field_mutable:nodeService.MsgPack.source)
  return source_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* MsgPack::release_source() {
  // @@protoc_insertion_point(field_release:nodeService.MsgPack.source)
  
  return source_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void MsgPack::set_allocated_source(::std::string* source) {
  if (source != NULL) {
    
  } else {
    
  }
  source_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), source);
  // @@protoc_insertion_point(field_set_allocated:nodeService.MsgPack.source)
}

// optional string dest = 2;
inline void MsgPack::clear_dest() {
  dest_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& MsgPack::dest() const {
  // @@protoc_insertion_point(field_get:nodeService.MsgPack.dest)
  return dest_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void MsgPack::set_dest(const ::std::string& value) {
  
  dest_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:nodeService.MsgPack.dest)
}
inline void MsgPack::set_dest(const char* value) {
  
  dest_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:nodeService.MsgPack.dest)
}
inline void MsgPack::set_dest(const char* value, size_t size) {
  
  dest_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:nodeService.MsgPack.dest)
}
inline ::std::string* MsgPack::mutable_dest() {
  
  // @@protoc_insertion_point(field_mutable:nodeService.MsgPack.dest)
  return dest_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* MsgPack::release_dest() {
  // @@protoc_insertion_point(field_release:nodeService.MsgPack.dest)
  
  return dest_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void MsgPack::set_allocated_dest(::std::string* dest) {
  if (dest != NULL) {
    
  } else {
    
  }
  dest_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), dest);
  // @@protoc_insertion_point(field_set_allocated:nodeService.MsgPack.dest)
}

// optional string msgname = 3;
inline void MsgPack::clear_msgname() {
  msgname_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& MsgPack::msgname() const {
  // @@protoc_insertion_point(field_get:nodeService.MsgPack.msgname)
  return msgname_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void MsgPack::set_msgname(const ::std::string& value) {
  
  msgname_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:nodeService.MsgPack.msgname)
}
inline void MsgPack::set_msgname(const char* value) {
  
  msgname_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:nodeService.MsgPack.msgname)
}
inline void MsgPack::set_msgname(const char* value, size_t size) {
  
  msgname_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:nodeService.MsgPack.msgname)
}
inline ::std::string* MsgPack::mutable_msgname() {
  
  // @@protoc_insertion_point(field_mutable:nodeService.MsgPack.msgname)
  return msgname_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* MsgPack::release_msgname() {
  // @@protoc_insertion_point(field_release:nodeService.MsgPack.msgname)
  
  return msgname_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void MsgPack::set_allocated_msgname(::std::string* msgname) {
  if (msgname != NULL) {
    
  } else {
    
  }
  msgname_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), msgname);
  // @@protoc_insertion_point(field_set_allocated:nodeService.MsgPack.msgname)
}

// optional .nodeService.MsgType msgtype = 4;
inline void MsgPack::clear_msgtype() {
  msgtype_ = 0;
}
inline ::nodeService::MsgType MsgPack::msgtype() const {
  // @@protoc_insertion_point(field_get:nodeService.MsgPack.msgtype)
  return static_cast< ::nodeService::MsgType >(msgtype_);
}
inline void MsgPack::set_msgtype(::nodeService::MsgType value) {
  
  msgtype_ = value;
  // @@protoc_insertion_point(field_set:nodeService.MsgPack.msgtype)
}

// optional uint32 sessionid = 5;
inline void MsgPack::clear_sessionid() {
  sessionid_ = 0u;
}
inline ::google::protobuf::uint32 MsgPack::sessionid() const {
  // @@protoc_insertion_point(field_get:nodeService.MsgPack.sessionid)
  return sessionid_;
}
inline void MsgPack::set_sessionid(::google::protobuf::uint32 value) {
  
  sessionid_ = value;
  // @@protoc_insertion_point(field_set:nodeService.MsgPack.sessionid)
}

// optional bytes pbdata = 6;
inline void MsgPack::clear_pbdata() {
  pbdata_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& MsgPack::pbdata() const {
  // @@protoc_insertion_point(field_get:nodeService.MsgPack.pbdata)
  return pbdata_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void MsgPack::set_pbdata(const ::std::string& value) {
  
  pbdata_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:nodeService.MsgPack.pbdata)
}
inline void MsgPack::set_pbdata(const char* value) {
  
  pbdata_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:nodeService.MsgPack.pbdata)
}
inline void MsgPack::set_pbdata(const void* value, size_t size) {
  
  pbdata_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:nodeService.MsgPack.pbdata)
}
inline ::std::string* MsgPack::mutable_pbdata() {
  
  // @@protoc_insertion_point(field_mutable:nodeService.MsgPack.pbdata)
  return pbdata_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* MsgPack::release_pbdata() {
  // @@protoc_insertion_point(field_release:nodeService.MsgPack.pbdata)
  
  return pbdata_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void MsgPack::set_allocated_pbdata(::std::string* pbdata) {
  if (pbdata != NULL) {
    
  } else {
    
  }
  pbdata_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), pbdata);
  // @@protoc_insertion_point(field_set_allocated:nodeService.MsgPack.pbdata)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace nodeService

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::nodeService::RegistorResultType> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::nodeService::RegistorResultType>() {
  return ::nodeService::RegistorResultType_descriptor();
}
template <> struct is_proto_enum< ::nodeService::MsgType> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::nodeService::MsgType>() {
  return ::nodeService::MsgType_descriptor();
}

}  // namespace protobuf
}  // namespace google
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_nodeService_2eproto__INCLUDED
