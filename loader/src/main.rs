#![no_std]
#![no_main]

extern crate alloc;

use core::panic::PanicInfo;
use uefi::prelude::*;
use uefi::helpers;
use uefi::table::boot::MemoryType;

// Import the handoff module
mod handoff;
use handoff::{Loader, AxiomHandoff};

// 🔑 REQUIRED: global allocator symbol
#[global_allocator]
static ALLOCATOR: uefi::allocator::Allocator = uefi::allocator::Allocator;

#[entry]
fn main(_image: Handle, st: SystemTable<Boot>) -> Status {
    // Initializes allocator + logging backend
    if helpers::init().is_err() {
        return Status::ABORTED;
    }

    log::info!("AxiomOS UEFI loader alive");

    // Prepare handoff structure
    let handoff_result = {
        let bt: &BootServices = st.boot_services();
        let loader = Loader::new(bt, &st);
        loader.prepare_handoff()
    };

    match handoff_result {
        Ok(handoff) => {
            log::info!("Handoff structure prepared successfully");

            let bt: &BootServices = st.boot_services();

            // Allocate memory for handoff structure in a known location
            // This should be in a region that the kernel can access
            let handoff_ptr = bt.allocate_pool(
                MemoryType::LOADER_DATA,
                AxiomHandoff::size(),
            ).expect("Failed to allocate handoff memory");

            // Copy handoff data to allocated memory
            unsafe {
                core::ptr::copy_nonoverlapping(
                    &handoff as *const AxiomHandoff,
                    handoff_ptr.as_ptr() as *mut AxiomHandoff,
                    1,
                );
            }

            log::info!("Handoff structure copied to 0x{:x}", handoff_ptr.as_ptr() as u64);

            // Exit boot services
            let loader = {
                let bt: &BootServices = st.boot_services();
                Loader::new(bt, &st)
            };

            if let Err(e) = loader.exit_boot_services(&handoff) {
                log::error!("Failed to exit boot services: {:?}", e);
                return Status::ABORTED;
            }

            log::info!("Boot services exited, ready to transfer control to kernel");

            // At this point, we would normally jump to the kernel entry point
            // For now, we'll just halt to demonstrate successful completion
            let bt: &BootServices = st.boot_services();
            loop {
                bt.stall(1_000_000);
            }
        }
        Err(e) => {
            log::error!("Failed to prepare handoff: {:?}", e);
            return Status::ABORTED;
        }
    }
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}
