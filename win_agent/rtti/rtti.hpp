#pragma once
#include <meta.hpp>
#include <optional>
#include <rva.hpp>
#include <set>
#include <string>

namespace tyrecon {
struct Block;
struct Module;

namespace rtti {

// fowrad-declarations
struct Pmd;
struct TypeDescriptor;
struct CompleteObjectLocator;
struct ClassHierarchyDescriptor;
struct BaseClassArray;
struct BaseClassDescriptor;

enum class Signature : u32 {
  X86 = 0,
  X64 = 1,
};

inline bool signature_matches_architecture(Signature signature) {
  switch (signature) {
  case Signature::X86:
    return sizeof(void *) == 4;
  case Signature::X64:
    return sizeof(void *) == 8;
  default:
    return false;
  }
}

struct Pmd {
  i32 mdisp; // member displacement
  i32 pdisp; // vbtable displacement
  i32 vdisp; // displacement inside vbtable
};

struct TypeDescriptor // type_info
{
  void *vtbl;
  char const *undecorated_name;
  char const decorated_name[1];

  std::optional<std::string> demangle() const;
};

struct CompleteObjectLocator {
  Signature signature; // 0 = x86, 1 = x64
  u32 offset;              // offset of this vtable in the complete class
  u32 cd_offset;           // constructor displacement offset

  Rva32<TypeDescriptor>
      type_descriptor; // TypeDescriptor of the complete class
  Rva32<ClassHierarchyDescriptor>
      class_descriptor; // describes inheritance hierarchy
  Rva32<CompleteObjectLocator> self;

  bool check_signature() const {
    return signature_matches_architecture(signature);
  }
};

struct ClassHierarchyDescriptor {
  Signature signature;

  // flags for |attributes|
  static auto constexpr MULTIPLE_INHERITANCE = (1 << 0);
  static auto constexpr VIRTUAL_INHERITANCE = (1 << 1);

  u32 attributes;
  u32 num_base_classes;
  Rva32<BaseClassArray> base_class_array;

  bool check_signature() const {
    return signature_matches_architecture(signature);
  }
};

struct BaseClassArray {
  Rva32<BaseClassDescriptor> descriptors[1];
};

struct BaseClassDescriptor {
  Rva32<TypeDescriptor> type_descriptor; // type descriptor of the class

  u32 num_contained_bases; // number of nested classes following in the Base
                           // Class Array

  Pmd where;      // pointer-to-member displacement info
  u32 attributes; // flags, usually 0
};

void find_dispatch_tables(tyrecon::Module const &mod,
                          std::set<tyrecon::Block> &dispatch_tables);

void find_rtti(tyrecon::Module const &mod);
} // namespace rtti
} // namespace tyrecon