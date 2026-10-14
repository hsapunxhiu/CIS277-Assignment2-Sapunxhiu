#include "MemoryPool.h"

MemoryPool::MemoryPool(size_t blockSize, size_t blockCount)
    : m_blockSize(blockSize),
      m_blockCount(blockCount),
      m_poolBuffer(nullptr),
      m_isAllocated(blockCount, false)
{
    if (m_blockSize > 0 && m_blockCount > 0) {
        m_poolBuffer = new uint8_t[m_blockSize * m_blockCount];

        for (size_t i = 0; i < m_blockCount; ++i) {
            void* blockPtr = static_cast<void*>(m_poolBuffer + (i * m_blockSize));
            m_freeStack.push(blockPtr);
        }
    }
}

MemoryPool::~MemoryPool() {
    delete[] m_poolBuffer;
}

std::ptrdiff_t MemoryPool::getBlockIndex(void* ptr) const {
    if (!ptr || !m_poolBuffer) {
        return -1;
    }

    const uint8_t* bytePtr = static_cast<const uint8_t*>(ptr);
    
    if (bytePtr < m_poolBuffer || bytePtr >= m_poolBuffer + capacity()) {
        return -1;
    }

    std::ptrdiff_t offset = bytePtr - m_poolBuffer;

    if (offset % m_blockSize != 0) {
        return -1;
    }

    return offset / static_cast<std::ptrdiff_t>(m_blockSize);
}

void* MemoryPool::allocate() {
    if (m_freeStack.empty()) {
        return nullptr;
    }

    void* ptr = m_freeStack.pop();
    std::ptrdiff_t index = getBlockIndex(ptr);
    
    if (index >= 0) {
        m_isAllocated[index] = true;
    }

    return ptr;
}

bool MemoryPool::deallocate(void* ptr) {
    std::ptrdiff_t index = getBlockIndex(ptr);

    if (index < 0) {
        return false;
    }

    if (!m_isAllocated[index]) {
        return false;
    }

    m_isAllocated[index] = false;
    m_freeStack.push(ptr);

    return true;
}

size_t MemoryPool::availableBlocks() const {
    return m_freeStack.size();
}

size_t MemoryPool::allocatedBlocks() const {
    return m_blockCount - m_freeStack.size();
}

size_t MemoryPool::blockSize() const {
    return m_blockSize;
}

size_t MemoryPool::capacity() const {
    return m_blockSize * m_blockCount;
}