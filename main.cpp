#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include "Queue.h"
#include "PacketQueue.h"

void printStatus(const PacketQueue& pq, const std::string& label)
{
    std::cout << "[" << label << "] Pending packets: " << pq.pending()
              << " | Available blocks: " << pq.availableBlocks() << "\n";
}

int main()
{
    std::cout << "--- 1. Testing Queue<T> standalone (int & std::string) ---\n";
    Queue<int> intQ(3);
    int intVal = 0;
    assert(intQ.enqueue(10));
    assert(intQ.enqueue(20));
    assert(intQ.enqueue(30));
    assert(intQ.full());
    assert(!intQ.enqueue(40)); // Full reject
    assert(intQ.front(intVal) && intVal == 10);
    assert(intQ.dequeue(intVal) && intVal == 10);
    assert(intQ.enqueue(40)); // Circular wraparound check
    assert(intQ.size() == 3);

    Queue<std::string> strQ(2);
    std::string strVal;
    assert(strQ.enqueue("Hello"));
    assert(strQ.enqueue("World"));
    assert(strQ.dequeue(strVal) && strVal == "Hello");
    assert(strQ.dequeue(strVal) && strVal == "World");
    assert(!strQ.dequeue(strVal)); // Empty reject
    std::cout << "Queue<T> unit tests passed successfully!\n\n";

    std::cout << "--- 2. Demonstrating PacketQueue Arrival & Processing ---\n";
    // PacketQueue with 3 blocks of 64 bytes max
    PacketQueue pq(64, 3);
    printStatus(pq, "Initial state");

    // Binary payload with 0x00 and 0xFF
    unsigned char p1[] = {0x10, 0x00, 0xAB, 0xFF, 0x55};
    unsigned char p2[] = {0x20, 0x00, 0xFF, 0x44};
    unsigned char p3[] = {0x30, 0xAA, 0xBB, 0x00, 0xFF};

    std::cout << "Receiving Packets 1, 2, and 3...\n";
    assert(pq.receive(101, p1, sizeof(p1)));
    assert(pq.receive(102, p2, sizeof(p2)));
    assert(pq.receive(103, p3, sizeof(p3)));
    printStatus(pq, "After receiving 3 packets");

    std::cout << "\nAttempting to receive Packet 4 (Queue Overflow)...\n";
    unsigned char p4[] = {0x40, 0x41};
    bool overflowRes = pq.receive(104, p4, sizeof(p4));
    std::cout << "Receive result on full queue: " << (overflowRes ? "Success" : "Failed (Expected)") << "\n";
    printStatus(pq, "After overflow attempt");

    std::cout << "\nProcessing oldest packet (Should be ID 101)...\n";
    std::size_t outId = 0;
    std::vector<unsigned char> outBytes;
    assert(pq.processNext(outId, outBytes));
    std::cout << "Processed Packet ID: " << outId << " | Size: " << outBytes.size() << " bytes\n";
    printStatus(pq, "After processing ID 101");

    std::cout << "\nReceiving Packet 4 now that a block was freed (Demonstrating FIFO with LIFO Pool)...\n";
    assert(pq.receive(104, p4, sizeof(p4)));
    printStatus(pq, "After receiving ID 104");

    std::cout << "\nDraining remaining packets to verify FIFO sequence...\n";
    while (pq.pending() > 0)
    {
        pq.processNext(outId, outBytes);
        std::cout << "Processed Packet ID: " << outId << " [First byte: 0x" 
                  << std::hex << (int)outBytes[0] << std::dec << "]\n";
    }
    printStatus(pq, "Queue completely drained");

    std::cout << "\nAttempting processNext on empty queue...\n";
    outId = 999;
    bool underflowRes = pq.processNext(outId, outBytes);
    std::cout << "processNext on empty queue result: " << (underflowRes ? "Success" : "Failed (Expected)") << "\n";
    std::cout << "outId remains unchanged: " << outId << "\n";

    std::cout << "\n--- 3. Testing Boundary Conditions ---\n";
    // Single byte packet
    unsigned char minP[] = {0x7F};
    assert(pq.receive(201, minP, 1));
    pq.processNext(outId, outBytes);
    assert(outBytes.size() == 1 && outBytes[0] == 0x7F);

    // Oversized packet rejection
    unsigned char maxP[65] = {0};
    assert(!pq.receive(202, maxP, 65)); // Exceeds block size 64

    std::cout << "All packet queue demonstrations completed successfully!\n";
    return 0;
}