// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: nodemsg_test.proto

#ifndef PROTOBUF_nodemsg_5ftest_2eproto__INCLUDED
#define PROTOBUF_nodemsg_5ftest_2eproto__INCLUDED

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
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace nodemsg_test {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_nodemsg_5ftest_2eproto();
void protobuf_AssignDesc_nodemsg_5ftest_2eproto();
void protobuf_ShutdownFile_nodemsg_5ftest_2eproto();

class TestReq1;
class TestRsp1;

// ===================================================================

class TestReq1 : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:nodemsg_test.TestReq1) */ {
 public:
  TestReq1();
  virtual ~TestReq1();

  TestReq1(const TestReq1& from);

  inline TestReq1& operator=(const TestReq1& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const TestReq1& default_instance();

  void Swap(TestReq1* other);

  // implements Message ----------------------------------------------

  inline TestReq1* New() const { return New(NULL); }

  TestReq1* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const TestReq1& from);
  void MergeFrom(const TestReq1& from);
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
  void InternalSwap(TestReq1* other);
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

  // optional string req = 1;
  void clear_req();
  static const int kReqFieldNumber = 1;
  const ::std::string& req() const;
  void set_req(const ::std::string& value);
  void set_req(const char* value);
  void set_req(const char* value, size_t size);
  ::std::string* mutable_req();
  ::std::string* release_req();
  void set_allocated_req(::std::string* req);

  // @@protoc_insertion_point(class_scope:nodemsg_test.TestReq1)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr req_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_nodemsg_5ftest_2eproto();
  friend void protobuf_AssignDesc_nodemsg_5ftest_2eproto();
  friend void protobuf_ShutdownFile_nodemsg_5ftest_2eproto();

  void InitAsDefaultInstance();
  static TestReq1* default_instance_;
};
// -------------------------------------------------------------------

class TestRsp1 : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:nodemsg_test.TestRsp1) */ {
 public:
  TestRsp1();
  virtual ~TestRsp1();

  TestRsp1(const TestRsp1& from);

  inline TestRsp1& operator=(const TestRsp1& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const TestRsp1& default_instance();

  void Swap(TestRsp1* other);

  // implements Message ----------------------------------------------

  inline TestRsp1* New() const { return New(NULL); }

  TestRsp1* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const TestRsp1& from);
  void MergeFrom(const TestRsp1& from);
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
  void InternalSwap(TestRsp1* other);
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

  // optional int32 status = 1;
  void clear_status();
  static const int kStatusFieldNumber = 1;
  ::google::protobuf::int32 status() const;
  void set_status(::google::protobuf::int32 value);

  // optional string rsp = 2;
  void clear_rsp();
  static const int kRspFieldNumber = 2;
  const ::std::string& rsp() const;
  void set_rsp(const ::std::string& value);
  void set_rsp(const char* value);
  void set_rsp(const char* value, size_t size);
  ::std::string* mutable_rsp();
  ::std::string* release_rsp();
  void set_allocated_rsp(::std::string* rsp);

  // @@protoc_insertion_point(class_scope:nodemsg_test.TestRsp1)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr rsp_;
  ::google::protobuf::int32 status_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_nodemsg_5ftest_2eproto();
  friend void protobuf_AssignDesc_nodemsg_5ftest_2eproto();
  friend void protobuf_ShutdownFile_nodemsg_5ftest_2eproto();

  void InitAsDefaultInstance();
  static TestRsp1* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// TestReq1

// optional string req = 1;
inline void TestReq1::clear_req() {
  req_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& TestReq1::req() const {
  // @@protoc_insertion_point(field_get:nodemsg_test.TestReq1.req)
  return req_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void TestReq1::set_req(const ::std::string& value) {
  
  req_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:nodemsg_test.TestReq1.req)
}
inline void TestReq1::set_req(const char* value) {
  
  req_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:nodemsg_test.TestReq1.req)
}
inline void TestReq1::set_req(const char* value, size_t size) {
  
  req_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:nodemsg_test.TestReq1.req)
}
inline ::std::string* TestReq1::mutable_req() {
  
  // @@protoc_insertion_point(field_mutable:nodemsg_test.TestReq1.req)
  return req_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* TestReq1::release_req() {
  // @@protoc_insertion_point(field_release:nodemsg_test.TestReq1.req)
  
  return req_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void TestReq1::set_allocated_req(::std::string* req) {
  if (req != NULL) {
    
  } else {
    
  }
  req_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), req);
  // @@protoc_insertion_point(field_set_allocated:nodemsg_test.TestReq1.req)
}

// -------------------------------------------------------------------

// TestRsp1

// optional int32 status = 1;
inline void TestRsp1::clear_status() {
  status_ = 0;
}
inline ::google::protobuf::int32 TestRsp1::status() const {
  // @@protoc_insertion_point(field_get:nodemsg_test.TestRsp1.status)
  return status_;
}
inline void TestRsp1::set_status(::google::protobuf::int32 value) {
  
  status_ = value;
  // @@protoc_insertion_point(field_set:nodemsg_test.TestRsp1.status)
}

// optional string rsp = 2;
inline void TestRsp1::clear_rsp() {
  rsp_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& TestRsp1::rsp() const {
  // @@protoc_insertion_point(field_get:nodemsg_test.TestRsp1.rsp)
  return rsp_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void TestRsp1::set_rsp(const ::std::string& value) {
  
  rsp_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:nodemsg_test.TestRsp1.rsp)
}
inline void TestRsp1::set_rsp(const char* value) {
  
  rsp_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:nodemsg_test.TestRsp1.rsp)
}
inline void TestRsp1::set_rsp(const char* value, size_t size) {
  
  rsp_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:nodemsg_test.TestRsp1.rsp)
}
inline ::std::string* TestRsp1::mutable_rsp() {
  
  // @@protoc_insertion_point(field_mutable:nodemsg_test.TestRsp1.rsp)
  return rsp_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* TestRsp1::release_rsp() {
  // @@protoc_insertion_point(field_release:nodemsg_test.TestRsp1.rsp)
  
  return rsp_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void TestRsp1::set_allocated_rsp(::std::string* rsp) {
  if (rsp != NULL) {
    
  } else {
    
  }
  rsp_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), rsp);
  // @@protoc_insertion_point(field_set_allocated:nodemsg_test.TestRsp1.rsp)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace nodemsg_test

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_nodemsg_5ftest_2eproto__INCLUDED
