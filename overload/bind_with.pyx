#distutils: language = c++
#cython: language_level = 3
from inspect import _POSITIONAL_ONLY, _VAR_POSITIONAL, _KEYWORD_ONLY, _VAR_KEYWORD, _empty
import itertools
from cpython cimport PyObject, Py_INCREF, Py_DECREF
from libcpp cimport bool

cdef int _c_positional_only = _POSITIONAL_ONLY
cdef int _c_var_positional = _VAR_POSITIONAL
cdef int _c_keyword_only = _KEYWORD_ONLY
cdef int _c_var_keyword = _VAR_KEYWORD

cdef Parameter parameter(object py_param):
	cdef Parameter param

	param.name = <PyObject*> py_param.name
	param.kind = py_param.kind
	param.annotation = <PyObject*> py_param.annotation
	param.has_default = py_param.default is not _empty

	Py_INCREF(py_param.name)
	Py_INCREF(py_param.annotation)

	return param


cdef void delparameter(Parameter param):
	Py_DECREF(<object> param.name)
	Py_DECREF(<object> param.annotation)


cdef class Signature:
	def __init__(self, py_sig):
		# Note that the `parameter(py_param)` pseudoconstructor calls Py_INCREF.
		# Matching Py_DECREF is called in `delparameter` during __dealloc__.
		# While __dealloc__ is guaranteed to run once, __init__ is not. If any memory leaks occur, it's likely because
		# of this.
		self.parameters.reserve(len(py_sig.parameters))
		for py_param in py_sig.parameters.values():
			self.parameters.push_back(parameter(py_param))
	
	def __dealloc__(self):
		cdef Parameter param
		for param in self.parameters:
			delparameter(param)


cpdef bind_with(Signature sig, bind_func, args, kwargs):	
	cdef int args_i = 0
	cdef int parameters_i = 0

	while True:
		# Let's iterate through the positional arguments and corresponding
		# parameters
		if args_i < len(args):
			arg_val = args[args_i]
			args_i += 1

			# We have a positional argument to process
			if parameters_i < sig.parameters.size():
				param = sig.parameters[parameters_i]
				parameters_i += 1
				annotation = <object> param.annotation
				if annotation is _empty:
					annotation = object
				
				if param.kind == _c_var_keyword or param.kind == _c_keyword_only:
					# Looks like we have no parameter for this positional
					# argument
					raise TypeError('too many positional arguments') from None

				if param.kind == _c_var_positional:
					break

				if <object> param.name in kwargs and param.kind != _c_positional_only:
					raise TypeError(f'multiple values for argument {<object> param.name!r}') from None

				if not bind_func(arg_val, annotation):
					raise TypeError(
						f"argument '{<object> param.name!r}' has unexpected type '{type(arg_val).__qualname__}'"
					)
			else:
				raise TypeError('too many positional arguments') from None
		else:
			# No more positional arguments
			if parameters_i < sig.parameters.size():
				param = sig.parameters[parameters_i]
				parameters_i += 1
				annotation = <object> param.annotation
				if annotation is _empty:
					annotation = object
				
				if param.kind == _c_var_positional:
					# That's OK, just empty *args.  Let's start parsing
					# kwargs
					break
				elif <object> param.name in kwargs:
					if param.kind == _c_positional_only:
						raise TypeError(
							f'{<object> param.name!r} parameter is positional only, but was passed as a keyword'
						) from None
					parameters_i -= 1
					break
				elif (param.kind == _c_var_keyword or param.has_default):
					# That's fine too - we have a default value for this
					# parameter.  So, lets start parsing `kwargs`, starting
					# with the current parameter
					parameters_i -= 1
					break
				else:
					# No default, not VAR_KEYWORD, not VAR_POSITIONAL,
					# not in `kwargs`
					raise TypeError(f'missing a required argument: {<object> param.name!r}') from None
			else:
				# No more parameters. That's it. Just need to check that
				# we have no `kwargs` after this while loop
				break

	# Now, we iterate through the remaining parameters to process
	# keyword arguments
	cdef bool kwargs_param = False
	while parameters_i < sig.parameters.size():
		param = sig.parameters[parameters_i]
		parameters_i += 1
		if param.kind == _c_var_keyword:
			# Memorize that we have a '**kwargs'-like parameter
			kwargs_param = True
			continue

		if param.kind == _c_var_positional:
			# Named arguments don't refer to '*args'-like parameters.
			# We only arrive here if the positional arguments ended
			# before reaching the last parameter before *args.
			continue

		param_name = <object> param.name
		try:
			arg_val = kwargs.pop(param_name)
		except KeyError:
			# We have no value for this parameter.  It's fine though,
			# if it has a default value, or it is an '*args'-like
			# parameter, left alone by the processing of positional
			# arguments.
			if (param.kind != _c_var_positional and not param.has_default):
				raise TypeError(f'missing a required argument: {param_name!r}') from None

		else:
			if param.kind == _c_positional_only:
				# This should never happen in case of a properly built
				# Signature object (but let's have this check here
				# to ensure correct behaviour just in case)
				raise TypeError(f'{<object> param.name!r} parameter is positional only, but was passed as a keyword')

			if not bind_func(arg_val, annotation):
				raise TypeError(f"argument '{param_name!r}' has unexpected type '{type(arg_val).__qualname__}'")

	if kwargs and not kwargs_param:
		raise TypeError(f'got an unexpected keyword argument {next(iter(kwargs))!r}')
