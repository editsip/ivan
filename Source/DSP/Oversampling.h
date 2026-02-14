#pragma once
#include <juce_dsp/juce_dsp.h>

namespace ivan {

// ============================================================================
// Oversampling wrapper
//
// Tube distortion generates harmonics that can alias badly at normal sample
// rates. 4x oversampling with steep anti-aliasing filters is the standard
// approach. We use JUCE's built-in oversampling which provides polyphase
// FIR filters with configurable steepness.
//
// The oversampling factor can be adjusted per quality setting:
// - Draft: 2x (low latency, some aliasing)
// - Normal: 4x (good balance)
// - HQ: 8x (near-perfect, higher CPU)
// ============================================================================

enum class OversamplingQuality {
    Draft,    // 2x oversampling
    Normal,   // 4x oversampling
    HQ        // 8x oversampling
};

inline int getOversamplingFactor(OversamplingQuality q) {
    switch (q) {
        case OversamplingQuality::Draft:  return 1;  // 2^1 = 2x
        case OversamplingQuality::Normal: return 2;  // 2^2 = 4x
        case OversamplingQuality::HQ:     return 3;  // 2^3 = 8x
        default: return 2;
    }
}

} // namespace ivan
