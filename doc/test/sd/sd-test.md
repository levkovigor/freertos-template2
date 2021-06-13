# SD-Card Tests

The SD-Card is a part of the boot process which already verifies functionalities like
consecutive file writes, file deletion, file locking and file copying.

## Basic tests

Write to simple file - Verified
Copy file - Verified
Read file information - Verified
Delete File - Verified
Lock and Unlock File - Verified

## Basic Image tests

Not verified

1. Put a small test file on one SD-Card which to mark it
2. Switch the SD card via telecommand and verify whether switch is performed successfully

## Software Image tests

### Basic Test

Verified

1. Upload a binary and verify the full size has been written by checking its size. 
   Also verify lock status

###  Copy Test

Verified

1. Copy the uploaded binary to NOR-Flash. Verify the binary size was written to FRAM as well and verify it is correct.

