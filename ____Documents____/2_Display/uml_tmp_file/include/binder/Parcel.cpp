
#include "Parcel.h"
#include "IPCThreadState.h"
#include "IBinder.h"
#include "TextOutput.h"

Parcel::Parcel(){
}

Parcel::~Parcel(){
}

const uint8_t * Parcel::data() const {
}

size_t Parcel::dataSize() const {
}

size_t Parcel::dataAvail() const {
}

size_t Parcel::dataPosition() const {
}

size_t Parcel::dataCapacity() const {
}

status_t Parcel::setDataSize(size_t size) {
}

void Parcel::setDataPosition(size_t pos) const {
}

status_t Parcel::setDataCapacity(size_t size) {
}

status_t Parcel::setData(const uint8_t * buffer, size_t len) {
}

status_t Parcel::appendFrom(const Parcel * parcel, size_t start, size_t len) {
}

bool Parcel::pushAllowFds(bool allowFds) {
}

void Parcel::restoreAllowFds(bool lastValue) {
}

bool Parcel::hasFileDescriptors() const {
}

// Writes the RPC header.
status_t Parcel::writeInterfaceToken(const String16 & interface) {
}

// Parses the RPC header, returning true if the interface name
// in the header matches the expected interface from the caller.
//
// Additionally, enforceInterface does part of the work of
// propagating the StrictMode policy mask, populating the current
// IPCThreadState, which as an optimization may optionally be
// passed in.
bool Parcel::enforceInterface(const String16 & interface, IPCThreadState * threadState) const {
}

bool Parcel::checkInterface(IBinder * ) const {
}

void Parcel::freeData() {
}

const binder_size_t * Parcel::objects() const {
}

size_t Parcel::objectsCount() const {
}

status_t Parcel::errorCheck() const {
}

void Parcel::setError(status_t err) {
}

status_t Parcel::write(const void * data, size_t len) {
}

void * Parcel::writeInplace(size_t len) {
}

status_t Parcel::writeUnpadded(const void * data, size_t len) {
}

status_t Parcel::writeInt32(int32_t val) {
}

status_t Parcel::writeInt64(int64_t val) {
}

status_t Parcel::writeFloat(float val) {
}

status_t Parcel::writeDouble(double val) {
}

status_t Parcel::writeCString(const char * str) {
}

status_t Parcel::writeString8(const String8 & str) {
}

status_t Parcel::writeString16(const String16 & str) {
}

status_t Parcel::writeString16(const char16_t * str, size_t len) {
}

status_t Parcel::writeStrongBinder(const sp<IBinder> & val) {
}

status_t Parcel::writeWeakBinder(const wp<IBinder> & val) {
}

status_t Parcel::writeInt32Array(size_t len, const int32_t * val) {
}

status_t Parcel::writeByteArray(size_t len, const uint8_t * val) {
}

// ---------------------------------------------------------------------------

template<typename T>
status_t Parcel::write(const Flattenable<T> & val) {
    const FlattenableHelper<T> helper(val);
    return write(helper);
}

template<typename T>
status_t Parcel::write(const LightFlattenable<T> & val) {
    size_t size(val.getFlattenedSize());
    if (!val.isFixedSize()) {
        status_t err = writeInt32(size);
        if (err != NO_ERROR) {
            return err;
        }
    }
    if (size) {
        void* buffer = writeInplace(size);
        if (buffer == NULL)
            return NO_MEMORY;
        return val.flatten(buffer, size);
    }
    return NO_ERROR;
}

// Place a native_handle into the parcel (the native_handle's file-
// descriptors are dup'ed, so it is safe to delete the native_handle
// when this function returns). 
// Doesn't take ownership of the native_handle.
status_t Parcel::writeNativeHandle(const native_handle * handle) {
}

// Place a file descriptor into the parcel.  The given fd must remain
// valid for the lifetime of the parcel.
// The Parcel does not take ownership of the given fd unless you ask it to.
status_t Parcel::writeFileDescriptor(int fd, bool takeOwnership) {
}

// Place a file descriptor into the parcel.  A dup of the fd is made, which
// will be closed once the parcel is destroyed.
status_t Parcel::writeDupFileDescriptor(int fd) {
}

// Writes a raw fd and optional comm channel fd to the parcel as a ParcelFileDescriptor.
// A dup's of the fds are made, which will be closed once the parcel is destroyed.
// Null values are passed as -1.
status_t Parcel::writeParcelFileDescriptor(int fd, int commChannel) {
}

// Writes a blob to the parcel.
// If the blob is small, then it is stored in-place, otherwise it is
// transferred by way of an anonymous shared memory region.
// The caller should call release() on the blob after writing its contents.
status_t Parcel::writeBlob(size_t len, Parcel::WritableBlob * outBlob) {
}

status_t Parcel::writeObject(const flat_binder_object & val, bool nullMetaData) {
}

// Like Parcel.java's writeNoException().  Just writes a zero int32.
// Currently the native implementation doesn't do any of the StrictMode
// stack gathering and serialization that the Java implementation does.
status_t Parcel::writeNoException() {
}

