#include "versionInfo.h"
#include <stddef.h>

bool versionChanged(const volatile VersionInfo *versionInfo){
    bool major_version_changed = versionInfo->major != MAJOR_REV;
    bool minor_version_changed = versionInfo->minor != MINOR_REV;
    bool changed = (major_version_changed || minor_version_changed);
    return changed;
}
