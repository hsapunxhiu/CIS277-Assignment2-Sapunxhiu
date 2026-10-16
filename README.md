# CIS-277 Assignment 2: Network Packet Arrival Queue

## Student
* **Name:** Hana Sapunxhiu

## Design
The network packet system consists of two primary layers: memory management (`MemoryPool`) and order tracking (`Queue`).
* **Circular Array Queue (`Queue<T>`):** Implemented using a fixed-capacity `std::vector<T>` storage container. The queue maintains a `front_` index and a `count_` tracking total elements. The insertion index (`back`) is derived using the formula: `(front_ + count_) % capacity_`. This guarantees structural $O(1)$ dynamic push and pop operations without needing array memory shifts.
* **Separation of Responsibilities:** `PacketQueue` delegates byte storage management to `MemoryPool` and arrival ordering to `Queue<PacketDescriptor>`. Descriptors store only primitive fields (`id`, `length`, and `blockPtr`), preventing redundant memory duplications while in transit.

## How to Compile
Compile using any standard C++23 compliant compiler (e.g., GCC 13+ or Clang 16+):
bash
g++ -std=c++23 -Wall -Wextra -pedantic main.cpp PacketQueue.cpp MemoryPool.cpp -o packet_queue'

## How to Run
Execute the compiled binary directly from the terminal: ./packet_queue

## Tests and Results
PacketQueue Arrival & Processing Tests:
[Initial state] Pending packets: 0 | Available blocks: 3
Receiving Packets 1, 2, and 3...
[After receiving 3 packets] Pending packets: 3 | Available blocks: 0

Attempting to receive Packet 4 (Queue Overflow)...
Receive result on full queue: Failed (Expected)

Processing oldest packet:
Processed Packet ID: 101 | Size: 5 bytes
[After processing ID 101] Pending packets: 2 | Available blocks: 1

Receiving Packet 4 now that a block was freed (Demonstrating FIFO with LIFO Pool)...
[After receiving ID 104] Pending packets: 3 | Available blocks: 0

Draining remaining packets to verify FIFO sequence...
Processed Packet ID: 102 [First byte: 0x20]
Processed Packet ID: 103 [First byte: 0x30]
Processed Packet ID: 104 [First byte: 0x40]
[Queue completely drained] Pending packets: 0 | Available blocks: 3

Attempting processNext on empty queue...
processNext on empty queue result: Failed (Expected)
outId remains unchanged: 999

## Analysis Questions
1. Why must packet service be FIFO even though free-block reuse is LIFO?
Network packet delivery requires strict fairness: earlier arriving packets must be processed before newly arrived ones to prevent starvation and latency spikes. The LIFO behavior of the memory pool is purely an internal memory optimization designed to maximize cache locality for recently freed memory addresses; it does not dictate packet arrival sequencing.

2. How does your circular array wrap without shifting all waiting packets?
Instead of shifting remaining elements toward index 0 upon dequeue, the front index advances via (front_ + 1) % capacity_. When adding new items, the write index is calculated directly using (front_ + count_) % capacity_. Modulo arithmetic wraps indices back to index 0 when reaching the buffer end, preserving O(1) operations without memory shifts.

3. What happens at queue overflow and underflow, and why must those attempts leave state unchanged?
Overflow: Occurs when calling enqueue() on a full queue or receive() with no available pool blocks. The operation immediately returns false.
Underflow: Occurs when calling dequeue() or processNext() on an empty queue. The operation immediately returns false and leaves output variables intact.
Leaving state unchanged guarantees strong exception safety and system atomicity—failed operational attempts do not leak pool blocks, corrupt index counters, or erase input parameters.

4. What are the time complexities of queue operations and of receive/processNext when the packet has length bytes?
Raw Queue<T> operations (enqueue, dequeue, front) take O(1) time complexity because index arithmetic and array lookups are constant time.
receive and processNext take O(length) time complexity because they perform explicit byte copies (std::copy / std::vector::assign) to transfer packet payloads into and out of the memory pool blocks.

5. Why must processNext copy bytes out before returning the block to the pool?
Returning the block to the MemoryPool makes that memory slice immediately eligible for reallocation by a subsequent receive() call. If the block were deallocated prior to copying out the packet payload, incoming thread writes could overwrite the data mid-processing, leading to dangling pointers, race conditions, and data corruption.

## AI Use
AI was utilized to verify edge-case coverage and generate unit test validation scripts. All logic, pointers, and memory boundary guarantees were manually verified against the assignment specifications.