void Parcel::remove(size_t start, size_t amt) {
}

status_t Parcel::read(void * outData, size_t len) const {
}

const void * Parcel::readInplace(size_t len) const {
}

int32_t Parcel::readInt32() const {
}

status_t Parcel::readInt32(int32_t * pArg) const {
}

int64_t Parcel::readInt64() const {
}

status_t Parcel::readInt64(int64_t * pArg) const {
}

float Parcel::readFloat() const {
}

status_t Parcel::readFloat(float * pArg) const {
}

double Parcel::readDouble() const {
}

status_t Parcel::readDouble(double * pArg) const {
}

intptr_t Parcel::readIntPtr() const {
}

status_t Parcel::readIntPtr(intptr_t * pArg) const {
}

const char * Parcel::readCString() const {
}

String8 Parcel::readString8() const {
}

String16 Parcel::readString16() const {
}

const char16_t * Parcel::readString16Inplace(size_t * outLen) const {
}

sp<IBinder> Parcel::readStrongBinder() const {
}

wp<IBinder> Parcel::readWeakBinder() const {
}

template<typename T>
status_t Parcel::read(Flattenable<T> & val) const {
    FlattenableHelper<T> helper(val);
    return read(helper);
}

template<typename T>
status_t Parcel::read(LightFlattenable<T> & val) const {
    size_t size;
    if (val.isFixedSize()) {
        size = val.getFlattenedSize();
    } else {
        int32_t s;
        status_t err = readInt32(&s);
        if (err != NO_ERROR) {
            return err;
        }
        size = s;
    }
    if (size) {
        void const* buffer = readInplace(size);
        return buffer == NULL ? NO_MEMORY :
                val.unflatten(buffer, size);
    }
    return NO_ERROR;
}

// Like Parcel.java's readExceptionCode().  Reads the first int32
// off of a Parcel's header, returning 0 or the negative error
// code on exceptions, but also deals with skipping over rich
// response headers.  Callers should use this to read & parse the
// response headers rather than doing it by hand.
int32_t Parcel::readExceptionCode() const {
}

// Retrieve native_handle from the parcel. This returns a copy of the
// parcel's native_handle (the caller takes ownership). The caller
// must free the native_handle with native_handle_close() and 
// native_handle_delete().
native_handle * Parcel::readNativeHandle() const {
}

// Retrieve a file descriptor from the parcel.  This returns the raw fd
// in the parcel, which you do not own -- use dup() to get your own copy.
int Parcel::readFileDescriptor() const {
}

// Reads a ParcelFileDescriptor from the parcel.  Returns the raw fd as
// the result, and the optional comm channel fd in outCommChannel.
// Null values are returned as -1.
int Parcel::readParcelFileDescriptor(int & outCommChannel) const {
}

// Reads a blob from the parcel.
// The caller should call release() on the blob after reading its contents.
status_t Parcel::readBlob(size_t len, Parcel::ReadableBlob * outBlob) const {
}

const flat_binder_object * Parcel::readObject(bool nullMetaData) const {
}

// Explicitly close all file descriptors in the parcel.
void Parcel::closeFileDescriptors() {
}

uintptr_t Parcel::ipcData() const {
}

size_t Parcel::ipcDataSize() const {
}

uintptr_t Parcel::ipcObjects() const {
}

size_t Parcel::ipcObjectsCount() const {
}

void Parcel::ipcSetDataReference(const uint8_t * data, size_t dataSize, const binder_size_t * objects, size_t objectsCount, Parcel::release_func relFunc, void * relCookie) {
}

void Parcel::print(TextOutput & to, uint32_t flags) const {
}

Parcel::Parcel(const Parcel & o){
}

Parcel & Parcel::operator =(const Parcel & o) {
}

status_t Parcel::finishWrite(size_t len) {
}

void Parcel::releaseObjects() {
}

void Parcel::acquireObjects() {
}

status_t Parcel::growData(size_t len) {
}

status_t Parcel::restartWrite(size_t desired) {
}

status_t Parcel::continueWrite(size_t desired) {
}

status_t Parcel::writePointer(uintptr_t val) {
}

status_t Parcel::readPointer(uintptr_t * pArg) const {
}

uintptr_t Parcel::readPointer() const {
}

void Parcel::freeDataNoInit() {
}

void Parcel::initState() {
}

void Parcel::scanForFds() const {
}

status_t Parcel::readAligned(T * pArg) const {
}

T Parcel::readAligned() const {
}

status_t Parcel::writeAligned(T val) {
}

Parcel::Blob::Blob(){
}

Parcel::Blob::~Blob(){
}

void Parcel::Blob::release() {
}

void Parcel::Blob::init(bool mapped, void * data, size_t size) {
}

void Parcel::Blob::clear() {
}

status_t Parcel::write(const Parcel::FlattenableHelperInterface & val) {
}

status_t Parcel::read(Parcel::FlattenableHelperInterface & val) const {
}

