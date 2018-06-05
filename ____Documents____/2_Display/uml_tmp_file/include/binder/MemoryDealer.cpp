
#include "MemoryDealer.h"
#include "IMemory.h"
#include "SimpleBestFitAllocator.h"

MemoryDealer::MemoryDealer(size_t size, const char * name, uint32_t flags){
}

sp<IMemory> MemoryDealer::allocate(size_t size) {
}

void MemoryDealer::deallocate(size_t offset) {
}

void MemoryDealer::dump(const char * what) const {
}

MemoryDealer::~MemoryDealer(){
}

const sp<IMemoryHeap> & MemoryDealer::heap() const {
}

SimpleBestFitAllocator * MemoryDealer::allocator() const {
}

