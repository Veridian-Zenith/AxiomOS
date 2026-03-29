# =========================================================
# AxiomOS CMake Configuration (Pure LLVM/LLD)
# =========================================================
cmake_minimum_required(VERSION 3.25)
project(AxiomOS LANGUAGES C CXX ASM)

# Clear all default flags
set(CMAKE_C_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

# ====================================================================
# Kernel Target
# ====================================================================
set(KERNEL_FLAGS
    "-march=native"
    "-O0"
    "-ffreestanding"
    "-fno-exceptions"
    "-fno-rtti"
    "-fno-stack-protector"
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
target_compile_options(kernel PRIVATE ${KERNEL_FLAGS})
target_link_options(kernel PRIVATE
    "-nostdlib"
    "-fuse-ld=lld"
    "-Wl,--nmagic"
    "-Wl,--build-id=sha1"
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
    "-Wpedantic"
)

add_executable(bootloader.efi src/bootloader/main.cpp)
target_compile_options(bootloader.efi PRIVATE ${UEFI_FLAGS})
set_target_properties(bootloader.efi PROPERTIES SUFFIX "")
set_target_properties(bootloader.efi PROPERTIES LINK_FLAGS "")
target_link_options(bootloader.efi PRIVATE
    "-target" "x86_64-unknown-uefi"
    "-nostdlib"
    "-fuse-ld=lld"
    "-Wl,-entry:efi_main"
    "-Wl,-subsystem:efi_application"
)
