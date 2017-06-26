# -*- coding: utf-8 -*- #
# Author: Jiaquan Fang

import os
import sys

class TaobaoLDALibraryNotFound(Exception):
    pass

class PlatformNotSupported(Exception):
    pass

def find_lib_path():
    curr_path = os.path.dirname(os.path.abspath(os.path.expanduser(__file__)))

    dll_path = [curr_path, os.path.join(curr_path, '../../lib'),
                os.path.join(curr_path, './lib'),
                os.path.join(sys.prefix, 'twitterlda')]

    if os.name == 'nt':
        raise PlatformNotSupported('Windows is not supported')
    else:
        dll_path = [os.path.join(p, 'libtwitterlda.so') for p in dll_path]

    lib_path = [p for p in dll_path if os.path.exists(p) and os.path.isfile(p)]

    if not lib_path:
        raise TaobaoLDALibraryNotFound(
            'Cannot find TwitterLDA Library in the candidate path'
        )

    return lib_path
