
#include "MemoryHeapBase.h"

//     * maps the memory referenced by fd. but DOESN'T take ownership
//     * of the filedescriptor (it makes a copy with dup()
MemoryHeapBase::MemoryHeapBase(int fd, size_t size, uint32_t flags, uint32_t offset){
}

//     * maps memory from the given device
MemoryHeapBase::MemoryHeapBase(const char * device, size_t size, uint32_t flags){
}

//     * maps memory from ashmem, with the given name for debugging
MemoryHeapBase::MemoryHeapBase(size_t size, uint32_t flags, const char * name){
}

MemoryHeapBase::~MemoryHeapBase(){
}

// implement IMemoryHeap interface 
int MemoryHeapBase::getHeapID() const {
}

// virtual address of the heap. returns MAP_FAILED in case of error 
void * MemoryHeapBase::getBase() const {
}

size_t MemoryHeapBase::getSize() const {
}

uint32_t MemoryHeapBase::getFlags() const {
}

uint32_t MemoryHeapBase::getOffset() const {
}

const char * MemoryHeapBase::getDevice() const {
}

// this closes this heap -- use carefully 
void MemoryHeapBase::dispose() {
}

MemoryHeapBase::MemoryHeapBase(){
}

// init() takes ownership of fd
status_t MemoryHeapBase::init(int fd, void * base, int size, int flags, const char * device) {
}

status_t MemoryHeapBase::mapfd(int fd, size_t size, uint32_t offset) {
}

