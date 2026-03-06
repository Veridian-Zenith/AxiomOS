# =========================================================
# AxiomOS CMake Configuration (Pure LLVM/LLD)
# =========================================================
cmake_minimum_required(VERSION 3.25)
project(AxiomOS LANGUAGES C CXX ASM)

# --------------------------------------------------------------------
# 1. Reset Environment Flags
# --------------------------------------------------------------------
# Clear all default flags to prevent host environment pollution
set(CMAKE_C_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS "" CACHE STRING "" FORCE)

set(CMAKE_C_FLAGS_DEBUG "" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "" CACHE STRING "" FORCE)

# --------------------------------------------------------------------
# 2. Global Settings
# --------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

# ====================================================================
# Kernel Flags (Freestanding)
# ====================================================================
# Note: -fuse-ld=lld is a linker flag, not a compile flag.
# It should be passed to target_link_options, not target_compile_options.
set(KERNEL_FLAGS
    "-march=native"
    "-O3"
    "-ffreestanding"
    "-fno-exceptions"
    "-fno-rtti"
    "-fno-stack-protector"
    "-mno-red-zone"
    "-mno-mmx"
    "-mno-sse"
    "-mno-sse2"
    "-mcmodel=kernel"
    "-Wall"
    "-Wextra"
    "-Werror"
    "-flto=thin"
)

# ====================================================================
# Kernel Target
# ====================================================================
set(KERNEL_SOURCES
    src/kernel/main.cpp
    src/kernel/utils/serial.cpp
    src/kernel/arch/x86_64/entry.S
    src/kernel/arch/x86_64/x86_64.cpp
    src/kernel/mm/allocator.cpp
    src/kernel/mm/vmm.cpp
)

add_executable(kernel ${KERNEL_SOURCES})
target_compile_options(kernel PRIVATE ${KERNEL_FLAGS})
target_link_options(kernel PRIVATE
    "-nostdlib"
    "-fuse-ld=lld"
    "-Wl,--nmagic"
    "-Wl,--build-id=sha1"
    "-T${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/linker.ld"
)

# ====================================================================
# UEFI / Bootloader Flags
# ====================================================================
# Use the official 'x86_64-unknown-uefi' target.
# This instructs Clang to generate a PE/COFF executable suitable for UEFI
# without relying on Windows-specific target names or external tools.
set(UEFI_FLAGS
    "-target" "x86_64-unknown-uefi"
    "-ffreestanding"
    "-fshort-wchar"
    "-mno-red-zone"
    "-fno-exceptions"
    "-fno-rtti"
    "-fno-stack-protector"
    "-O3"
    "-march=native"
    "-flto=thin"
)

# Object library for Bootloader (isolates flags)
add_library(bootloader_obj OBJECT src/bootloader/main.cpp)
target_compile_options(bootloader_obj PRIVATE ${UEFI_FLAGS} "-Wno-unused-command-line-argument")

# EFI executable
add_executable(bootloader.efi $<TARGET_OBJECTS:bootloader_obj>)
set_target_properties(bootloader.efi PROPERTIES SUFFIX "")
set_target_properties(bootloader.efi PROPERTIES LINK_FLAGS "")

# Link bootloader for UEFI subsystem.
# The 'uefi' target implies the correct subsystem and entry point conventions.
# The linker must also be told which target to use.
target_link_options(bootloader.efi PRIVATE
    "-target" "x86_64-unknown-uefi"
    "-nostdlib"
    "-fuse-ld=lld"
    "-Wl,-entry:efi_main"
)

