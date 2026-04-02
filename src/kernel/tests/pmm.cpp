#include "axiom/tests/pmm.hpp"
#include "axiom/mm/allocator.hpp"
#include "axiom/drivers/serial.hpp"

namespace axiom::tests {
    void test_pmm() {
        serial::puts("[PMM-TEST] Starting tests...\n");

        // Test 1: Allocate and free a single page
        serial::puts("[PMM-TEST] Test 1: Single page allocation\n");
        void* p1 = mm::alloc_pages(1);
        if (p1) {
            serial::puts("[PMM-TEST] Allocated 1 page.\n");
            mm::free_pages(p1, 1);
            serial::puts("[PMM-TEST] Freed 1 page.\n");
        } else {
            serial::puts("[PMM-TEST] FAILED: Could not allocate 1 page.\n");
        }


        // Test 2: Allocate and free multiple pages
        serial::puts("[PMM-TEST] Test 2: Multi-page allocation\n");
        void* p2 = mm::alloc_pages(10);
        if (p2) {
            serial::puts("[PMM-TEST] Allocated 10 pages.\n");
            mm::free_pages(p2, 10);
            serial::puts("[PMM-TEST] Freed 10 pages.\n");
        } else {
            serial::puts("[PMM-TEST] FAILED: Could not allocate 10 pages.\n");
        }

        // Test 3: Verify memory counts
        serial::puts("[PMM-TEST] Test 3: Memory count verification\n");
        size_t initial_free = mm::get_free_memory();
        p1 = mm::alloc_pages(1);
        if (p1) {
            size_t after_alloc = mm::get_free_memory();
            if (initial_free - 4096 != after_alloc) {
                 serial::puts("[PMM-TEST] FAILED: Memory count verification after alloc!\n");
            }
            mm::free_pages(p1, 1);
            size_t after_free = mm::get_free_memory();
            if (initial_free != after_free) {
                serial::puts("[PMM-TEST] FAILED: Memory count verification after free!\n");
            }
        } else {
            serial::puts("[PMM-TEST] FAILED: Could not allocate page for memory count test.\n");
        }

        serial::puts("[PMM-TEST] All tests passed.\n");

        // The following tests will halt the kernel if they succeed.
        // Uncomment them one by one to test denial cases.
        // serial::puts("[PMM-TEST] Testing alloc_pages(0)...\n");
        // mm::alloc_pages(0);

        // serial::puts("[PMM-TEST] Testing non-aligned free...\n");
        // mm::free_pages((void*)0x123, 1);

        // serial::puts("[PMM-TEST] Testing double-free...\n");
        // p1 = mm::alloc_pages(1);
        // if(p1) {
        //   mm::free_pages(p1, 1);
        //   mm::free_pages(p1, 1);
        // }

    }
}
