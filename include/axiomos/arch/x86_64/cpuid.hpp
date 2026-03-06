#ifndef AXIOMOS_ARCH_X86_64_CPUID_HPP
#define AXIOMOS_ARCH_X86_64_CPUID_HPP

#include <stdint.h>
#include "axiomos/utils/serial.hpp"

namespace axiom::arch::x86_64 {

    struct CpuidResult {
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;
    };

    inline CpuidResult cpuid(uint32_t leaf, uint32_t subleaf = 0) {
        CpuidResult result;
        __asm__ volatile("cpuid"
            : "=a"(result.eax), "=b"(result.ebx), "=c"(result.ecx), "=d"(result.edx)
            : "a"(leaf), "c"(subleaf)
        );
        return result;
    }

    /// @brief Check for Intel Hybrid Technology (Alder Lake and newer)
    /// @return True if hybrid architecture is detected
    inline bool isHybridCpu() {
        // Check max leaf
        CpuidResult res0 = cpuid(0);
        if (res0.eax < 0x1A) return false;

        // Check Hybrid Information Leaf (0x1A)
        // If EAX implies a core type (20h or 40h), it's hybrid-aware.
        CpuidResult res1A = cpuid(0x1A);
        uint32_t core_type = (res1A.eax >> 24) & 0xFF;
        return (core_type == 0x20 || core_type == 0x40);
    }

    /// @brief Get the core type of the running processor (P-core vs E-core)
    /// @return 0x20 for Atom (E-core), 0x40 for Core (P-core), 0 for unknown
    inline uint32_t getCoreType() {
        CpuidResult res0 = cpuid(0);
        if (res0.eax < 0x1A) return 0;

        CpuidResult res1A = cpuid(0x1A);
        return (res1A.eax >> 24) & 0xFF;
    }

    inline void printCpuInfo() {
        // Vendor ID
        CpuidResult res0 = cpuid(0);
        char vendor[13];
        reinterpret_cast<uint32_t*>(vendor)[0] = res0.ebx;
        reinterpret_cast<uint32_t*>(vendor)[1] = res0.edx;
        reinterpret_cast<uint32_t*>(vendor)[2] = res0.ecx;
        vendor[12] = '\0';
        
        axiom::serial::printf("[CPU] Vendor: %s\n", vendor);

        // Processor Brand String
        // Extended function 0x80000002, 0x80000003, 0x80000004
        CpuidResult resExt = cpuid(0x80000000);
        if (resExt.eax >= 0x80000004) {
            char brand[49];
            uint32_t* brand_ptr = reinterpret_cast<uint32_t*>(brand);
            
            CpuidResult b1 = cpuid(0x80000002);
            brand_ptr[0] = b1.eax; brand_ptr[1] = b1.ebx; brand_ptr[2] = b1.ecx; brand_ptr[3] = b1.edx;
            
            CpuidResult b2 = cpuid(0x80000003);
            brand_ptr[4] = b2.eax; brand_ptr[5] = b2.ebx; brand_ptr[6] = b2.ecx; brand_ptr[7] = b2.edx;
            
            CpuidResult b3 = cpuid(0x80000004);
            brand_ptr[8] = b3.eax; brand_ptr[9] = b3.ebx; brand_ptr[10] = b3.ecx; brand_ptr[11] = b3.edx;
            
            brand[48] = '\0';
            // Simple trim (optional)
            axiom::serial::printf("[CPU] Model: %s\n", brand);
        }

        if (isHybridCpu()) {
            uint32_t type = getCoreType();
            const char* type_str = "Unknown";
            if (type == 0x20) type_str = "Atom (E-core)";
            else if (type == 0x40) type_str = "Core (P-core)";
            
            axiom::serial::printf("[CPU] Hybrid Arch Detected. BSP Core Type: %s\n", type_str);
        }
    }

} // namespace axiom::arch::x86_64

#endif // AXIOMOS_ARCH_X86_64_CPUID_HPP
