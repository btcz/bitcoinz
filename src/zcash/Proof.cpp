#include "Proof.hpp"

#include "crypto/common.h"

#include <mutex>

namespace libzcash {

ProofVerifier ProofVerifier::Strict() {
    return ProofVerifier(true);
}

ProofVerifier ProofVerifier::Disabled() {
    return ProofVerifier(false);
}

}
