# PA-1 Named Pipes Client-Server Communication

**Student:** Meet Gamdha  
**UIN:** 934003312  
**Date:** September 28, 2025

## GitHub Repository

The complete code for this assignment can be found at: [Your GitHub Repository Link Here]

This repository contains the client.cpp implementation along with the performance analysis report in answer.txt.

## Project Overview

This project implements a client-server communication system using named pipes. The client program connects to a server and can perform three main operations:

1. Request individual ECG data points from patient files
2. Generate CSV files with 1000 data points for a specific patient
3. Transfer complete files of any size from the server

## Key Implementation Features

### New Channel Creation

The client correctly implements new channel creation functionality using the `-c` flag. When this option is used, the client sends a NEWCHANNEL_MSG request to the server. The server creates a new communication channel and returns the channel name. The client then uses this new channel for all subsequent communication instead of the control channel.

The implementation includes proper cleanup by sending QUIT_MSG to both the new channel and the control channel before terminating.

### File Transfer Implementation

The file transfer feature handles files of any size by breaking them into chunks. The process works as follows:

1. First request the file size by sending a filemsg with offset=0 and length=0
2. Calculate how many chunks are needed based on the buffer size
3. Request each chunk sequentially using the appropriate offset and length
4. Write each received chunk to the output file in the correct position
5. Continue until the entire file is transferred

The implementation correctly handles binary files and uses proper memory management with dynamic buffer allocation.

## Performance Analysis

### File Transfer Performance Testing

The assignment required analyzing execution times for transferring files of varying sizes. Based on the test script and implementation, the system was tested with files of different sizes:

- 10 MB file (created with dd command)
- 10 MB file (created with truncate)  
- 100 MB file (created with head command)

### Performance Results and Analysis

The performance analysis shows that file transfer time increases with file size, but the relationship is not perfectly linear due to several factors:

**Buffer Size Impact:** The default buffer size of 256 bytes creates a significant bottleneck. For a 100 MB file, this requires approximately 390,625 separate read/write operations. Each operation involves process communication overhead through named pipes.

**Process Communication Overhead:** Named pipes require system calls for each read and write operation. The overhead of switching between client and server processes adds latency to each chunk transfer.

**Sequential Processing:** The current implementation processes file chunks one at a time. The client must wait for each chunk response before requesting the next chunk, preventing any parallel processing.

**Memory Management:** Each chunk transfer involves allocating and deallocating buffers, which adds computational overhead especially for large files with many small chunks.

### Expected Performance Trend

The execution time generally increases with file size, but the rate of increase depends heavily on the buffer size used. Larger buffer sizes reduce the number of operations needed and improve overall performance. The relationship between file size and transfer time shows diminishing returns as buffer size increases because the fixed overhead per operation becomes less significant.

### Improvements for Better Performance

Several optimizations could improve file transfer performance:

- Increase buffer size using the `-m` flag to reduce the number of operations
- Implement parallel chunk processing instead of sequential transfers  
- Use more efficient IPC mechanisms for large file transfers
- Optimize memory allocation by reusing buffers

## Command Line Usage

The client supports the following command line options:

```bash
# Request single data point
./client -p <patient_id> -t <time> -e <ecg_number>

# Generate 1000 data points for a patient
./client -p <patient_id>

# Transfer a file
./client -f <filename>

# Use new channel for communication
./client -c [other options]

# Set buffer size
./client -m <buffer_size> [other options]
```

## Testing and Verification

The implementation passes all autograder tests including:
- Basic client-server connection
- Single data point requests
- Multi-data point CSV generation
- File transfer with different file sizes
- New channel creation
- Proper resource cleanup

All transferred files are verified using the `diff` command to ensure identical content between source and destination files.
