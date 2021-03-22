# Bootloader Tests

# Setup Procedure

1. Update NOR-Flash with most recent bootloader
2. Update NOR-Flash with mosten recent OBSW binary

# Baseline Tests

## Regular Boot Test

1. Disable all hamming code checks (global and local checks)
2. Boot OBSW and verify through terminal that OBSW from NOR-Flash is booted
3. Verify that the OBSW runs stable for a few minutes

## Software Update Tests

1. Load a SW update with a higher version number than the one currently used.
2. Enable to load SW update
3. Disable all hamming code checks.
4. Reboot and verify software update is loaded

# Advanced Tests - Hamming code checks disabled

## Restart counter cycling test 1

1. Prepare the SD Card by deleting all image slots.
2. Disable all hamming code checks.
3. Restart the OBSW multiple times without resetting the boot counters (3 times)
4. Verify that the bootloader now loads the image slot instead. Because the file does not exist, it should immediately increment the reboot counter and reboot for all image slots until it boots the NOR-Flash again

## Restart counter cycling test 2

1. Prepare the SD card by introducing an image in slot 0.
2. Disable all hamming code checks.
3. Restart the OBSW multiple times without resetting the boot counters.
4. Verify that the bootloader now loads the SD card 0 slot instead.

## Restart counter cycling test 3

1. Prepare the SD card by introducing an image in slot 1.
2. Disable all hamming code checks.
3. Restart the OBSW multiple times without resetting the boot counters.
4. Verify that the bootloader now loads the SD card 1 slot instead.

