# Axiom-VFS Specification (3-Boot Storage)

## 1. Overview

Axiom-VFS is a high-performance virtual filesystem layer. It bridges the gap between hardware storage and user-space applications, providing a unified interface while optimizing for the unique characteristics of the target architecture and the reference storage controller.

## 2. The "Pinning System"

To achieve instantaneous system responsiveness, Axiom-VFS implements a memory pinning mechanism.

- **Selective Pinning:** Critical system binaries (Registry core, Fish shell), core libraries, and hot metadata are permanently mapped into physical RAM.
- **Cache Sovereignty:** Pinned pages are excluded from the VMM's page-replacement algorithms, ensuring zero disk-read latency for core system functions.
- **Dynamic Pinning:** Users or high-priority applications can request to pin specific datasets (e.g., a database index) into memory, subject to available physical RAM limits.

## 3. Ephemeral Files

Axiom-VFS includes a built-in memory-backed filesystem for temporary and volatile data.

- **Volatile Storage:** Directories like `/tmp`, `/var/run`, and `/var/log/session` reside entirely in RAM.
- **Lifecycle:** All ephemeral data is automatically discarded on system shutdown or crash, ensuring that the next boot begins from a clean, known-good state.
- **Performance:** Bypasses the storage stack entirely, providing near-CPU-speed access for temporary computation files and IPC sockets.

### 3.1 3-Boot Purge

A core integrity feature of AxiomOS is the "3-Boot Purge".

- **Concept:** Every three successful boots, the ephemeral storage and system log buffers are forcefully purged to prevent forensic accumulation and ensure a "fresh" state.
- **Trigger:** Tracked by the Registry in the non-volatile UEFI variable space.
- **Inclusion:** Automatically includes all RAM-backed directories.

## 4. Storage Controller Interface

The VFS interacts directly with the storage controller using a native driver.

- **HighwayHash Checksumming:** Every data block (4KiB) is checksummed using **HighwayHash-64** during write operations. Checksums are stored as metadata and verified upon every read to detect silent data corruption.
- **Direct Submission:** Uses high-priority submission queues (SQ) and completion queues (CQ) to minimize latency between the VFS request and hardware execution.
- **Wear Leveling & Health:** The VFS monitors storage device health telemetry and exposes it through the Fish shell.

## 5. 3-Boot Storage Logic

AxiomOS manages its storage across three logical boot stages:

1. **Boot 1 (Firmware):** The EFI System Partition (FAT32) containing the `Sentinel` bootloader and early diagnostics.
2. **Boot 2 (The Registry):** A small, high-integrity boot partition (read-only) containing the AxiomOS kernel and essential Plug-in I/O drivers.
3. **Boot 3 (Axiom Core):** The main storage partition (XFS-formatted) containing the full userspace, user data, and non-critical applications.

## 6. XFS Implementation

- **Read/Write Support:** Full support for the primary filesystem partition on the storage device.
- **Consistency:** Leverages XFS journaling for fast recovery after power loss.
- **Coexistence:** Axiom-VFS can mount other partitions (e.g., NTFS, ext4) for read-only access in multi-boot scenarios.

## 7. Security

- **Encrypted Volumes:** Support for full-disk encryption using the hardware acceleration for encryption where available (e.g., AES-NI).
- **Signed Storage:** The VFS can enforce digital signature verification for any executable read from the Boot 2 or Boot 3 partitions.
