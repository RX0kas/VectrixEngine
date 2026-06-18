#ifndef VECTRIXWORKSPACE_RESULT_H
#define VECTRIXWORKSPACE_RESULT_H
#include <string>

namespace Vectrix {
    enum VectrixResult {
        SUCCESS,
        UNKNOWN_ERROR,
        UNKNOWN_TYPE,
        NOT_FOUND,
        WRONG_TYPE,
        WRONG_FILE,
        OUTDATED
    };

    inline std::string toString(VectrixResult r) {
        switch (r) {
            case SUCCESS: return "SUCCESS";
            case UNKNOWN_TYPE: return "UNKNOWN_TYPE";
            case NOT_FOUND: return "NOT_FOUND";
            case WRONG_TYPE: return "WRONG_TYPE";
            case UNKNOWN_ERROR: return "UNKNOWN_ERROR";
            case WRONG_FILE: return "WRONG_FILE";
            case OUTDATED: return "OUTDATED";
        }
        return "UNKNOWN_RESULT_ASSET_LOADING";
    }
}

#endif //VECTRIXWORKSPACE_RESULT_H