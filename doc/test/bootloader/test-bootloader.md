# Bootloader Tests

## Basic Tests

1. Update NOR-Flash with most recent bootloader
2. Update NOR-Flash with mosten recent OBSW binary
3. Disable all hamming code checks (global and local checks)
4. Boot OBSW and verify through terminal that OBSW from NOR-Flash is booted
5. Verify that the OBSW runs stable for a few minutes

## Tests - Hamming code checks disabled

### Restart counter cycling test

1. Prepare the SD Card by deleting all image slots.
1. Restart the OBSW multiple times without resetting the boot counters (3 times)
3. Verify that the bootloader now loads the image slot instead. Because the file does not exist, it should immediately increment the reboot counter and reboot for all image slots until it boots the NOR-Flash again
