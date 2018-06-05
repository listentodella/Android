
#include "BpBinder.h"
#include "Parcel.h"

BpBinder::BpBinder(int32_t handle){
}

const String16 & BpBinder::getInterfaceDescriptor() const {
}

bool BpBinder::isBinderAlive() const {
}

status_t BpBinder::pingBinder() {
}

status_t BpBinder::dump(int fd, const Vector<String16> & args) {
}

status_t BpBinder::transact(uint32_t code, const Parcel & data, Parcel * reply, uint32_t flags) {
}

status_t BpBinder::linkToDeath(const sp<DeathRecipient> & recipient, void * cookie, uint32_t flags) {
}

status_t BpBinder::unlinkToDeath(const wp<DeathRecipient> & recipient, void * cookie, uint32_t flags, wp<DeathRecipient> * outRecipient) {
}

void BpBinder::attachObject(const void * objectID, void * object, void * cleanupCookie, object_cleanup_func func) {
}

void * BpBinder::findObject(const void * objectID) const {
}

void BpBinder::detachObject(const void * objectID) {
}

BpBinder * BpBinder::remoteBinder() {
}

status_t BpBinder::setConstantData(const void * data, size_t size) {
}

void BpBinder::sendObituary() {
}

BpBinder::ObjectManager::ObjectManager(){
}

BpBinder::ObjectManager::~ObjectManager(){
}

void BpBinder::ObjectManager::attach(const void * objectID, void * object, void * cleanupCookie, IBinder::object_cleanup_func func) {
}

void * BpBinder::ObjectManager::find(const void * objectID) const {
}

void BpBinder::ObjectManager::detach(const void * objectID) {
}

void BpBinder::ObjectManager::kill() {
}

BpBinder::ObjectManager::ObjectManager(const BpBinder::ObjectManager & ){
}

BpBinder::ObjectManager & BpBinder::ObjectManager::operator =(const BpBinder::ObjectManager & ) {
}

BpBinder::~BpBinder(){
}

void BpBinder::onFirstRef() {
}

void BpBinder::onLastStrongRef(const void * id) {
}

bool BpBinder::onIncStrongAttempted(uint32_t flags, const void * id) {
}

void BpBinder::reportOneDeath(const BpBinder::Obituary & obit) {
}

bool BpBinder::isDescriptorCached() const {
}

