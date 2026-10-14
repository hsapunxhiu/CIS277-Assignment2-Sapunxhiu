#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#include <cstddef>
#include <vector>
#include "MemoryPool.h"
#include "Queue.h"

struct PacketDescriptor
{
    std::size_t id;
    unsigned char* blockPtr;
    std::size_t length;
};

class PacketQueue
{
public:
    PacketQueue(std::size_t blockSize, std::size_t blockCount);

    bool receive(std::size_t id, const unsigned char* bytes, std::size_t length);
    bool processNext(std::size_t& id, std::vector<unsigned char>& bytes);

    std::size_t pending() const;
    std::size_t availableBlocks() const;

private:
    MemoryPool pool_;
    Queue<PacketDescriptor> queue_;
    std::size_t maxBlockSize_;
};

#endif // PACKET_QUEUE_H