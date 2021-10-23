#pragma once
#include "ref.hpp"

namespace {

struct submodules {
    using initfunc = void (*)(ref module);

    static initfunc initfunctions[32];
    static int n;

    /**
     * @brief Execute all registered AT_SUBMODULE_INIT functions in order of declaration.
     */
    static void init(ref module) {
        for (int i = 0; i < n; i++) initfunctions[i](module);
    }
};

submodules::initfunc submodules::initfunctions[] = {};
int submodules::n = 0;

int operator+=(submodules, submodules::initfunc f) {
    submodules::initfunctions[submodules::n] = f;
    return submodules::n++;
}

#define AT_SUBMODULE_INIT3(counter) int _dummy_var_ ## counter = submodules{} += []
#define AT_SUBMODULE_INIT2(counter) AT_SUBMODULE_INIT3(counter)

/**
 * @brief Define a function that will run during module initialization.
 * @example ```
 * AT_SUBMODULE_INIT(ref module) {
 *     PyModule_AddObject(&*module, "Object", ...)
 * };
 * ```
 */
#define AT_SUBMODULE_INIT AT_SUBMODULE_INIT2(__COUNTER__)

} // namespace
