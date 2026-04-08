#pragma once
#include <cstdint>

namespace axiom {

struct PageTableEntry {
    uint64_t Present : 1;
    uint64_t Writable : 1;
    uint64_t User : 1;
    uint64_t WriteThrough : 1;
    uint64_t CacheDisable : 1;
    uint64_t Accessed : 1;
    uint64_t Dirty : 1;
    uint64_t HugePage : 1;
    uint64_t Global : 1;
    uint64_t Ignored : 3;
    uint64_t Address : 40;
    uint64_t Ignored2 : 11;
    uint64_t NX : 1;
};

// ... add PageTable structure ...
typedef PageTableEntry PageTable[512];

} // namespace axiom
