# =========================================================
# AxiomOS CMake Configuration (Pure LLVM/LLD)
# =========================================================
cmake_minimum_required(VERSION 3.25)

# 1. FORCE CORE SETTINGS BEFORE PROJECT()
# This prevents CMake from trying to link host-level sanitizers during the sanity check.
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# 2. CLEAR HOST ENVIRONMENT LEAKS
# We explicitly overwrite these to ensure 'march=native' or '-fsanitize' from the host don't bleed in.
set(CMAKE_C_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "" FORCE)

project(AxiomOS LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

# ====================================================================
# Kernel Target
# ====================================================================
# Using a list for better readability and management
set(KERNEL_FLAGS
    "-ffreestanding"
    "-O0"
    "-fno-stack-protector"
    "-fno-exceptions"
    "-fno-rtti"
    "-mno-red-zone"
    "-mno-mmx"
    "-mno-sse"
    "-mno-sse2"
    "-mcmodel=kernel"
    "-fno-pie"
    "-fno-pic"
    "-Wall"
    "-Wextra"
    "-Werror"
    "-Wpedantic"
    "-gz=none"           # Explicitly disable debug compression leaks
)

set(KERNEL_SOURCES
    src/kernel/main.cpp
    src/kernel/drivers/serial.cpp
    src/kernel/arch/x64/entry.S
    src/kernel/arch/x64/cpu.cpp
    src/kernel/arch/x64/gdt.cpp
    src/kernel/arch/x64/gdt.S
    src/kernel/arch/x64/idt.cpp
    src/kernel/arch/x64/interrupts.S
    src/kernel/arch/x64/apic.cpp
    src/kernel/mm/allocator.cpp
)

add_executable(kernel ${KERNEL_SOURCES})

# Use 'BEFORE' and 'PRIVATE' to ensure these are the ONLY flags used
target_compile_options(kernel BEFORE PRIVATE ${KERNEL_FLAGS})

target_link_options(kernel PRIVATE
    "-nostdlib"
    "-fuse-ld=lld"
    "-Wl,--nmagic"
    "-Wl,--build-id=none" # Remove build-id to prevent host-specific hash leaks
    "-static"
    "-no-pie"
    "-Wl,-z,max-page-size=4096"
    "-Wl,-z,common-page-size=4096"
    "-T${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/linker.ld"
)

# ====================================================================
# UEFI Bootloader Target
# ====================================================================
set(UEFI_FLAGS
    "-target" "x86_64-unknown-uefi"
    "-ffreestanding"
    "-fshort-wchar"
    "-mno-red-zone"
    "-fno-exceptions"
    "-fno-rtti"
    "-O0"
    "-Wall"
    "-Wextra"
)

add_executable(bootloader.efi src/bootloader/main.cpp)
target_compile_options(bootloader.efi BEFORE PRIVATE ${UEFI_FLAGS})

# Ensure the bootloader doesn't inherit any default link flags
set_target_properties(bootloader.efi PROPERTIES
    LINK_FLAGS ""
    SUFFIX ""
)

target_link_options(bootloader.efi PRIVATE
    "-target" "x86_64-unknown-uefi"
    "-nostdlib"
    "-fuse-ld=lld"
    "-Wl,-entry:efi_main"
    "-Wl,-subsystem:efi_application"
)
