
#include "IBinder.h"
#include "IInterface.h"
#include "Parcel.h"
#include "BpBinder.h"

IBinder::IBinder(){
}

//*
//     * Check if this IBinder implements the interface named by
//     * @a descriptor.  If it does, the base pointer to it is returned,
//     * which you can safely static_cast<> to the concrete C++ interface.
sp<IInterface> IBinder::queryLocalInterface(const String16 & descriptor) {
}

bool IBinder::checkSubclass(const void * subclassID) const {
}

BBinder * IBinder::localBinder() {
}

BpBinder * IBinder::remoteBinder() {
}

IBinder::~IBinder(){
}

