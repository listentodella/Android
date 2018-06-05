
#include "MemoryBase.h"

MemoryBase::MemoryBase(const sp<IMemoryHeap> & heap, ssize_t offset, size_t size){
}

MemoryBase::~MemoryBase(){
}

sp<IMemoryHeap> MemoryBase::getMemory(ssize_t * offset, size_t * size) const {
}

