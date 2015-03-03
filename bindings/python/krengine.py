# This file was automatically generated by SWIG (http://www.swig.org).
# Version 2.0.4
#
# Do not make changes to this file unless you know what you are doing--modify
# the SWIG interface file instead.



from sys import version_info
if version_info >= (2,6,0):
    def swig_import_helper():
        from os.path import dirname
        import imp
        fp = None
        try:
            fp, pathname, description = imp.find_module('_krengine', [dirname(__file__)])
        except ImportError:
            import _krengine
            return _krengine
        if fp is not None:
            try:
                _mod = imp.load_module('_krengine', fp, pathname, description)
            finally:
                fp.close()
            return _mod
    _krengine = swig_import_helper()
    del swig_import_helper
else:
    import _krengine
del version_info
try:
    _swig_property = property
except NameError:
    pass # Python < 2.2 doesn't have 'property'.
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'SwigPyObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    if (name == "thisown"): return self.this.own()
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError(name)

def _swig_repr(self):
    try: strthis = "proxy of " + self.this.__repr__()
    except: strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

try:
    _object = object
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0


KR_MSGID_LEN = _krengine.KR_MSGID_LEN
KR_METHOD_LEN = _krengine.KR_METHOD_LEN
KR_DATASRC_LEN = _krengine.KR_DATASRC_LEN
KR_MSGLEN_LEN = _krengine.KR_MSGLEN_LEN
KR_MSGHEADER_LEN = _krengine.KR_MSGHEADER_LEN
KR_MSGTYPE_ERROR = _krengine.KR_MSGTYPE_ERROR
KR_MSGTYPE_SUCCESS = _krengine.KR_MSGTYPE_SUCCESS
KR_BUFFER_MAX_LEN = _krengine.KR_BUFFER_MAX_LEN
class T_KRBuffer(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, T_KRBuffer, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, T_KRBuffer, name)
    __repr__ = _swig_repr
    __swig_setmethods__["size"] = _krengine.T_KRBuffer_size_set
    __swig_getmethods__["size"] = _krengine.T_KRBuffer_size_get
    if _newclass:size = _swig_property(_krengine.T_KRBuffer_size_get, _krengine.T_KRBuffer_size_set)
    __swig_setmethods__["data"] = _krengine.T_KRBuffer_data_set
    __swig_getmethods__["data"] = _krengine.T_KRBuffer_data_get
    if _newclass:data = _swig_property(_krengine.T_KRBuffer_data_get, _krengine.T_KRBuffer_data_set)
    def __init__(self): 
        this = _krengine.new_T_KRBuffer()
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _krengine.delete_T_KRBuffer
    __del__ = lambda self : None;
T_KRBuffer_swigregister = _krengine.T_KRBuffer_swigregister
T_KRBuffer_swigregister(T_KRBuffer)

class T_KRMessage(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, T_KRMessage, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, T_KRMessage, name)
    __repr__ = _swig_repr
    __swig_setmethods__["msgtype"] = _krengine.T_KRMessage_msgtype_set
    __swig_getmethods__["msgtype"] = _krengine.T_KRMessage_msgtype_get
    if _newclass:msgtype = _swig_property(_krengine.T_KRMessage_msgtype_get, _krengine.T_KRMessage_msgtype_set)
    __swig_setmethods__["msgid"] = _krengine.T_KRMessage_msgid_set
    __swig_getmethods__["msgid"] = _krengine.T_KRMessage_msgid_get
    if _newclass:msgid = _swig_property(_krengine.T_KRMessage_msgid_get, _krengine.T_KRMessage_msgid_set)
    __swig_setmethods__["method"] = _krengine.T_KRMessage_method_set
    __swig_getmethods__["method"] = _krengine.T_KRMessage_method_get
    if _newclass:method = _swig_property(_krengine.T_KRMessage_method_get, _krengine.T_KRMessage_method_set)
    __swig_setmethods__["datasrc"] = _krengine.T_KRMessage_datasrc_set
    __swig_getmethods__["datasrc"] = _krengine.T_KRMessage_datasrc_get
    if _newclass:datasrc = _swig_property(_krengine.T_KRMessage_datasrc_get, _krengine.T_KRMessage_datasrc_set)
    __swig_setmethods__["msglen"] = _krengine.T_KRMessage_msglen_set
    __swig_getmethods__["msglen"] = _krengine.T_KRMessage_msglen_get
    if _newclass:msglen = _swig_property(_krengine.T_KRMessage_msglen_get, _krengine.T_KRMessage_msglen_set)
    __swig_setmethods__["msgbuf"] = _krengine.T_KRMessage_msgbuf_set
    __swig_getmethods__["msgbuf"] = _krengine.T_KRMessage_msgbuf_get
    if _newclass:msgbuf = _swig_property(_krengine.T_KRMessage_msgbuf_get, _krengine.T_KRMessage_msgbuf_set)
    def __init__(self): 
        this = _krengine.new_T_KRMessage()
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _krengine.delete_T_KRMessage
    __del__ = lambda self : None;
