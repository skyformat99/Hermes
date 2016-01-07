// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: Communication.proto

#ifndef PROTOBUF_Communication_2eproto__INCLUDED
#define PROTOBUF_Communication_2eproto__INCLUDED

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

namespace com {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_Communication_2eproto();
void protobuf_AssignDesc_Communication_2eproto();
void protobuf_ShutdownFile_Communication_2eproto();

class Communication;
class Message;

// ===================================================================

class Message : public ::google::protobuf::Message {
 public:
  Message();
  virtual ~Message();

  Message(const Message& from);

  inline Message& operator=(const Message& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Message& default_instance();

  void Swap(Message* other);

  // implements Message ----------------------------------------------

  inline Message* New() const { return New(NULL); }

  Message* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Message& from);
  void MergeFrom(const Message& from);
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
  void InternalSwap(Message* other);
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

  // optional int32 id = 1;
  void clear_id();
  static const int kIdFieldNumber = 1;
  ::google::protobuf::int32 id() const;
  void set_id(::google::protobuf::int32 value);

  // optional string name = 2;
  void clear_name();
  static const int kNameFieldNumber = 2;
  const ::std::string& name() const;
  void set_name(const ::std::string& value);
  void set_name(const char* value);
  void set_name(const char* value, size_t size);
  ::std::string* mutable_name();
  ::std::string* release_name();
  void set_allocated_name(::std::string* name);

  // optional string object = 3;
  void clear_object();
  static const int kObjectFieldNumber = 3;
  const ::std::string& object() const;
  void set_object(const ::std::string& value);
  void set_object(const char* value);
  void set_object(const char* value, size_t size);
  ::std::string* mutable_object();
  ::std::string* release_object();
  void set_allocated_object(::std::string* object);

  // optional string from = 4;
  void clear_from();
  static const int kFromFieldNumber = 4;
  const ::std::string& from() const;
  void set_from(const ::std::string& value);
  void set_from(const char* value);
  void set_from(const char* value, size_t size);
  ::std::string* mutable_from();
  ::std::string* release_from();
  void set_allocated_from(::std::string* from);

  // optional string to = 5;
  void clear_to();
  static const int kToFieldNumber = 5;
  const ::std::string& to() const;
  void set_to(const ::std::string& value);
  void set_to(const char* value);
  void set_to(const char* value, size_t size);
  ::std::string* mutable_to();
  ::std::string* release_to();
  void set_allocated_to(::std::string* to);

  // optional string msg = 6;
  void clear_msg();
  static const int kMsgFieldNumber = 6;
  const ::std::string& msg() const;
  void set_msg(const ::std::string& value);
  void set_msg(const char* value);
  void set_msg(const char* value, size_t size);
  ::std::string* mutable_msg();
  ::std::string* release_msg();
  void set_allocated_msg(::std::string* msg);

  // @@protoc_insertion_point(class_scope:com.Message)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr name_;
  ::google::protobuf::internal::ArenaStringPtr object_;
  ::google::protobuf::internal::ArenaStringPtr from_;
  ::google::protobuf::internal::ArenaStringPtr to_;
  ::google::protobuf::internal::ArenaStringPtr msg_;
  ::google::protobuf::int32 id_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_Communication_2eproto();
  friend void protobuf_AssignDesc_Communication_2eproto();
  friend void protobuf_ShutdownFile_Communication_2eproto();

  void InitAsDefaultInstance();
  static Message* default_instance_;
};
// -------------------------------------------------------------------

class Communication : public ::google::protobuf::Message {
 public:
  Communication();
  virtual ~Communication();

  Communication(const Communication& from);

  inline Communication& operator=(const Communication& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Communication& default_instance();

  void Swap(Communication* other);

  // implements Message ----------------------------------------------

  inline Communication* New() const { return New(NULL); }

  Communication* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Communication& from);
  void MergeFrom(const Communication& from);
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
  void InternalSwap(Communication* other);
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

  // repeated .com.Message message = 1;
  int message_size() const;
  void clear_message();
  static const int kMessageFieldNumber = 1;
  const ::com::Message& message(int index) const;
  ::com::Message* mutable_message(int index);
  ::com::Message* add_message();
  ::google::protobuf::RepeatedPtrField< ::com::Message >*
      mutable_message();
  const ::google::protobuf::RepeatedPtrField< ::com::Message >&
      message() const;

  // @@protoc_insertion_point(class_scope:com.Communication)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::RepeatedPtrField< ::com::Message > message_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_Communication_2eproto();
  friend void protobuf_AssignDesc_Communication_2eproto();
  friend void protobuf_ShutdownFile_Communication_2eproto();

