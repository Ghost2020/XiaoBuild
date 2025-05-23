// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: exception.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_exception_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_exception_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3018000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3018000 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_exception_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_exception_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxiliaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[1]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_exception_2eproto;
class FException;
struct FExceptionDefaultTypeInternal;
extern FExceptionDefaultTypeInternal _FException_default_instance_;
PROTOBUF_NAMESPACE_OPEN
template<> ::FException* Arena::CreateMaybeMessage<::FException>(Arena*);
PROTOBUF_NAMESPACE_CLOSE

// ===================================================================

class FException final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:FException) */ {
 public:
  inline FException() : FException(nullptr) {}
  ~FException() override;
  explicit constexpr FException(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  FException(const FException& from);
  FException(FException&& from) noexcept
    : FException() {
    *this = ::std::move(from);
  }

  inline FException& operator=(const FException& from) {
    CopyFrom(from);
    return *this;
  }
  inline FException& operator=(FException&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const FException& default_instance() {
    return *internal_default_instance();
  }
  static inline const FException* internal_default_instance() {
    return reinterpret_cast<const FException*>(
               &_FException_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(FException& a, FException& b) {
    a.Swap(&b);
  }
  inline void Swap(FException* other) {
    if (other == this) return;
    if (GetOwningArena() == other->GetOwningArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(FException* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline FException* New() const final {
    return new FException();
  }

  FException* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<FException>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const FException& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom(const FException& from);
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message* to, const ::PROTOBUF_NAMESPACE_ID::Message& from);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(FException* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "FException";
  }
  protected:
  explicit FException(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kTimeStampFieldNumber = 1,
    kBuildIdFieldNumber = 2,
    kExecHostFieldNumber = 3,
    kMacFieldNumber = 4,
    kApplicationFieldNumber = 6,
    kArgumentsFieldNumber = 7,
    kWorkingDirFieldNumber = 8,
    kDescriptionFieldNumber = 9,
    kLogFileFieldNumber = 10,
    kExitCodeFieldNumber = 5,
  };
  // string TimeStamp = 1;
  void clear_timestamp();
  const std::string& timestamp() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_timestamp(ArgT0&& arg0, ArgT... args);
  std::string* mutable_timestamp();
  PROTOBUF_MUST_USE_RESULT std::string* release_timestamp();
  void set_allocated_timestamp(std::string* timestamp);
  private:
  const std::string& _internal_timestamp() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_timestamp(const std::string& value);
  std::string* _internal_mutable_timestamp();
  public:

  // string BuildId = 2;
  void clear_buildid();
  const std::string& buildid() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_buildid(ArgT0&& arg0, ArgT... args);
  std::string* mutable_buildid();
  PROTOBUF_MUST_USE_RESULT std::string* release_buildid();
  void set_allocated_buildid(std::string* buildid);
  private:
  const std::string& _internal_buildid() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_buildid(const std::string& value);
  std::string* _internal_mutable_buildid();
  public:

  // string ExecHost = 3;
  void clear_exechost();
  const std::string& exechost() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_exechost(ArgT0&& arg0, ArgT... args);
  std::string* mutable_exechost();
  PROTOBUF_MUST_USE_RESULT std::string* release_exechost();
  void set_allocated_exechost(std::string* exechost);
  private:
  const std::string& _internal_exechost() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_exechost(const std::string& value);
  std::string* _internal_mutable_exechost();
  public:

  // string Mac = 4;
  void clear_mac();
  const std::string& mac() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_mac(ArgT0&& arg0, ArgT... args);
  std::string* mutable_mac();
  PROTOBUF_MUST_USE_RESULT std::string* release_mac();
  void set_allocated_mac(std::string* mac);
  private:
  const std::string& _internal_mac() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_mac(const std::string& value);
  std::string* _internal_mutable_mac();
  public:

  // string Application = 6;
  void clear_application();
  const std::string& application() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_application(ArgT0&& arg0, ArgT... args);
  std::string* mutable_application();
  PROTOBUF_MUST_USE_RESULT std::string* release_application();
  void set_allocated_application(std::string* application);
  private:
  const std::string& _internal_application() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_application(const std::string& value);
  std::string* _internal_mutable_application();
  public:

  // string Arguments = 7;
  void clear_arguments();
  const std::string& arguments() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_arguments(ArgT0&& arg0, ArgT... args);
  std::string* mutable_arguments();
  PROTOBUF_MUST_USE_RESULT std::string* release_arguments();
  void set_allocated_arguments(std::string* arguments);
  private:
  const std::string& _internal_arguments() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_arguments(const std::string& value);
  std::string* _internal_mutable_arguments();
  public:

  // string WorkingDir = 8;
  void clear_workingdir();
  const std::string& workingdir() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_workingdir(ArgT0&& arg0, ArgT... args);
  std::string* mutable_workingdir();
  PROTOBUF_MUST_USE_RESULT std::string* release_workingdir();
  void set_allocated_workingdir(std::string* workingdir);
  private:
  const std::string& _internal_workingdir() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_workingdir(const std::string& value);
  std::string* _internal_mutable_workingdir();
  public:

  // string Description = 9;
  void clear_description();
  const std::string& description() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_description(ArgT0&& arg0, ArgT... args);
  std::string* mutable_description();
  PROTOBUF_MUST_USE_RESULT std::string* release_description();
  void set_allocated_description(std::string* description);
  private:
  const std::string& _internal_description() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_description(const std::string& value);
  std::string* _internal_mutable_description();
  public:

  // string LogFile = 10;
  void clear_logfile();
  const std::string& logfile() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_logfile(ArgT0&& arg0, ArgT... args);
  std::string* mutable_logfile();
  PROTOBUF_MUST_USE_RESULT std::string* release_logfile();
  void set_allocated_logfile(std::string* logfile);
  private:
  const std::string& _internal_logfile() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_logfile(const std::string& value);
  std::string* _internal_mutable_logfile();
  public:

  // int32 ExitCode = 5;
  void clear_exitcode();
  ::PROTOBUF_NAMESPACE_ID::int32 exitcode() const;
  void set_exitcode(::PROTOBUF_NAMESPACE_ID::int32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int32 _internal_exitcode() const;
  void _internal_set_exitcode(::PROTOBUF_NAMESPACE_ID::int32 value);
  public:

  // @@protoc_insertion_point(class_scope:FException)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr timestamp_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr buildid_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr exechost_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr mac_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr application_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr arguments_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr workingdir_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr description_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr logfile_;
  ::PROTOBUF_NAMESPACE_ID::int32 exitcode_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_exception_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// FException

// string TimeStamp = 1;
inline void FException::clear_timestamp() {
  timestamp_.ClearToEmpty();
}
inline const std::string& FException::timestamp() const {
  // @@protoc_insertion_point(field_get:FException.TimeStamp)
  return _internal_timestamp();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void FException::set_timestamp(ArgT0&& arg0, ArgT... args) {
 
 timestamp_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:FException.TimeStamp)
}
inline std::string* FException::mutable_timestamp() {
  std::string* _s = _internal_mutable_timestamp();
  // @@protoc_insertion_point(field_mutable:FException.TimeStamp)
  return _s;
}
inline const std::string& FException::_internal_timestamp() const {
  return timestamp_.Get();
}
inline void FException::_internal_set_timestamp(const std::string& value) {
  
  timestamp_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* FException::_internal_mutable_timestamp() {
  
  return timestamp_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* FException::release_timestamp() {
  // @@protoc_insertion_point(field_release:FException.TimeStamp)
  return timestamp_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void FException::set_allocated_timestamp(std::string* timestamp) {
  if (timestamp != nullptr) {
    
  } else {
    
  }
  timestamp_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), timestamp,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:FException.TimeStamp)
}

// string BuildId = 2;
inline void FException::clear_buildid() {
  buildid_.ClearToEmpty();
}
inline const std::string& FException::buildid() const {
  // @@protoc_insertion_point(field_get:FException.BuildId)
  return _internal_buildid();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void FException::set_buildid(ArgT0&& arg0, ArgT... args) {
 
 buildid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:FException.BuildId)
}
inline std::string* FException::mutable_buildid() {
  std::string* _s = _internal_mutable_buildid();
  // @@protoc_insertion_point(field_mutable:FException.BuildId)
  return _s;
}
inline const std::string& FException::_internal_buildid() const {
  return buildid_.Get();
}
inline void FException::_internal_set_buildid(const std::string& value) {
  
  buildid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* FException::_internal_mutable_buildid() {
  
  return buildid_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* FException::release_buildid() {
  // @@protoc_insertion_point(field_release:FException.BuildId)
  return buildid_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void FException::set_allocated_buildid(std::string* buildid) {
  if (buildid != nullptr) {
    
  } else {
    
  }
  buildid_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), buildid,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:FException.BuildId)
}

// string ExecHost = 3;
inline void FException::clear_exechost() {
  exechost_.ClearToEmpty();
}
inline const std::string& FException::exechost() const {
  // @@protoc_insertion_point(field_get:FException.ExecHost)
  return _internal_exechost();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void FException::set_exechost(ArgT0&& arg0, ArgT... args) {
 
 exechost_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:FException.ExecHost)
}
inline std::string* FException::mutable_exechost() {
  std::string* _s = _internal_mutable_exechost();
  // @@protoc_insertion_point(field_mutable:FException.ExecHost)
  return _s;
}
inline const std::string& FException::_internal_exechost() const {
  return exechost_.Get();
}
inline void FException::_internal_set_exechost(const std::string& value) {
  
  exechost_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* FException::_internal_mutable_exechost() {
  
  return exechost_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* FException::release_exechost() {
  // @@protoc_insertion_point(field_release:FException.ExecHost)
  return exechost_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void FException::set_allocated_exechost(std::string* exechost) {
  if (exechost != nullptr) {
    
  } else {
    
  }
  exechost_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), exechost,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:FException.ExecHost)
}

// string Mac = 4;
inline void FException::clear_mac() {
  mac_.ClearToEmpty();
}
inline const std::string& FException::mac() const {
  // @@protoc_insertion_point(field_get:FException.Mac)
  return _internal_mac();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void FException::set_mac(ArgT0&& arg0, ArgT... args) {
 
 mac_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:FException.Mac)
}
inline std::string* FException::mutable_mac() {
  std::string* _s = _internal_mutable_mac();
  // @@protoc_insertion_point(field_mutable:FException.Mac)
  return _s;
}
inline const std::string& FException::_internal_mac() const {
  return mac_.Get();
}
inline void FException::_internal_set_mac(const std::string& value) {
  
  mac_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* FException::_internal_mutable_mac() {
  
  return mac_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* FException::release_mac() {
  // @@protoc_insertion_point(field_release:FException.Mac)
  return mac_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void FException::set_allocated_mac(std::string* mac) {
  if (mac != nullptr) {
    
  } else {
    
  }
  mac_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), mac,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:FException.Mac)
}

// int32 ExitCode = 5;
inline void FException::clear_exitcode() {
  exitcode_ = 0;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 FException::_internal_exitcode() const {
  return exitcode_;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 FException::exitcode() const {
  // @@protoc_insertion_point(field_get:FException.ExitCode)
  return _internal_exitcode();
}
inline void FException::_internal_set_exitcode(::PROTOBUF_NAMESPACE_ID::int32 value) {
  
  exitcode_ = value;
}
inline void FException::set_exitcode(::PROTOBUF_NAMESPACE_ID::int32 value) {
  _internal_set_exitcode(value);
  // @@protoc_insertion_point(field_set:FException.ExitCode)
}

// string Application = 6;
inline void FException::clear_application() {
  application_.ClearToEmpty();
}
inline const std::string& FException::application() const {
  // @@protoc_insertion_point(field_get:FException.Application)
  return _internal_application();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void FException::set_application(ArgT0&& arg0, ArgT... args) {
 
 application_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:FException.Application)
}
inline std::string* FException::mutable_application() {
  std::string* _s = _internal_mutable_application();
  // @@protoc_insertion_point(field_mutable:FException.Application)
  return _s;
}
inline const std::string& FException::_internal_application() const {
  return application_.Get();
}
inline void FException::_internal_set_application(const std::string& value) {
  
  application_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* FException::_internal_mutable_application() {
  
  return application_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* FException::release_application() {
  // @@protoc_insertion_point(field_release:FException.Application)
  return application_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void FException::set_allocated_application(std::string* application) {
  if (application != nullptr) {
    
  } else {
    
  }
  application_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), application,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:FException.Application)
}

// string Arguments = 7;
inline void FException::clear_arguments() {
  arguments_.ClearToEmpty();
}
inline const std::string& FException::arguments() const {
  // @@protoc_insertion_point(field_get:FException.Arguments)
  return _internal_arguments();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void FException::set_arguments(ArgT0&& arg0, ArgT... args) {
 
 arguments_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:FException.Arguments)
}
inline std::string* FException::mutable_arguments() {
  std::string* _s = _internal_mutable_arguments();
  // @@protoc_insertion_point(field_mutable:FException.Arguments)
  return _s;
}
inline const std::string& FException::_internal_arguments() const {
  return arguments_.Get();
}
inline void FException::_internal_set_arguments(const std::string& value) {
  
  arguments_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* FException::_internal_mutable_arguments() {
  
  return arguments_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* FException::release_arguments() {
  // @@protoc_insertion_point(field_release:FException.Arguments)
  return arguments_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void FException::set_allocated_arguments(std::string* arguments) {
  if (arguments != nullptr) {
    
  } else {
    
  }
  arguments_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), arguments,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:FException.Arguments)
}

// string WorkingDir = 8;
inline void FException::clear_workingdir() {
  workingdir_.ClearToEmpty();
}
inline const std::string& FException::workingdir() const {
  // @@protoc_insertion_point(field_get:FException.WorkingDir)
  return _internal_workingdir();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void FException::set_workingdir(ArgT0&& arg0, ArgT... args) {
 
 workingdir_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:FException.WorkingDir)
}
inline std::string* FException::mutable_workingdir() {
  std::string* _s = _internal_mutable_workingdir();
  // @@protoc_insertion_point(field_mutable:FException.WorkingDir)
  return _s;
}
inline const std::string& FException::_internal_workingdir() const {
  return workingdir_.Get();
}
inline void FException::_internal_set_workingdir(const std::string& value) {
  
  workingdir_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* FException::_internal_mutable_workingdir() {
  
  return workingdir_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* FException::release_workingdir() {
  // @@protoc_insertion_point(field_release:FException.WorkingDir)
  return workingdir_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void FException::set_allocated_workingdir(std::string* workingdir) {
  if (workingdir != nullptr) {
    
  } else {
    
  }
  workingdir_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), workingdir,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:FException.WorkingDir)
}

// string Description = 9;
inline void FException::clear_description() {
  description_.ClearToEmpty();
}
inline const std::string& FException::description() const {
  // @@protoc_insertion_point(field_get:FException.Description)
  return _internal_description();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void FException::set_description(ArgT0&& arg0, ArgT... args) {
 
 description_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:FException.Description)
}
inline std::string* FException::mutable_description() {
  std::string* _s = _internal_mutable_description();
  // @@protoc_insertion_point(field_mutable:FException.Description)
  return _s;
}
inline const std::string& FException::_internal_description() const {
  return description_.Get();
}
inline void FException::_internal_set_description(const std::string& value) {
  
  description_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* FException::_internal_mutable_description() {
  
  return description_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* FException::release_description() {
  // @@protoc_insertion_point(field_release:FException.Description)
  return description_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void FException::set_allocated_description(std::string* description) {
  if (description != nullptr) {
    
  } else {
    
  }
  description_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), description,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:FException.Description)
}

// string LogFile = 10;
inline void FException::clear_logfile() {
  logfile_.ClearToEmpty();
}
inline const std::string& FException::logfile() const {
  // @@protoc_insertion_point(field_get:FException.LogFile)
  return _internal_logfile();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void FException::set_logfile(ArgT0&& arg0, ArgT... args) {
 
 logfile_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:FException.LogFile)
}
inline std::string* FException::mutable_logfile() {
  std::string* _s = _internal_mutable_logfile();
  // @@protoc_insertion_point(field_mutable:FException.LogFile)
  return _s;
}
inline const std::string& FException::_internal_logfile() const {
  return logfile_.Get();
}
inline void FException::_internal_set_logfile(const std::string& value) {
  
  logfile_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* FException::_internal_mutable_logfile() {
  
  return logfile_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* FException::release_logfile() {
  // @@protoc_insertion_point(field_release:FException.LogFile)
  return logfile_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void FException::set_allocated_logfile(std::string* logfile) {
  if (logfile != nullptr) {
    
  } else {
    
  }
  logfile_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), logfile,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:FException.LogFile)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_exception_2eproto