T_KRMessage_swigregister = _krengine.T_KRMessage_swigregister
T_KRMessage_swigregister(T_KRMessage)

class T_KREngineConfig(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, T_KREngineConfig, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, T_KREngineConfig, name)
    __repr__ = _swig_repr
    __swig_setmethods__["logpath"] = _krengine.T_KREngineConfig_logpath_set
    __swig_getmethods__["logpath"] = _krengine.T_KREngineConfig_logpath_get
    if _newclass:logpath = _swig_property(_krengine.T_KREngineConfig_logpath_get, _krengine.T_KREngineConfig_logpath_set)
    __swig_setmethods__["logname"] = _krengine.T_KREngineConfig_logname_set
    __swig_getmethods__["logname"] = _krengine.T_KREngineConfig_logname_get
    if _newclass:logname = _swig_property(_krengine.T_KREngineConfig_logname_get, _krengine.T_KREngineConfig_logname_set)
    __swig_setmethods__["loglevel"] = _krengine.T_KREngineConfig_loglevel_set
    __swig_getmethods__["loglevel"] = _krengine.T_KREngineConfig_loglevel_get
    if _newclass:loglevel = _swig_property(_krengine.T_KREngineConfig_loglevel_get, _krengine.T_KREngineConfig_loglevel_set)
    __swig_setmethods__["dbname"] = _krengine.T_KREngineConfig_dbname_set
    __swig_getmethods__["dbname"] = _krengine.T_KREngineConfig_dbname_get
    if _newclass:dbname = _swig_property(_krengine.T_KREngineConfig_dbname_get, _krengine.T_KREngineConfig_dbname_set)
    __swig_setmethods__["dbuser"] = _krengine.T_KREngineConfig_dbuser_set
    __swig_getmethods__["dbuser"] = _krengine.T_KREngineConfig_dbuser_get
    if _newclass:dbuser = _swig_property(_krengine.T_KREngineConfig_dbuser_get, _krengine.T_KREngineConfig_dbuser_set)
    __swig_setmethods__["dbpass"] = _krengine.T_KREngineConfig_dbpass_set
    __swig_getmethods__["dbpass"] = _krengine.T_KREngineConfig_dbpass_get
    if _newclass:dbpass = _swig_property(_krengine.T_KREngineConfig_dbpass_get, _krengine.T_KREngineConfig_dbpass_set)
    __swig_setmethods__["krdb_module"] = _krengine.T_KREngineConfig_krdb_module_set
    __swig_getmethods__["krdb_module"] = _krengine.T_KREngineConfig_krdb_module_get
    if _newclass:krdb_module = _swig_property(_krengine.T_KREngineConfig_krdb_module_get, _krengine.T_KREngineConfig_krdb_module_set)
    __swig_setmethods__["data_module"] = _krengine.T_KREngineConfig_data_module_set
    __swig_getmethods__["data_module"] = _krengine.T_KREngineConfig_data_module_get
    if _newclass:data_module = _swig_property(_krengine.T_KREngineConfig_data_module_get, _krengine.T_KREngineConfig_data_module_set)
    __swig_setmethods__["rule_module"] = _krengine.T_KREngineConfig_rule_module_set
    __swig_getmethods__["rule_module"] = _krengine.T_KREngineConfig_rule_module_get
    if _newclass:rule_module = _swig_property(_krengine.T_KREngineConfig_rule_module_get, _krengine.T_KREngineConfig_rule_module_set)
    __swig_setmethods__["hdi_cache_size"] = _krengine.T_KREngineConfig_hdi_cache_size_set
    __swig_getmethods__["hdi_cache_size"] = _krengine.T_KREngineConfig_hdi_cache_size_get
    if _newclass:hdi_cache_size = _swig_property(_krengine.T_KREngineConfig_hdi_cache_size_get, _krengine.T_KREngineConfig_hdi_cache_size_set)
    __swig_setmethods__["thread_pool_size"] = _krengine.T_KREngineConfig_thread_pool_size_set
    __swig_getmethods__["thread_pool_size"] = _krengine.T_KREngineConfig_thread_pool_size_get
    if _newclass:thread_pool_size = _swig_property(_krengine.T_KREngineConfig_thread_pool_size_get, _krengine.T_KREngineConfig_thread_pool_size_set)
    __swig_setmethods__["high_water_mark"] = _krengine.T_KREngineConfig_high_water_mark_set
    __swig_getmethods__["high_water_mark"] = _krengine.T_KREngineConfig_high_water_mark_get
    if _newclass:high_water_mark = _swig_property(_krengine.T_KREngineConfig_high_water_mark_get, _krengine.T_KREngineConfig_high_water_mark_set)
    def __init__(self): 
        this = _krengine.new_T_KREngineConfig()
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _krengine.delete_T_KREngineConfig
    __del__ = lambda self : None;
