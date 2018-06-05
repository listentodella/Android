
#include "Binder.h"
#include "Parcel.h"

BBinder::BBinder(){
}

const String16 & BBinder::getInterfaceDescriptor() const {
}

bool BBinder::isBinderAlive() const {
}

status_t BBinder::pingBinder() {
}

status_t BBinder::dump(int fd, const Vector<String16> & args) {
}

status_t BBinder::transact(uint32_t code, const Parcel & data, Parcel * reply, uint32_t flags) {
}

status_t BBinder::linkToDeath(const sp<DeathRecipient> & recipient, void * cookie, uint32_t flags) {
}

status_t BBinder::unlinkToDeath(const wp<DeathRecipient> & recipient, void * cookie, uint32_t flags, wp<DeathRecipient> * outRecipient) {
}

void BBinder::attachObject(const void * objectID, void * object, void * cleanupCookie, object_cleanup_func func) {
}

void * BBinder::findObject(const void * objectID) const {
}

void BBinder::detachObject(const void * objectID) {
}

BBinder * BBinder::localBinder() {
}

BBinder::~BBinder(){
}

status_t BBinder::onTransact(uint32_t code, const Parcel & data, Parcel * reply, uint32_t flags) {
}

BBinder::BBinder(const BBinder & o){
}

BBinder & BBinder::operator =(const BBinder & o) {
}

BpRefBase::BpRefBase(const sp<IBinder> & o){
}

BpRefBase::~BpRefBase(){
}

void BpRefBase::onFirstRef() {
}

void BpRefBase::onLastStrongRef(const void * id) {
}

bool BpRefBase::onIncStrongAttempted(uint32_t flags, const void * id) {
}

BpRefBase::BpRefBase(const BpRefBase & o){
}

BpRefBase & BpRefBase::operator =(const BpRefBase & o) {
}

