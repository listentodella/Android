
#include "PermissionCache.h"

// free the whole cache, but keep the permission name pool
void PermissionCache::purge() {
}

status_t PermissionCache::check(bool * granted, const String16 & permission, uid_t uid) const {
}

void PermissionCache::cache(const String16 & permission, uid_t uid, bool granted) {
}

PermissionCache::PermissionCache(){
}

bool PermissionCache::checkCallingPermission(const String16 & permission)
{
}

bool PermissionCache::checkCallingPermission(const String16 & permission, int32_t * outPid, int32_t * outUid)
{
}

bool PermissionCache::checkPermission(const String16 & permission, pid_t pid, uid_t uid)
{
}