T_KREngineConfig_swigregister = _krengine.T_KREngineConfig_swigregister
T_KREngineConfig_swigregister(T_KREngineConfig)

class T_KREngineArg(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, T_KREngineArg, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, T_KREngineArg, name)
    __repr__ = _swig_repr
    __swig_setmethods__["apply"] = _krengine.T_KREngineArg_apply_set
    __swig_getmethods__["apply"] = _krengine.T_KREngineArg_apply_get
    if _newclass:apply = _swig_property(_krengine.T_KREngineArg_apply_get, _krengine.T_KREngineArg_apply_set)
    __swig_setmethods__["reply"] = _krengine.T_KREngineArg_reply_set
    __swig_getmethods__["reply"] = _krengine.T_KREngineArg_reply_get
    if _newclass:reply = _swig_property(_krengine.T_KREngineArg_reply_get, _krengine.T_KREngineArg_reply_set)
    __swig_setmethods__["cb_func"] = _krengine.T_KREngineArg_cb_func_set
    __swig_getmethods__["cb_func"] = _krengine.T_KREngineArg_cb_func_get
    if _newclass:cb_func = _swig_property(_krengine.T_KREngineArg_cb_func_get, _krengine.T_KREngineArg_cb_func_set)
    __swig_setmethods__["data"] = _krengine.T_KREngineArg_data_set
    __swig_getmethods__["data"] = _krengine.T_KREngineArg_data_get
    if _newclass:data = _swig_property(_krengine.T_KREngineArg_data_get, _krengine.T_KREngineArg_data_set)
    def __init__(self): 
        this = _krengine.new_T_KREngineArg()
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _krengine.delete_T_KREngineArg
    __del__ = lambda self : None;
T_KREngineArg_swigregister = _krengine.T_KREngineArg_swigregister
T_KREngineArg_swigregister(T_KREngineArg)


def kr_message_alloc():
  return _krengine.kr_message_alloc()
kr_message_alloc = _krengine.kr_message_alloc

def kr_message_free(*args):
  return _krengine.kr_message_free(*args)
kr_message_free = _krengine.kr_message_free

def kr_message_parse(*args):
  return _krengine.kr_message_parse(*args)
kr_message_parse = _krengine.kr_message_parse

def kr_message_dump(*args):
  return _krengine.kr_message_dump(*args)
kr_message_dump = _krengine.kr_message_dump

def kr_engine_startup(*args):
  return _krengine.kr_engine_startup(*args)
kr_engine_startup = _krengine.kr_engine_startup

def kr_engine_shutdown(*args):
  return _krengine.kr_engine_shutdown(*args)
kr_engine_shutdown = _krengine.kr_engine_shutdown

def kr_engine_run(*args):
  return _krengine.kr_engine_run(*args)
kr_engine_run = _krengine.kr_engine_run

def kr_engine_register(*args):
  return _krengine.kr_engine_register(*args)
kr_engine_register = _krengine.kr_engine_register
# This file is compatible with both classic and new-style classes.


