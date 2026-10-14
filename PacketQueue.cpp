#include "PacketQueue.h"
#include <algorithm>

PacketQueue::PacketQueue(std::size_t blockSize, std::size_t blockCount)
    : pool_(blockSize, blockCount),
      queue_(blockCount),
      maxBlockSize_(blockSize)
{
}

bool PacketQueue::receive(std::size_t id, const unsigned char* bytes, std::size_t length)
{
    if (bytes == nullptr || length == 0 || length > maxBlockSize_)
    {
        return false;
    }

    if (queue_.full() || pool_.availableBlocks() == 0)
    {
        return false;
    }

    void* rawBlock = pool_.allocate();
    if (rawBlock == nullptr)
    {
        return false;
    }

    unsigned char* blockPtr = static_cast<unsigned char*>(rawBlock);
    std::copy(bytes, bytes + length, blockPtr);

    PacketDescriptor desc{id, blockPtr, length};
    if (!queue_.enqueue(desc))
    {
        pool_.deallocate(rawBlock);
        return false;
    }

    return true;
}

bool PacketQueue::processNext(std::size_t& id, std::vector<unsigned char>& bytes)
{
    PacketDescriptor desc;
    if (!queue_.dequeue(desc))
    {
        return false;
    }

    bytes.assign(desc.blockPtr, desc.blockPtr + desc.length);
    id = desc.id;

    pool_.deallocate(desc.blockPtr);
    return true;
}

std::size_t PacketQueue::pending() const
{
    return queue_.size();
}

std::size_t PacketQueue::availableBlocks() const
{
    return pool_.availableBlocks();
}