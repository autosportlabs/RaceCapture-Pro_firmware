#ifndef VERSIONINFO_H_
#define VERSIONINFO_H_
#include <stdint.h>
#include <stdbool.h>

typedef struct _VersionInfo {
    uint32_t major;
    uint32_t minor;
    uint32_t bugfix;
} VersionInfo;

#define DEFAULT_VERSION_INFO {MAJOR_REV, MINOR_REV, BUGFIX_REV}

bool versionChanged(const volatile VersionInfo *versionInfo);

#endif /*VERSIONINFO_H_*/
