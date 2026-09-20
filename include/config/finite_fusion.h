#ifndef GUARD_CONFIG_FINITE_FUSION_H
#define GUARD_CONFIG_FINITE_FUSION_H

// Opt-in saved COPY sandbox, not live-party fusion. Build from clean output with
// make FF_NATIVE_DEBUG=1. Use a NEW, disposable save for this distinct layout.
#ifndef FF_NATIVE_DEBUG
#define FF_NATIVE_DEBUG 0
#endif

#if FF_NATIVE_DEBUG && defined(RELEASE)
#error Finite Fusion native sandbox is not supported in release builds
#endif

#endif
