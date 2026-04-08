set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Force use of LLVM/Clang
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Target architectures
set(CMAKE_C_COMPILER_TARGET x86_64-unknown-none)
set(CMAKE_CXX_COMPILER_TARGET x86_64-unknown-none)

# Essential flags for freestanding environment
set(CMAKE_CXX_FLAGS "-ffreestanding -fno-exceptions -fno-rtti -std=c++26 -Wall -Wextra -Werror -flto=thin" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS "-ffreestanding -Wall -Wextra -Werror -flto=thin" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "-flto=thin" CACHE STRING "" FORCE)
