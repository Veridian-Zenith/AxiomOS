#ifndef AXIOMOS_ELF_HPP
#define AXIOMOS_ELF_HPP

#include <stdint.h>

namespace axiom::elf {

    typedef uint64_t Elf64_Addr;
    typedef uint64_t Elf64_Off;
    typedef uint16_t Elf64_Half;
    typedef uint32_t Elf64_Word;
    typedef int32_t  Elf64_Sword;
    typedef uint64_t Elf64_Xword;
    typedef int64_t  Elf64_Sxword;

    // ELF Header
    struct Elf64_Ehdr {
        unsigned char e_ident[16];
        Elf64_Half    e_type;
        Elf64_Half    e_machine;
        Elf64_Word    e_version;
        Elf64_Addr    e_entry;
        Elf64_Off     e_phoff;
        Elf64_Off     e_shoff;
        Elf64_Word    e_flags;
        Elf64_Half    e_ehsize;
        Elf64_Half    e_phentsize;
        Elf64_Half    e_phnum;
        Elf64_Half    e_shentsize;
        Elf64_Half    e_shnum;
        Elf64_Half    e_shstrndx;
    };

    constexpr uint32_t ELFMAG = 0x464C457F; // "\x7FELF" in little endian

    // Program Header
    struct Elf64_Phdr {
        Elf64_Word    p_type;
        Elf64_Word    p_flags;
        Elf64_Off     p_offset;
        Elf64_Addr    p_vaddr;
        Elf64_Addr    p_paddr;
        Elf64_Xword   p_filesz;
        Elf64_Xword   p_memsz;
        Elf64_Xword   p_align;
    };

    constexpr uint32_t PT_LOAD = 1;

    constexpr uint32_t PF_X = 1;
    constexpr uint32_t PF_W = 2;
    constexpr uint32_t PF_R = 4;

} // namespace axiom::elf

#endif // AXIOMOS_ELF_HPP
