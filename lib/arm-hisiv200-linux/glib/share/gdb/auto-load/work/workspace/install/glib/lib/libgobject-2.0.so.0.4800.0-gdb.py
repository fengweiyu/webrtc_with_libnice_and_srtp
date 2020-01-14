import sys
import gdb

# Update module path.
dir_ = '/work/workspace/install/glib/share/glib-2.0/gdb'
if not dir_ in sys.path:
    sys.path.insert(0, dir_)

from gobject import register
register (gdb.current_objfile ())
