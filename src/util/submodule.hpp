#pragma once
namespace {

struct submodule {
    using initfunc = void (*)(bool* module);

    static initfunc initfunctions[32];
    static int n;

    static void exec() {
        for (int i = 0; i < n; i++) initfunctions[i](nullptr);
    }
};

submodule::initfunc submodule::initfunctions[] = {};
int submodule::n = 0;

int operator+=(submodule, submodule::initfunc f) {
    submodule::initfunctions[submodule::n] = f;
    return submodule::n++;
}

#define at_submodule_init3(counter) int _dummy_var_ ## counter = submodule{} += [](bool* module)
#define at_submodule_init2(counter) at_submodule_init3(counter)
#define at_submodule_init at_submodule_init2(__COUNTER__)

#undef at_submodule_init2
#undef at_submodule_init3

} // namespace
