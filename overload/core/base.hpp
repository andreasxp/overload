/// Various helper methods and macros for simplifying writing python extensions.
#pragma once
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <array>
#include <memory>

// Include ciso646 because for whatever reason MSVC does not enable "and" and "or" keywords by default
#include <ciso646>

#include "flat_hash_map.hpp"
using ska::flat_hash_map;
using ska::flat_hash_set;

#include "span.hpp"
using tcb::span;

using ssize = Py_ssize_t;

// Memory management ===================================================================================================
using ref = PyObject*;

namespace impl {

/**
 * @brief Custom deleter for PyObject, for use in uref.
 * @see https://docs.python.org/3/c-api/refcounting.html#c.Py_DECREF
 */
struct Deleter {
	void operator()(ref ptr) const noexcept {
		Py_DECREF(ptr);
	}
};

} // namespace impl

/// A unique pointer to a PyObject, which is cleaned up automatically.
using uref = std::unique_ptr<PyObject, impl::Deleter>;

// Function wrappers ===================================================================================================
uref getattr(ref object, const char* attrname) {
	return uref{PyObject_GetAttrString(object, attrname)};
}

uref getattr(const uref& object, const char* attrname) {
	return uref{PyObject_GetAttrString(object.get(), attrname)};
}

void setattr(ref object, const char* attrname, ref value) {
	PyObject_SetAttrString(object, attrname, value);
}

void setattr(const uref& object, const char* attrname, ref value) {
	PyObject_SetAttrString(object.get(), attrname, value);
}

uref import(const char* modulename) {
	return uref{PyImport_ImportModule(modulename)};
}

uref import_from(const char* modulename, const char* name) {
	uref mod = import(modulename);
	return getattr(mod, name);
}

void print(const char* text) {
	uref methodPrint = import_from("builtins", "print");
	uref result {PyObject_CallFunction(&*methodPrint, "s", text)};
}

void print(ref object) {
	uref methodPrint = import_from("builtins", "print");
	uref result {PyObject_CallFunction(&*methodPrint, "O", object)};
}

// Conversion ==========================================================================================================
uref to_list(std::vector<ref>& vec) {
	uref result {PyList_New(vec.size())};

	for (size_t i = 0; i < vec.size(); i++) {
		// Note: PyList_SET_ITEM steals a reference
		Py_INCREF(vec[i]);
		PyList_SET_ITEM(&*result, i, vec[i]);
	}

	return result;
}

// PARSEARGS macro =====================================================================================================
#define PP_NARG(...) PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)

#define PP_ARG_N(                                                                                                      \
	_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
	_25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, \
	_48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, N, ...) N

#define PP_RSEQ_N()                                                                                                    \
	63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36,    \
	35,	34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7,   \
	6, 5, 4, 3, 2, 1, 0

#define FE_CALLITn01(a,b) a b
#define FE_CALLITn02(a,b) a b
#define FE_CALLITn03(a,b) a b
#define FE_CALLITn04(a,b) a b
#define FE_CALLITn04(a,b) a b
#define FE_CALLITn05(a,b) a b
#define FE_CALLITn06(a,b) a b
#define FE_CALLITn07(a,b) a b
#define FE_CALLITn08(a,b) a b
#define FE_CALLITn09(a,b) a b
#define FE_CALLITn10(a,b) a b
#define FE_CALLITn11(a,b) a b
#define FE_CALLITn12(a,b) a b
#define FE_CALLITn13(a,b) a b
#define FE_CALLITn14(a,b) a b
#define FE_CALLITn15(a,b) a b
#define FE_CALLITn16(a,b) a b
#define FE_CALLITn17(a,b) a b
#define FE_CALLITn18(a,b) a b
#define FE_CALLITn19(a,b) a b
#define FE_CALLITn20(a,b) a b
#define FE_CALLITn21(a,b) a b

/* the MSVC preprocessor expands __VA_ARGS__ as a single argument, so it needs
 * to be expanded indirectly through the CALLIT macros.
 * http://connect.microsoft.com/VisualStudio/feedback/details/380090/variadic-macro-replacement
 * http://stackoverflow.com/questions/21869917/visual-studio-va-args-issue
 */
