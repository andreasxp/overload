#distutils: language = c++
#cython: profile=True
#cython: infer_types=True
#cython: boundscheck=False
#cython: wraparound=False
#cython: language_level = 3
from inspect import _POSITIONAL_ONLY, _VAR_POSITIONAL, _KEYWORD_ONLY, _VAR_KEYWORD, _empty
import itertools
from libcpp cimport bool
from .signature cimport Signature

cdef int _c_positional_only = _POSITIONAL_ONLY
cdef int _c_var_positional = _VAR_POSITIONAL
cdef int _c_keyword_only = _KEYWORD_ONLY
cdef int _c_var_keyword = _VAR_KEYWORD


cdef bind_with(Signature sig, object bind_func, tuple args, dict kwargs):
	cdef dict kwargs_ = kwargs.copy()
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
					return TypeError('too many positional arguments')

				if param.kind == _c_var_positional:
					break

				if <object> param.name in kwargs_ and param.kind != _c_positional_only:
					return TypeError(f'multiple values for argument {<object> param.name!r}')

				if not bind_func(arg_val, annotation):
					return TypeError(
						f"argument '{<object> param.name!r}' has unexpected type '{type(arg_val).__qualname__}'"
					)
			else:
				return TypeError('too many positional arguments')
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
				elif <object> param.name in kwargs_:
					if param.kind == _c_positional_only:
						return TypeError(
							f'{<object> param.name!r} parameter is positional only, but was passed as a keyword'
						)
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
					return TypeError(f'missing a required argument: {<object> param.name!r}')
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

		arg_val = kwargs_.pop(<object> param.name, None)
		if arg_val is None:
			# We have no value for this parameter.  It's fine though,
			# if it has a default value, or it is an '*args'-like
			# parameter, left alone by the processing of positional
			# arguments.
			if (param.kind != _c_var_positional and not param.has_default):
				return TypeError(f'missing a required argument: {<object> param.name!r}')
		else:
			if param.kind == _c_positional_only:
				# This should never happen in case of a properly built
				# Signature object (but let's have this check here
				# to ensure correct behaviour just in case)
				return TypeError(f'{<object> param.name!r} parameter is positional only, but was passed as a keyword')

			if not bind_func(arg_val, annotation):
				return TypeError(f"argument '{<object> param.name!r}' has unexpected type '{type(arg_val).__qualname__}'")

	if kwargs_ and not kwargs_param:
		return TypeError(f'got an unexpected keyword argument {next(iter(kwargs_))!r}')
