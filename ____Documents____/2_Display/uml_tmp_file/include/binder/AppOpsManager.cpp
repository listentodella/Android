
#include "AppOpsManager.h"
#include "IAppOpsCallback.h"
#include "IAppOpsService.h"

AppOpsManager::AppOpsManager(){
}

int32_t AppOpsManager::checkOp(int32_t op, int32_t uid, const String16 & callingPackage) {
}

int32_t AppOpsManager::noteOp(int32_t op, int32_t uid, const String16 & callingPackage) {
}

int32_t AppOpsManager::startOp(int32_t op, int32_t uid, const String16 & callingPackage) {
}

void AppOpsManager::finishOp(int32_t op, int32_t uid, const String16 & callingPackage) {
}

void AppOpsManager::startWatchingMode(int32_t op, const String16 & packageName, const sp<IAppOpsCallback> & callback) {
}

void AppOpsManager::stopWatchingMode(const sp<IAppOpsCallback> & callback) {
}

sp<IAppOpsService> AppOpsManager::getService() {
}

