
#include "ProcessState.h"
#include "IBinder.h"

sp<ProcessState> ProcessState::self()
{
}

void ProcessState::setContextObject(const sp<IBinder> & object) {
}

sp<IBinder> ProcessState::getContextObject(const sp<IBinder> & caller) {
}

void ProcessState::setContextObject(const sp<IBinder> & object, const String16 & name) {
}

sp<IBinder> ProcessState::getContextObject(const String16 & name, const sp<IBinder> & caller) {
}

void ProcessState::startThreadPool() {
}

bool ProcessState::isContextManager(void ) const {
}

bool ProcessState::becomeContextManager(ProcessState::context_check_func checkFunc, void * userData) {
}

sp<IBinder> ProcessState::getStrongProxyForHandle(int32_t handle) {
}

wp<IBinder> ProcessState::getWeakProxyForHandle(int32_t handle) {
}

void ProcessState::expungeHandle(int32_t handle, IBinder * binder) {
}

void ProcessState::spawnPooledThread(bool isMain) {
}

status_t ProcessState::setThreadPoolMaxThreadCount(size_t maxThreads) {
}

void ProcessState::giveThreadPoolName() {
}

ProcessState::ProcessState(){
}

ProcessState::~ProcessState(){
}

ProcessState::ProcessState(const ProcessState & o){
}

ProcessState & ProcessState::operator =(const ProcessState & o) {
}

String8 ProcessState::makeBinderThreadName() {
}

ProcessState::handle_entry * ProcessState::lookupHandleLocked(int32_t handle) {
}

