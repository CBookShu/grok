// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: proto_test.proto

#ifndef PROTOBUF_proto_5ftest_2eproto__INCLUDED
#define PROTOBUF_proto_5ftest_2eproto__INCLUDED

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

namespace proto_test {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_proto_5ftest_2eproto();
void protobuf_AssignDesc_proto_5ftest_2eproto();
void protobuf_ShutdownFile_proto_5ftest_2eproto();

class Food;
class Player;
class Union;

// ===================================================================

class Union : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:proto_test.Union) */ {
 public:
  Union();
  virtual ~Union();

  Union(const Union& from);

  inline Union& operator=(const Union& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Union& default_instance();

  void Swap(Union* other);

  // implements Message ----------------------------------------------

  inline Union* New() const { return New(NULL); }

  Union* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Union& from);
  void MergeFrom(const Union& from);
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
  void InternalSwap(Union* other);
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

  // repeated .proto_test.Player players = 1;
  int players_size() const;
  void clear_players();
  static const int kPlayersFieldNumber = 1;
  const ::proto_test::Player& players(int index) const;
  ::proto_test::Player* mutable_players(int index);
  ::proto_test::Player* add_players();
  ::google::protobuf::RepeatedPtrField< ::proto_test::Player >*
      mutable_players();
  const ::google::protobuf::RepeatedPtrField< ::proto_test::Player >&
      players() const;

  // @@protoc_insertion_point(class_scope:proto_test.Union)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::RepeatedPtrField< ::proto_test::Player > players_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_proto_5ftest_2eproto();
  friend void protobuf_AssignDesc_proto_5ftest_2eproto();
  friend void protobuf_ShutdownFile_proto_5ftest_2eproto();

  void InitAsDefaultInstance();
  static Union* default_instance_;
};
// -------------------------------------------------------------------

class Player : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:proto_test.Player) */ {
 public:
  Player();
  virtual ~Player();

  Player(const Player& from);

  inline Player& operator=(const Player& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Player& default_instance();

  void Swap(Player* other);

  // implements Message ----------------------------------------------

  inline Player* New() const { return New(NULL); }

  Player* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Player& from);
  void MergeFrom(const Player& from);
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
  void InternalSwap(Player* other);
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

  // optional int32 age = 1;
  void clear_age();
  static const int kAgeFieldNumber = 1;
  ::google::protobuf::int32 age() const;
  void set_age(::google::protobuf::int32 value);

  // optional int32 male = 2;
  void clear_male();
  static const int kMaleFieldNumber = 2;
  ::google::protobuf::int32 male() const;
  void set_male(::google::protobuf::int32 value);

  // optional string name = 3;
  void clear_name();
  static const int kNameFieldNumber = 3;
  const ::std::string& name() const;
  void set_name(const ::std::string& value);
  void set_name(const char* value);
  void set_name(const char* value, size_t size);
  ::std::string* mutable_name();
  ::std::string* release_name();
  void set_allocated_name(::std::string* name);

  // optional .proto_test.Food food = 4;
  bool has_food() const;
  void clear_food();
  static const int kFoodFieldNumber = 4;
  const ::proto_test::Food& food() const;
  ::proto_test::Food* mutable_food();
  ::proto_test::Food* release_food();
  void set_allocated_food(::proto_test::Food* food);

  // @@protoc_insertion_point(class_scope:proto_test.Player)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::int32 age_;
  ::google::protobuf::int32 male_;
  ::google::protobuf::internal::ArenaStringPtr name_;
  ::proto_test::Food* food_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_proto_5ftest_2eproto();
  friend void protobuf_AssignDesc_proto_5ftest_2eproto();
  friend void protobuf_ShutdownFile_proto_5ftest_2eproto();

  void InitAsDefaultInstance();
  static Player* default_instance_;
};
// -------------------------------------------------------------------

class Food : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:proto_test.Food) */ {
 public:
  Food();
  virtual ~Food();

  Food(const Food& from);

  inline Food& operator=(const Food& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Food& default_instance();

  void Swap(Food* other);

  // implements Message ----------------------------------------------

  inline Food* New() const { return New(NULL); }

  Food* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Food& from);
  void MergeFrom(const Food& from);
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
  void InternalSwap(Food* other);
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

  // optional string name = 1;
  void clear_name();
  static const int kNameFieldNumber = 1;
  const ::std::string& name() const;
  void set_name(const ::std::string& value);
  void set_name(const char* value);
  void set_name(const char* value, size_t size);
  ::std::string* mutable_name();
  ::std::string* release_name();
  void set_allocated_name(::std::string* name);

  // @@protoc_insertion_point(class_scope:proto_test.Food)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr name_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_proto_5ftest_2eproto();
  friend void protobuf_AssignDesc_proto_5ftest_2eproto();
  friend void protobuf_ShutdownFile_proto_5ftest_2eproto();

