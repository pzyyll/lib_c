// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: bs_errcode.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "bs_errcode.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace bs_czl {
class MSG_RETDefaultTypeInternal : public ::google::protobuf::internal::ExplicitlyConstructed<MSG_RET> {
} _MSG_RET_default_instance_;

namespace protobuf_bs_5ferrcode_2eproto {


namespace {

::google::protobuf::Metadata file_level_metadata[1];
const ::google::protobuf::EnumDescriptor* file_level_enum_descriptors[1];

}  // namespace

const ::google::protobuf::uint32 TableStruct::offsets[] = {
  ~0u,  // no _has_bits_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(MSG_RET, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
};

static const ::google::protobuf::internal::MigrationSchema schemas[] = {
  { 0, -1, sizeof(MSG_RET)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&_MSG_RET_default_instance_),
};

namespace {

void protobuf_AssignDescriptors() {
  AddDescriptors();
  ::google::protobuf::MessageFactory* factory = NULL;
  AssignDescriptors(
      "bs_errcode.proto", schemas, file_default_instances, TableStruct::offsets, factory,
      file_level_metadata, file_level_enum_descriptors, NULL);
}

void protobuf_AssignDescriptorsOnce() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::internal::RegisterAllTypes(file_level_metadata, 1);
}

}  // namespace

void TableStruct::Shutdown() {
  _MSG_RET_default_instance_.Shutdown();
  delete file_level_metadata[0].reflection;
}

void TableStruct::InitDefaultsImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::internal::InitProtobufDefaults();
  _MSG_RET_default_instance_.DefaultConstruct();
}

void InitDefaults() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &TableStruct::InitDefaultsImpl);
}
void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] = {
      "\n\020bs_errcode.proto\022\006bs_czl\"4\n\007MSG_RET\")\n"
      "\007ErrCode\022\013\n\007SUCCESS\020\000\022\021\n\004FAIL\020\377\377\377\377\377\377\377\377\377\001"
      "b\006proto3"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 88);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "bs_errcode.proto", &protobuf_RegisterTypes);
  ::google::protobuf::internal::OnShutdown(&TableStruct::Shutdown);
}

void AddDescriptors() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;

}  // namespace protobuf_bs_5ferrcode_2eproto

const ::google::protobuf::EnumDescriptor* MSG_RET_ErrCode_descriptor() {
  protobuf_bs_5ferrcode_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_bs_5ferrcode_2eproto::file_level_enum_descriptors[0];
}
bool MSG_RET_ErrCode_IsValid(int value) {
  switch (value) {
    case -1:
    case 0:
      return true;
    default:
      return false;
  }
}

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const MSG_RET_ErrCode MSG_RET::SUCCESS;
const MSG_RET_ErrCode MSG_RET::FAIL;
const MSG_RET_ErrCode MSG_RET::ErrCode_MIN;
const MSG_RET_ErrCode MSG_RET::ErrCode_MAX;
const int MSG_RET::ErrCode_ARRAYSIZE;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

MSG_RET::MSG_RET()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    protobuf_bs_5ferrcode_2eproto::InitDefaults();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:bs_czl.MSG_RET)
}
MSG_RET::MSG_RET(const MSG_RET& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  // @@protoc_insertion_point(copy_constructor:bs_czl.MSG_RET)
}

void MSG_RET::SharedCtor() {
  _cached_size_ = 0;
}

MSG_RET::~MSG_RET() {
  // @@protoc_insertion_point(destructor:bs_czl.MSG_RET)
  SharedDtor();
}

void MSG_RET::SharedDtor() {
}

void MSG_RET::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* MSG_RET::descriptor() {
  protobuf_bs_5ferrcode_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_bs_5ferrcode_2eproto::file_level_metadata[0].descriptor;
}

const MSG_RET& MSG_RET::default_instance() {
  protobuf_bs_5ferrcode_2eproto::InitDefaults();
  return *internal_default_instance();
}

MSG_RET* MSG_RET::New(::google::protobuf::Arena* arena) const {
  MSG_RET* n = new MSG_RET;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void MSG_RET::Clear() {
// @@protoc_insertion_point(message_clear_start:bs_czl.MSG_RET)
}

bool MSG_RET::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:bs_czl.MSG_RET)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
  handle_unusual:
    if (tag == 0 ||
        ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
        ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
      goto success;
    }
    DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
  }
success:
  // @@protoc_insertion_point(parse_success:bs_czl.MSG_RET)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:bs_czl.MSG_RET)
  return false;
#undef DO_
}

void MSG_RET::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:bs_czl.MSG_RET)
  // @@protoc_insertion_point(serialize_end:bs_czl.MSG_RET)
}

::google::protobuf::uint8* MSG_RET::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic;  // Unused
  // @@protoc_insertion_point(serialize_to_array_start:bs_czl.MSG_RET)
  // @@protoc_insertion_point(serialize_to_array_end:bs_czl.MSG_RET)
  return target;
}

size_t MSG_RET::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:bs_czl.MSG_RET)
  size_t total_size = 0;

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void MSG_RET::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:bs_czl.MSG_RET)
  GOOGLE_DCHECK_NE(&from, this);
  const MSG_RET* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const MSG_RET>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:bs_czl.MSG_RET)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:bs_czl.MSG_RET)
    MergeFrom(*source);
  }
}

void MSG_RET::MergeFrom(const MSG_RET& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:bs_czl.MSG_RET)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
}

void MSG_RET::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:bs_czl.MSG_RET)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void MSG_RET::CopyFrom(const MSG_RET& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:bs_czl.MSG_RET)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool MSG_RET::IsInitialized() const {
  return true;
}

void MSG_RET::Swap(MSG_RET* other) {
  if (other == this) return;
  InternalSwap(other);
}
void MSG_RET::InternalSwap(MSG_RET* other) {
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata MSG_RET::GetMetadata() const {
  protobuf_bs_5ferrcode_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_bs_5ferrcode_2eproto::file_level_metadata[0];
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// MSG_RET

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

}  // namespace bs_czl

// @@protoc_insertion_point(global_scope)
