#ifndef SONNET_CORE_EXPORT_H
#define SONNET_CORE_EXPORT_H

#ifdef SONNETCORE_STATIC_DEFINE
#  define SONNETCORE_EXPORT
#  define SONNETCORE_NO_EXPORT
#else
#  ifndef SONNETCORE_EXPORT
#    ifdef ScriteSonnetCore_EXPORTS
        /* We are building this library */
#      define SONNETCORE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SONNETCORE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SONNETCORE_NO_EXPORT
#    define SONNETCORE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SONNETCORE_DECL_DEPRECATED
#  define SONNETCORE_DECL_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SONNETCORE_DECL_DEPRECATED_EXPORT
#  define SONNETCORE_DECL_DEPRECATED_EXPORT SONNETCORE_EXPORT SONNETCORE_DECL_DEPRECATED
#endif

#ifndef SONNETCORE_DECL_DEPRECATED_NO_EXPORT
#  define SONNETCORE_DECL_DEPRECATED_NO_EXPORT SONNETCORE_NO_EXPORT SONNETCORE_DECL_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SONNETCORE_NO_DEPRECATED
#    define SONNETCORE_NO_DEPRECATED
#  endif
#endif

#include <sonnet_version.h>

#endif
