cmake_minimum_required(VERSION 3.25)

# 1. TOTAL ISOLATION: Stop CMake from injecting host environment flags
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Force zero out default flags in CACHE to stay sovereign
set(CMAKE_C_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "" FORCE)

# Explicit Toolchain Definition
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_LINKER "/usr/bin/ld.lld")

# 2. TRIGGER DETECTION: project() runs the internal compiler checks
project(AxiomOS LANGUAGES C CXX ASM)

# 3. ENFORCEMENT: Now that project() finished, we can check IDs and Versions
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "AxiomOS requires Clang/LLVM. Detected: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Enforce Clang 21 for reliable C++26 freestanding support
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "21.0")
    message(FATAL_ERROR "Clang version ${CMAKE_CXX_COMPILER_VERSION} is too old for C++26. Upgrade to 21.0+")
endif()

# 4. C++26 STANDARD MANDATE
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

# 5. SOURCE DEFINITIONS
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
    src/kernel/tests/pmm.cpp
)

# 6. KERNEL CONFIGURATION
add_executable(kernel ${KERNEL_SOURCES})

target_compile_options(kernel PRIVATE
    -ffreestanding
    -O0                         
    -fno-exceptions
    -fno-rtti
    -mno-mmx
    -mno-sse
    -mno-sse2
    -mcmodel=kernel
    -Wall -Wextra -Werror -Wpedantic
    -gz=none
    -fno-pic
    -fno-pie
    -fno-stack-protector
    -mno-red-zone
)

target_include_directories(kernel PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_options(kernel PRIVATE
    -nostdlib
    -fuse-ld=lld
    -Wl,--nmagic               
    -Wl,--build-id=none
    -static
    -Wl,-no-pie
    -Wl,--no-dynamic-linker
    -Wl,-z,max-page-size=4096
    -Wl,-z,common-page-size=4096
    -T${PROJECT_SOURCE_DIR}/src/kernel/linker.ld
)

# 7. BOOTLOADER CONFIGURATION (UEFI)
add_executable(bootloader.efi src/bootloader/main.cpp)

target_compile_options(bootloader.efi PRIVATE
    -target x86_64-unknown-uefi
    -ffreestanding
    -fshort-wchar
    -mno-red-zone
    -fno-exceptions
    -fno-rtti
    -O0
    -Wall -Wextra
)

target_include_directories(bootloader.efi PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_options(bootloader.efi PRIVATE
    -target x86_64-unknown-uefi
    -nostdlib
    -fuse-ld=lld
    -Wl,-entry:efi_main
    -Wl,-subsystem:efi_application
)

# 8. POST-BUILD
set_target_properties(kernel PROPERTIES OUTPUT_NAME "kernel")
set_target_properties(bootloader.efi PROPERTIES OUTPUT_NAME "bootloader.efi")
