#ifndef VECTRIXWORKSPACE_RESULT_H
#define VECTRIXWORKSPACE_RESULT_H
#include <string>

/**
 * @file Result.h
 * @brief Definition of VectrixResult, the value the engine reports an outcome with
 * @ingroup utils
 */

namespace Vectrix {
    /**
     * @brief What came out of an operation that can fail
     *
     * Returned by the parts of the engine that touch the disk, so the caller can tell a
     * missing file from a corrupted one. Always compare against #SUCCESS rather than
     * assuming a value.
     * @see toString(VectrixResult)
     * @ingroup utils
     */
    enum VectrixResult {
        SUCCESS,          ///< The operation went through
        UNKNOWN_ERROR,    ///< Something failed and could not be pinned down
        UNKNOWN_TYPE,     ///< The type asked for is not one the engine knows
        NOT_FOUND,        ///< What was asked for does not exist
        WRONG_TYPE,       ///< What was found is not of the expected type
        WRONG_FILE,       ///< The file is not what it claims to be
        OUTDATED,         ///< The file was written by a version that is no longer supported
        FORMATING_ERROR   ///< The content is of the right kind but is malformed
    };

    /**
     * @brief Turn a result into its name, for logging
     * @param r The result to describe
     * @return The name of the value, or a fallback when it is not a known one
     * @ingroup utils
     */
    inline std::string toString(VectrixResult r) {
        switch (r) {
            case SUCCESS: return "SUCCESS";
            case UNKNOWN_TYPE: return "UNKNOWN_TYPE";
            case NOT_FOUND: return "NOT_FOUND";
            case WRONG_TYPE: return "WRONG_TYPE";
            case UNKNOWN_ERROR: return "UNKNOWN_ERROR";
            case WRONG_FILE: return "WRONG_FILE";
            case OUTDATED: return "OUTDATED";
            case FORMATING_ERROR: return "FORMATING_ERROR";
        }
        return "UNKNOWN_RESULT_ASSET_LOADING";
    }
}

#endif //VECTRIXWORKSPACE_RESULT_H