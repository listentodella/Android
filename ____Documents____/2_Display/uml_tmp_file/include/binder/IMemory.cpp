
#include "IMemory.h"
#include "Parcel.h"
#include "IBinder.h"

status_t BnMemoryHeap::onTransact(uint32_t code, const Parcel & data, Parcel * reply, uint32_t flags) {
}

BnMemoryHeap::BnMemoryHeap(){
}

BnMemoryHeap::~BnMemoryHeap(){
}

// helpers
void * IMemory::fastPointer(const sp<IBinder> & heap, ssize_t offset) const {
}

void * IMemory::pointer() const {
}

size_t IMemory::size() const {
}

ssize_t IMemory::offset() const {
}

status_t BnMemory::onTransact(uint32_t code, const Parcel & data, Parcel * reply, uint32_t flags) {
}

BnMemory::BnMemory(){
}

BnMemory::~BnMemory(){
}

