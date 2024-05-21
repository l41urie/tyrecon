#include "module.hpp"
#include <windows.h>
#include <Psapi.h>

namespace ada {

Module::Module(void *image) : image(image) {
  char path[MAX_PATH];
  auto len = GetModuleFileNameA((HMODULE)image, path, MAX_PATH);

  module_path = std::string(path, len);
  name = &*(module_path.begin() + module_path.find_last_of('\\'));

  // important data structures
  u8 const *const base = (u8 *)image; // to bypass pointer arithmetic
  auto const *const dos = (IMAGE_DOS_HEADER *)image;
  auto const *const nt = (IMAGE_NT_HEADERS *)(base + dos->e_lfanew);

  // Fill sections
  u16 num_sections = nt->FileHeader.NumberOfSections;
  PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt);
  for (u16 i = 0; i < num_sections; i++) {
    sections.emplace_back(
        Section{.name = std::string((char *)section->Name),
                .memory_block{(u64)(base + section->VirtualAddress),
                              (u64)(base + section->VirtualAddress +
                                    section->Misc.VirtualSize)}});
    section += 1;
  }

  MODULEINFO m;
  GetModuleInformation(GetCurrentProcess(), (HMODULE)image, &m, sizeof(m));
  memory_block = {(u64)image, (u64)image + m.SizeOfImage};

  // Fill exports
  auto const &export_dir =
      nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
  if (export_dir.Size != 0) {
    auto export_list =
        (IMAGE_EXPORT_DIRECTORY *)(base + export_dir.VirtualAddress);

    auto names = (u32 *)(base + export_list->AddressOfNames);
    auto functions = (u32 *)(base + export_list->AddressOfFunctions);
    auto ordinals = (u16 *)(base + export_list->AddressOfNameOrdinals);
    for (auto i = 0; i < export_list->NumberOfNames; ++i) {
      auto name = (char const *)(base + names[i]);
      auto address = (void **)(base + functions[ordinals[i]]);

      exports.emplace_back(NamedFunction{.location = address, .name = name});
    }
  }

  // Fill imports
  auto const &import_dir =
      nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
  if (import_dir.Size != 0) {
    auto iid = (IMAGE_IMPORT_DESCRIPTOR *)(base + import_dir.VirtualAddress);
    while (iid->Characteristics) {
      auto orig_first_thunk =
          (IMAGE_THUNK_DATA *)(base + iid->OriginalFirstThunk);
      auto first_thunk = (IMAGE_THUNK_DATA *)(base + iid->FirstThunk);
      auto lib_name = (LPCSTR)(base + iid->Name);

      while (orig_first_thunk->u1.AddressOfData) {
        if (!(orig_first_thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
          auto ibn =
              (IMAGE_IMPORT_BY_NAME *)(base +
                                       orig_first_thunk->u1.AddressOfData);

          imports.emplace_back(NamedFunction{
              .location = (void **)&first_thunk->u1.Function,
              .name = ibn->Name,
          });
        }

        orig_first_thunk++;
        first_thunk++;
      }

      iid++;
    }
  }
}
} // namespace ada