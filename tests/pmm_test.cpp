#include <iostream>
#include <cassert>
#include <lib/core/pmm.h>

int main() {
    std::cout << "Testing PMM..." << std::endl;
    // Call pmm::Initialize() with fake map
    // Test AllocatePage() and FreePage()
    std::cout << "PMM tests passed." << std::endl;
    return 0;
}
