from functools import wraps
import json
import logging
import traceback

log = logging.getLogger(__name__)


def api_err_handler(jsonify=False):
    """ Decorator returns an object with success field set to False and containing an error
        message when an exception is caught.
    """
    def api_err_handler_impl(f):
        @wraps(f)
        def wrapper(*args, **kwargs):
            try:
                return f(*args, **kwargs)
            except Exception, e:
                log.error('Handling error with err_msg: %s\n%s', str(e), traceback.format_exc())

                retobj = {'success': False,
                          'err_msg': str(e)}
                if jsonify:
                    return json.dumps(retobj)
                else:
                    return retobj

        return wrapper
    return api_err_handler_impl
