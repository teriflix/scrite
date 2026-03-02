#ifndef SONNET_VERSION_H
#define SONNET_VERSION_H

#define SONNET_VERSION_STRING ScriteSonnetCore_VERSION_STR
#define SONNET_VERSION_MAJOR ScriteSonnetCore_VERSION_MAJOR
#define SONNET_VERSION_MINOR ScriteSonnetCore_VERSION_MINOR
#define SONNET_VERSION_PATCH ScriteSonnetCore_VESION_REVISION
#define SONNET_VERSION                                                                             \
    ((SONNET_VERSION_MAJOR << 16) | (SONNET_VERSION_MINOR << 8) | (SONNET_VERSION_PATCH))

#endif
