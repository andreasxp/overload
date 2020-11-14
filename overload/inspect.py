import inspect
from inspect import (_POSITIONAL_ONLY, _POSITIONAL_OR_KEYWORD, _VAR_POSITIONAL, _KEYWORD_ONLY, _VAR_KEYWORD, _empty)
import itertools


class Signature(inspect.Signature):
    """Signature class, derived from inspect.Signature. Overrides the `bind` method to compare args to parameter
    annotations.  
    When constructing this signature (using the function `signature`) you can specify a special binder object - a 
    callable `binder(arg, annotation) -> bool` that returns True if the arguments matches the parameter annotation.
    The simplest case of such a binder is `isinstance`. Parameters that do not have annotations will use `object` as an
    annotation instead.
    """
    def bind(self, args, kwargs, *, binder):
        parameters = iter(self.parameters.values())
        parameters_ex = ()
        arg_vals = iter(args)

        while True:
            # Let's iterate through the positional arguments and corresponding
            # parameters
            try:
                arg_val = next(arg_vals)
            except StopIteration:
                # No more positional arguments
                try:
                    param = next(parameters)
                    annotation = param.annotation
                    if annotation == self.empty:
                        annotation = object

                except StopIteration:
                    # No more parameters. That's it. Just need to check that
                    # we have no `kwargs` after this while loop
                    break
                else:
                    if param.kind == _VAR_POSITIONAL:
                        # That's OK, just empty *args.  Let's start parsing
                        # kwargs
                        break
                    elif param.name in kwargs:
                        if param.kind == _POSITIONAL_ONLY:
                            raise TypeError(
                                f'{param.name!r} parameter is positional only, but was passed as a keyword'
                            ) from None
                        parameters_ex = (param,)
                        break
                    elif (param.kind == _VAR_KEYWORD or param.default is not _empty):
                        # That's fine too - we have a default value for this
                        # parameter.  So, lets start parsing `kwargs`, starting
                        # with the current parameter
                        parameters_ex = (param,)
                        break
                    else:
                        # No default, not VAR_KEYWORD, not VAR_POSITIONAL,
                        # not in `kwargs`
                        raise TypeError(f'missing a required argument: {param.name!r}') from None
            else:
                # We have a positional argument to process
                try:
                    param = next(parameters)
                    annotation = param.annotation
                    if annotation == self.empty:
                        annotation = object

                except StopIteration:
                    raise TypeError('too many positional arguments') from None
                else:
                    if param.kind in (_VAR_KEYWORD, _KEYWORD_ONLY):
                        # Looks like we have no parameter for this positional
                        # argument
                        raise TypeError('too many positional arguments') from None

                    if param.kind == _VAR_POSITIONAL:
                        break

                    if param.name in kwargs and param.kind != _POSITIONAL_ONLY:
                        raise TypeError(f'multiple values for argument {param.name!r}') from None

                    if not binder(arg_val, annotation):
                        raise TypeError(
                            f"argument '{param.name!r}' has unexpected type '{type(arg_val).__qualname__}'"
                        )

        # Now, we iterate through the remaining parameters to process
        # keyword arguments
        kwargs_param = None
        for param in itertools.chain(parameters_ex, parameters):
            if param.kind == _VAR_KEYWORD:
                # Memorize that we have a '**kwargs'-like parameter
                kwargs_param = param
                continue

            if param.kind == _VAR_POSITIONAL:
                # Named arguments don't refer to '*args'-like parameters.
                # We only arrive here if the positional arguments ended
                # before reaching the last parameter before *args.
                continue

            param_name = param.name
            try:
                arg_val = kwargs.pop(param_name)
            except KeyError:
                # We have no value for this parameter.  It's fine though,
                # if it has a default value, or it is an '*args'-like
                # parameter, left alone by the processing of positional
                # arguments.
                if (param.kind != _VAR_POSITIONAL and param.default is _empty):
                    raise TypeError(f'missing a required argument: {param_name!r}') from None

            else:
                if param.kind == _POSITIONAL_ONLY:
                    # This should never happen in case of a properly built
                    # Signature object (but let's have this check here
                    # to ensure correct behaviour just in case)
                    raise TypeError(f'{param.name!r} parameter is positional only, but was passed as a keyword')

                if not binder(arg_val, annotation):
                    raise TypeError(f"argument '{param_name!r}' has unexpected type '{type(arg_val).__qualname__}'")

        if kwargs and kwargs_param is None:
            raise TypeError(f'got an unexpected keyword argument {next(iter(kwargs))!r}')


def signature(callable, *, follow_wrapped=True):
    """Returns a Signature object from a callable, optionally providing a special binder object."""
    return Signature.from_callable(callable, follow_wrapped=follow_wrapped)