#define FE_n00()
#define FE_n01(what, a, ...)  what(a)
#define FE_n02(what, a, ...)  what(a) FE_CALLITn02(FE_n01,(what, ##__VA_ARGS__))
#define FE_n03(what, a, ...)  what(a) FE_CALLITn03(FE_n02,(what, ##__VA_ARGS__))
#define FE_n04(what, a, ...)  what(a) FE_CALLITn04(FE_n03,(what, ##__VA_ARGS__))
#define FE_n05(what, a, ...)  what(a) FE_CALLITn05(FE_n04,(what, ##__VA_ARGS__))
#define FE_n06(what, a, ...)  what(a) FE_CALLITn06(FE_n05,(what, ##__VA_ARGS__))
#define FE_n07(what, a, ...)  what(a) FE_CALLITn07(FE_n06,(what, ##__VA_ARGS__))
#define FE_n08(what, a, ...)  what(a) FE_CALLITn08(FE_n07,(what, ##__VA_ARGS__))
#define FE_n09(what, a, ...)  what(a) FE_CALLITn09(FE_n08,(what, ##__VA_ARGS__))
#define FE_n10(what, a, ...)  what(a) FE_CALLITn10(FE_n09,(what, ##__VA_ARGS__))
#define FE_n11(what, a, ...)  what(a) FE_CALLITn11(FE_n10,(what, ##__VA_ARGS__))
#define FE_n12(what, a, ...)  what(a) FE_CALLITn12(FE_n11,(what, ##__VA_ARGS__))
#define FE_n13(what, a, ...)  what(a) FE_CALLITn13(FE_n12,(what, ##__VA_ARGS__))
#define FE_n14(what, a, ...)  what(a) FE_CALLITn14(FE_n13,(what, ##__VA_ARGS__))
#define FE_n15(what, a, ...)  what(a) FE_CALLITn15(FE_n14,(what, ##__VA_ARGS__))
#define FE_n16(what, a, ...)  what(a) FE_CALLITn16(FE_n15,(what, ##__VA_ARGS__))
#define FE_n17(what, a, ...)  what(a) FE_CALLITn17(FE_n16,(what, ##__VA_ARGS__))
#define FE_n18(what, a, ...)  what(a) FE_CALLITn18(FE_n17,(what, ##__VA_ARGS__))
#define FE_n19(what, a, ...)  what(a) FE_CALLITn19(FE_n18,(what, ##__VA_ARGS__))
#define FE_n20(what, a, ...)  what(a) FE_CALLITn20(FE_n19,(what, ##__VA_ARGS__))
#define FE_n21(what, a, ...)  what(a) FE_CALLITn21(FE_n20,(what, ##__VA_ARGS__))
#define FE_n22(...)           ERROR: FOR_EACH only supports up to 21 arguments

#define FE_GET_MACRO(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
	_21, _22,NAME,...) NAME

#define FOR_EACH(what, ...) FE_CALLITn01(FE_GET_MACRO(_0, ##__VA_ARGS__,                                               \
	FE_n22, FE_n21, FE_n20, FE_n19, FE_n18, FE_n17, FE_n16, FE_n15, FE_n14, FE_n13, FE_n12, FE_n11, FE_n10, FE_n09,    \
	FE_n08, FE_n07, FE_n06, FE_n05, FE_n04, FE_n03, FE_n02, FE_n01, FE_n00), (what, ##__VA_ARGS__))

#define ONE_PLUS(token) 1+
#define TO_STRING(token) #token,
#define O_STRING(token) "O"
#define CREATE_POINTER(name) ref name = nullptr;
#define ADDRESS(var) ,&var

#define VA_SIZE(...) (FOR_EACH(ONE_PLUS, __VA_ARGS__) 0)

/**
 * @brief Parse arguments for a python function call with args and kwargs.
 * The function using this macro has to have the following signature:
 * ref methodHello(ref, ref _a, ref _kw)
 * The first argument (unnamed) is the current module object. The second is *args, and the third - *kwargs.
 *
 * In this macro's parameters, write function argument names:
 * PARSEARGS(arg1, arg2, arg3)
 * For class methods, don't forget to add self:
 * PARSEARGS(self, arg1, arg2, arg3)
 * The macro will create variables in local scope with the specified names, with types of uref.
 */
#define PARSEARGS(...) \
FOR_EACH(CREATE_POINTER, __VA_ARGS__); \
{ \
const int _parseargs_argcount = VA_SIZE(__VA_ARGS__); \
static const char* _parseargs_argnames[_parseargs_argcount+1] = {FOR_EACH(TO_STRING, __VA_ARGS__) nullptr}; \
const char* _parseargs_format = FOR_EACH(O_STRING, __VA_ARGS__); \
if (!PyArg_ParseTupleAndKeywords(_a, _kw, _parseargs_format, (char**)_parseargs_argnames FOR_EACH(ADDRESS, __VA_ARGS__))) \
	{return nullptr;} \
}

// Submodules ==========================================================================================================
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
	assert(submodules::n < 32);
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

// std::unordered_map and set support ==================================================================================
struct py_unicode_hash {
	size_t operator()(ref k) const noexcept {
		return PyObject_Hash(k);
	}
};

struct py_unicode_equal {
	bool operator()(ref lhs, ref rhs) const noexcept {
		return PyUnicode_RichCompare(lhs, rhs, Py_EQ) == Py_True;
	}
};