  void InitAsDefaultInstance();
  static Communication* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// Message

// optional int32 id = 1;
inline void Message::clear_id() {
  id_ = 0;
}
inline ::google::protobuf::int32 Message::id() const {
  // @@protoc_insertion_point(field_get:com.Message.id)
  return id_;
}
inline void Message::set_id(::google::protobuf::int32 value) {
  
  id_ = value;
  // @@protoc_insertion_point(field_set:com.Message.id)
}

// optional string name = 2;
inline void Message::clear_name() {
  name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Message::name() const {
  // @@protoc_insertion_point(field_get:com.Message.name)
  return name_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Message::set_name(const ::std::string& value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:com.Message.name)
}
inline void Message::set_name(const char* value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:com.Message.name)
}
inline void Message::set_name(const char* value, size_t size) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:com.Message.name)
}
inline ::std::string* Message::mutable_name() {
  
  // @@protoc_insertion_point(field_mutable:com.Message.name)
  return name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Message::release_name() {
  
  return name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Message::set_allocated_name(::std::string* name) {
  if (name != NULL) {
    
  } else {
    
  }
  name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), name);
  // @@protoc_insertion_point(field_set_allocated:com.Message.name)
}

// optional string object = 3;
inline void Message::clear_object() {
  object_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Message::object() const {
  // @@protoc_insertion_point(field_get:com.Message.object)
  return object_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Message::set_object(const ::std::string& value) {
  
  object_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:com.Message.object)
}
inline void Message::set_object(const char* value) {
  
  object_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:com.Message.object)
}
inline void Message::set_object(const char* value, size_t size) {
  
  object_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:com.Message.object)
}
inline ::std::string* Message::mutable_object() {
  
  // @@protoc_insertion_point(field_mutable:com.Message.object)
  return object_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Message::release_object() {
  
  return object_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Message::set_allocated_object(::std::string* object) {
  if (object != NULL) {
    
  } else {
    
  }
  object_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), object);
  // @@protoc_insertion_point(field_set_allocated:com.Message.object)
}

// optional string from = 4;
inline void Message::clear_from() {
  from_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Message::from() const {
  // @@protoc_insertion_point(field_get:com.Message.from)
  return from_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Message::set_from(const ::std::string& value) {
  
  from_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:com.Message.from)
}
inline void Message::set_from(const char* value) {
  
  from_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:com.Message.from)
}
inline void Message::set_from(const char* value, size_t size) {
  
  from_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:com.Message.from)
}
inline ::std::string* Message::mutable_from() {
  
  // @@protoc_insertion_point(field_mutable:com.Message.from)
  return from_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Message::release_from() {
  
  return from_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Message::set_allocated_from(::std::string* from) {
  if (from != NULL) {
    
  } else {
    
  }
  from_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from);
  // @@protoc_insertion_point(field_set_allocated:com.Message.from)
}

// optional string to = 5;
inline void Message::clear_to() {
  to_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Message::to() const {
  // @@protoc_insertion_point(field_get:com.Message.to)
  return to_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Message::set_to(const ::std::string& value) {
  
  to_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:com.Message.to)
}
inline void Message::set_to(const char* value) {
  
  to_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:com.Message.to)
}
inline void Message::set_to(const char* value, size_t size) {
  
  to_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:com.Message.to)
}
inline ::std::string* Message::mutable_to() {
  
  // @@protoc_insertion_point(field_mutable:com.Message.to)
  return to_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Message::release_to() {
  
  return to_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Message::set_allocated_to(::std::string* to) {
  if (to != NULL) {
    
  } else {
    
  }
  to_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), to);
  // @@protoc_insertion_point(field_set_allocated:com.Message.to)
}

// optional string msg = 6;
inline void Message::clear_msg() {
  msg_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Message::msg() const {
  // @@protoc_insertion_point(field_get:com.Message.msg)
  return msg_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Message::set_msg(const ::std::string& value) {
  
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:com.Message.msg)
}
inline void Message::set_msg(const char* value) {
  
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:com.Message.msg)
}
inline void Message::set_msg(const char* value, size_t size) {
  
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:com.Message.msg)
}
inline ::std::string* Message::mutable_msg() {
  
  // @@protoc_insertion_point(field_mutable:com.Message.msg)
  return msg_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Message::release_msg() {
  
  return msg_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Message::set_allocated_msg(::std::string* msg) {
  if (msg != NULL) {
    
  } else {
    
  }
  msg_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), msg);
  // @@protoc_insertion_point(field_set_allocated:com.Message.msg)
}

// -------------------------------------------------------------------

// Communication

// repeated .com.Message message = 1;
inline int Communication::message_size() const {
  return message_.size();
}
inline void Communication::clear_message() {
  message_.Clear();
}
inline const ::com::Message& Communication::message(int index) const {
  // @@protoc_insertion_point(field_get:com.Communication.message)
  return message_.Get(index);
}
inline ::com::Message* Communication::mutable_message(int index) {
  // @@protoc_insertion_point(field_mutable:com.Communication.message)
  return message_.Mutable(index);
}
inline ::com::Message* Communication::add_message() {
  // @@protoc_insertion_point(field_add:com.Communication.message)
  return message_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::com::Message >*
Communication::mutable_message() {
  // @@protoc_insertion_point(field_mutable_list:com.Communication.message)
  return &message_;
}
inline const ::google::protobuf::RepeatedPtrField< ::com::Message >&
Communication::message() const {
  // @@protoc_insertion_point(field_list:com.Communication.message)
  return message_;
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace com

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_Communication_2eproto__INCLUDED