  void InitAsDefaultInstance();
  static Food* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// Union

// repeated .proto_test.Player players = 1;
inline int Union::players_size() const {
  return players_.size();
}
inline void Union::clear_players() {
  players_.Clear();
}
inline const ::proto_test::Player& Union::players(int index) const {
  // @@protoc_insertion_point(field_get:proto_test.Union.players)
  return players_.Get(index);
}
inline ::proto_test::Player* Union::mutable_players(int index) {
  // @@protoc_insertion_point(field_mutable:proto_test.Union.players)
  return players_.Mutable(index);
}
inline ::proto_test::Player* Union::add_players() {
  // @@protoc_insertion_point(field_add:proto_test.Union.players)
  return players_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::proto_test::Player >*
Union::mutable_players() {
  // @@protoc_insertion_point(field_mutable_list:proto_test.Union.players)
  return &players_;
}
inline const ::google::protobuf::RepeatedPtrField< ::proto_test::Player >&
Union::players() const {
  // @@protoc_insertion_point(field_list:proto_test.Union.players)
  return players_;
}

// -------------------------------------------------------------------

// Player

// optional int32 age = 1;
inline void Player::clear_age() {
  age_ = 0;
}
inline ::google::protobuf::int32 Player::age() const {
  // @@protoc_insertion_point(field_get:proto_test.Player.age)
  return age_;
}
inline void Player::set_age(::google::protobuf::int32 value) {
  
  age_ = value;
  // @@protoc_insertion_point(field_set:proto_test.Player.age)
}

// optional int32 male = 2;
inline void Player::clear_male() {
  male_ = 0;
}
inline ::google::protobuf::int32 Player::male() const {
  // @@protoc_insertion_point(field_get:proto_test.Player.male)
  return male_;
}
inline void Player::set_male(::google::protobuf::int32 value) {
  
  male_ = value;
  // @@protoc_insertion_point(field_set:proto_test.Player.male)
}

// optional string name = 3;
inline void Player::clear_name() {
  name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Player::name() const {
  // @@protoc_insertion_point(field_get:proto_test.Player.name)
  return name_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Player::set_name(const ::std::string& value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:proto_test.Player.name)
}
inline void Player::set_name(const char* value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:proto_test.Player.name)
}
inline void Player::set_name(const char* value, size_t size) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:proto_test.Player.name)
}
inline ::std::string* Player::mutable_name() {
  
  // @@protoc_insertion_point(field_mutable:proto_test.Player.name)
  return name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Player::release_name() {
  // @@protoc_insertion_point(field_release:proto_test.Player.name)
  
  return name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Player::set_allocated_name(::std::string* name) {
  if (name != NULL) {
    
  } else {
    
  }
  name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), name);
  // @@protoc_insertion_point(field_set_allocated:proto_test.Player.name)
}

// optional .proto_test.Food food = 4;
inline bool Player::has_food() const {
  return !_is_default_instance_ && food_ != NULL;
}
inline void Player::clear_food() {
  if (GetArenaNoVirtual() == NULL && food_ != NULL) delete food_;
  food_ = NULL;
}
inline const ::proto_test::Food& Player::food() const {
  // @@protoc_insertion_point(field_get:proto_test.Player.food)
  return food_ != NULL ? *food_ : *default_instance_->food_;
}
inline ::proto_test::Food* Player::mutable_food() {
  
  if (food_ == NULL) {
    food_ = new ::proto_test::Food;
  }
  // @@protoc_insertion_point(field_mutable:proto_test.Player.food)
  return food_;
}
inline ::proto_test::Food* Player::release_food() {
  // @@protoc_insertion_point(field_release:proto_test.Player.food)
  
  ::proto_test::Food* temp = food_;
  food_ = NULL;
  return temp;
}
inline void Player::set_allocated_food(::proto_test::Food* food) {
  delete food_;
  food_ = food;
  if (food) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_set_allocated:proto_test.Player.food)
}

// -------------------------------------------------------------------

// Food

// optional string name = 1;
inline void Food::clear_name() {
  name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Food::name() const {
  // @@protoc_insertion_point(field_get:proto_test.Food.name)
  return name_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Food::set_name(const ::std::string& value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:proto_test.Food.name)
}
inline void Food::set_name(const char* value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:proto_test.Food.name)
}
inline void Food::set_name(const char* value, size_t size) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:proto_test.Food.name)
}
inline ::std::string* Food::mutable_name() {
  
  // @@protoc_insertion_point(field_mutable:proto_test.Food.name)
  return name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Food::release_name() {
  // @@protoc_insertion_point(field_release:proto_test.Food.name)
  
  return name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Food::set_allocated_name(::std::string* name) {
  if (name != NULL) {
    
  } else {
    
  }
  name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), name);
  // @@protoc_insertion_point(field_set_allocated:proto_test.Food.name)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace proto_test

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_proto_5ftest_2eproto__INCLUDED
