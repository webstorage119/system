/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Elizabeth M Smith <auroraeosrose@gmail.com>                  |
  +----------------------------------------------------------------------+
*/

#include "php_winsystem.h"

ZEND_DECLARE_MODULE_GLOBALS(winsystem);

zend_class_entry *ce_winsystem_unicode;

static zend_object_handlers winsystem_unicode_handlers;
static zend_function winsystem_unicode_constructor_wrapper;

struct _winsystem_unicode_object {
	zend_object  std;
	zend_bool    is_constructed;
	WCHAR *      unicode_string;
	CHAR *       multibyte_string;
	int          multibyte_strlen;
};

wchar_t php_winsystem_bitswap(wchar_t value);

/* ----------------------------------------------------------------
    Win\System\Unicode C API
------------------------------------------------------------------*/

/* {{{ exported function to take a zval** enum instance and give you back the long value */
PHP_WINSYSTEM_API WCHAR * php_winsystem_unicode_get_wchar(zval** unicodeclass TSRMLS_DC)
{
	winsystem_unicode_object *unicode_object;

	unicode_object = (winsystem_unicode_object *) zend_object_store_get_object(*unicodeclass TSRMLS_CC);
	return unicode_object->unicode_string;
}
/* }}} */

/* {{{ exported function to take a zval** enum instance and give you back the long value */
PHP_WINSYSTEM_API CHAR * php_winsystem_unicode_get_char(zval** unicodeclass TSRMLS_DC)
{
	winsystem_unicode_object *unicode_object;

	unicode_object = (winsystem_unicode_object *) zend_object_store_get_object(*unicodeclass TSRMLS_CC);
	return unicode_object->multibyte_string;
}
/* }}} */

/* {{{ exported function to take a wchar_t in utf16 and put the right stuff in the object */
PHP_WINSYSTEM_API void php_winsystem_unicode_create(zval** object, wchar_t *text, int length TSRMLS_DC)
{
	winsystem_unicode_object *unicode_object;

	object_init_ex(*object, ce_winsystem_unicode);
	unicode_object = (winsystem_unicode_object *) zend_object_store_get_object(*object TSRMLS_CC);

	unicode_object->unicode_string = safe_emalloc(length + 1, sizeof(wchar_t), 0);
	memcpy(unicode_object->unicode_string, text, length);
	unicode_object->multibyte_string = php_winsystem_unicode_wchar_to_char(&text, CP_UTF8);
	unicode_object->multibyte_strlen = strlen(unicode_object->multibyte_string);
}
/* }}} */

/* {{{ win_system_convert_to_wchar
       api converts char * of charset codepage to utf16 wchar */
PHP_WINSYSTEM_API WCHAR * php_winsystem_unicode_char_to_wchar(const CHAR ** mb_string, int codepage)
{
	wchar_t * utf16_string = NULL;
	int utf16_string_length, result;

	utf16_string_length = MultiByteToWideChar(
		codepage,         /* convert from selected codepage */
		0,            /* error on invalid chars */
		*mb_string,      /* source string */
		-1,           /* total length of source UTF-8 string, in CHAR's (= bytes), including end-of-string \0 */
		NULL,         /* unused - no conversion done in this step */
		0             /* request size of destination buffer, in WCHAR's */
   );

	// TODO: utf8_string_length and utf16_string_length cannot be > INT_MAX - 1 or we chop chop

	utf16_string = emalloc(utf16_string_length * sizeof(wchar_t));

	// TODO: apply common flags when converting
	result = MultiByteToWideChar(
		codepage,               /* convert from codepage */
		0,
		*mb_string,            /* source string */
		-1,                 /* total length of source UTF-8 string */
		utf16_string,       /* destination buffer */
		utf16_string_length /* size of destination buffer, in WCHAR's */
	);

	// TODO: error handle, result better == utf16 string length

	return utf16_string;
}
/* }}} */

/* {{{ win_system_convert_to_char
       api converts wchar to char * of charset codepage */
PHP_WINSYSTEM_API CHAR * php_winsystem_unicode_wchar_to_char(const WCHAR ** utf16_string, int codepage)
{
	char * mb_string = NULL;
	int mb_string_length, result;

	mb_string_length = WideCharToMultiByte(
		codepage,          /* convert to type */
		0,             /* error on invalid chars*/
		*utf16_string, /* source string */
		-1,            /* convert the whole string */
		NULL,          /* unused - no conversion done in this step */
		0,             /* unused - no conversion done in this step */
		NULL, NULL
	);

	// TODO: utf8_string_length and utf16_string_length cannot be > INT_MAX - 1

	mb_string = emalloc(mb_string_length);

	// TODO: apply common flags when converting
	result = WideCharToMultiByte(
		codepage,
		0,
		*utf16_string,
		-1,
		mb_string,
		mb_string_length,
		NULL, NULL
	);

	// TODO: error handle, result should equal == utf16 string length

	return mb_string;
}
/* }}} */

/* ----------------------------------------------------------------
   Win\System\Unicode Userland API
------------------------------------------------------------------*/
ZEND_BEGIN_ARG_INFO(WinSystemUnicode___construct_args, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(ZEND_SEND_BY_VAL, value)
	ZEND_ARG_OBJ_INFO(ZEND_SEND_BY_VAL, codepage, Win\\System\\Codepage, FALSE)
ZEND_END_ARG_INFO()

/* {{{ proto array Win\System\Unicode->__construct(string value)
     Creates a new Win system unicode object */
PHP_METHOD(WinSystemUnicode, __construct)
{
	char * name = NULL;
	int name_length;
	long type;
	zval *codepage;
	winsystem_unicode_object *unicode_object;

	PHP_WINSYSTEM_EXCEPTIONS
	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sO", &name, &name_length, &codepage, ce_winsystem_codepage)) {
		return;
	}
	PHP_WINSYSTEM_RESTORE_ERRORS

	unicode_object = (winsystem_unicode_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	type = php_winsystem_get_enum_value(&codepage TSRMLS_CC);

	if (type == 1200 || type == 1201) {
		int i, w = 0;

		unicode_object->unicode_string = emalloc(name_length + 1 * sizeof(wchar_t));

		for (i = 0; i < name_length; ++i) {
			unicode_object->unicode_string[w] = (name[i + 1] << 8 ) | (name[i] & 0xff);
			i++;
			w++;
		}
		unicode_object->unicode_string[w] = '\0';


		/* if we're 1201 we're utf-16 big endian - bit swapping time */
		if (type == 1201) {
			for (i = 0; i < w; ++i) {
				unicode_object->unicode_string[i] = php_winsystem_bitswap(unicode_object->unicode_string[i]);
			}
		}

		unicode_object->multibyte_string = php_winsystem_unicode_wchar_to_char(&unicode_object->unicode_string, CP_UTF8);
		unicode_object->multibyte_strlen = strlen(unicode_object->multibyte_string);
	} else {
		unicode_object->unicode_string = php_winsystem_unicode_char_to_wchar(&name, type);
		unicode_object->multibyte_string = estrdup(name);
		unicode_object->multibyte_strlen = name_length;
	}

	unicode_object->is_constructed = TRUE;
}
/* }}} */

/* register unicode methods */
static zend_function_entry winsystem_unicode_functions[] = {
	PHP_ME(WinSystemUnicode, __construct, WinSystemUnicode___construct_args, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	ZEND_FE_END
};

/* ----------------------------------------------------------------
  Win\System\Unicode Custom Object magic
------------------------------------------------------------------*/

/* {{{ for utf16 big endian - the great bit swap */
static wchar_t php_winsystem_bitswap(wchar_t value)
{
    return (((value>> 8)) | (value << 8));
}
/* }}} */

/* {{{ winsystem_unicode_object_free
       frees up the wchar underneath */
static void winsystem_unicode_object_free(void *object TSRMLS_DC)
{
	winsystem_unicode_object *unicode_object = (winsystem_unicode_object *)object;

	zend_object_std_dtor(&unicode_object->std TSRMLS_CC);

	if(unicode_object->unicode_string) {
		efree(unicode_object->unicode_string);
	}

	if(unicode_object->multibyte_string) {
		efree(unicode_object->multibyte_string);
	}
	
	efree(unicode_object);
}
/* }}} */

/* {{{ winsystem_unicode_object_create
       object that has an internal WCHAR stored  */
static zend_object_value winsystem_unicode_object_create(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value              retval;
	winsystem_unicode_object       *unicode_object;
 
	unicode_object = ecalloc(1, sizeof(winsystem_unicode_object));
	zend_object_std_init(&unicode_object->std, ce TSRMLS_CC);
	unicode_object->unicode_string = NULL;
	unicode_object->multibyte_string = NULL;
	unicode_object->is_constructed = FALSE;
 
	object_properties_init(&unicode_object->std, ce);
 
	retval.handle = zend_objects_store_put(unicode_object,
		(zend_objects_store_dtor_t) zend_objects_destroy_object,
		(zend_objects_free_object_storage_t) winsystem_unicode_object_free,
		NULL TSRMLS_CC);
	retval.handlers = &winsystem_unicode_handlers;
	return retval;
}
/* }}} */

/* {{{ winsystem_unicode_get */
static zval* winsystem_unicode_get(zval *zobject TSRMLS_DC)
{
	winsystem_unicode_object *unicode_object = (winsystem_unicode_object *) zend_object_store_get_object(zobject TSRMLS_CC);
	zval *value;

	MAKE_STD_ZVAL(value);
	ZVAL_STRINGL(value, unicode_object->multibyte_string, unicode_object->multibyte_strlen, 1);
	Z_SET_REFCOUNT_P(value, 0);

	return value;
} /* }}} */

/* {{{ winsystem_unicode_cast */
static int winsystem_unicode_cast(zval *readobj, zval *writeobj, int type TSRMLS_DC)
{
	winsystem_unicode_object *unicode_object = (winsystem_unicode_object *) zend_object_store_get_object(readobj TSRMLS_CC);

	ZVAL_STRINGL(writeobj, unicode_object->multibyte_string, unicode_object->multibyte_strlen, 1);
	convert_to_explicit_type(writeobj, type);
	return SUCCESS;
}
/* }}} */

/* {{{ winsystem_unicode_get_constructor */
static zend_function *winsystem_unicode_get_constructor(zval *object TSRMLS_DC)
{
	/* Could always return constr_wrapper_fun, but it's uncessary to call the
	 * wrapper if instantiating the superclass */
	if (Z_OBJCE_P(object) == ce_winsystem_unicode)
		return zend_get_std_object_handlers()->
			get_constructor(object TSRMLS_CC);
	else
		return &winsystem_unicode_constructor_wrapper;
}
/* }}} */

/* {{{ winsystem_unicode_construction_wrapper */
static void winsystem_unicode_construction_wrapper(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *this = getThis();
	winsystem_unicode_object *tobj;
	zend_class_entry *this_ce;
	zend_function *zf;
	zend_fcall_info fci = {0};
	zend_fcall_info_cache fci_cache = {0};
	zval *retval_ptr = NULL;
	unsigned i;
 
	tobj = zend_object_store_get_object(this TSRMLS_CC);
	zf = zend_get_std_object_handlers()->get_constructor(this TSRMLS_CC);
	this_ce = Z_OBJCE_P(this);
 
	fci.size = sizeof(fci);
	fci.function_table = &this_ce->function_table;
	fci.object_ptr = this;
	/* fci.function_name = ; not necessary to bother */
	fci.retval_ptr_ptr = &retval_ptr;
	fci.param_count = ZEND_NUM_ARGS();
	fci.params = emalloc(fci.param_count * sizeof *fci.params);
	/* Or use _zend_get_parameters_array_ex instead of loop: */
	for (i = 0; i < fci.param_count; i++) {
		fci.params[i] = (zval **) (zend_vm_stack_top(TSRMLS_C) - 1 -
			(fci.param_count - i));
	}
	fci.object_ptr = this;
	fci.no_separation = 0;
 
	fci_cache.initialized = 1;
	fci_cache.called_scope = EG(current_execute_data)->called_scope;
	fci_cache.calling_scope = EG(current_execute_data)->current_scope;
	fci_cache.function_handler = zf;
	fci_cache.object_ptr = this;
 
	zend_call_function(&fci, &fci_cache TSRMLS_CC);
	if (!EG(exception) && tobj->is_constructed == 0)
		zend_throw_exception_ex(ce_winsystem_runtimeexception, 0 TSRMLS_CC,
			"parent::__construct() must be called in %s::__construct()", this_ce->name);
	efree(fci.params);
	zval_ptr_dtor(&retval_ptr);
}
/* }}} */

/* ----------------------------------------------------------------
  Win\System\Unicode Object Magic LifeCycle Functions
------------------------------------------------------------------ */
/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(winsystem_unicode)
{
	zend_class_entry ce;
 
	INIT_NS_CLASS_ENTRY(ce, PHP_WINSYSTEM_NS, "Unicode", winsystem_unicode_functions);

	ce_winsystem_unicode = zend_register_internal_class(&ce TSRMLS_CC);

	ce_winsystem_unicode->create_object = winsystem_unicode_object_create;
	memcpy(&winsystem_unicode_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	winsystem_unicode_handlers.cast_object = winsystem_unicode_cast;
	winsystem_unicode_handlers.get = winsystem_unicode_get;
	winsystem_unicode_handlers.get_constructor = winsystem_unicode_get_constructor;
 
	winsystem_unicode_constructor_wrapper.type = ZEND_INTERNAL_FUNCTION;
	winsystem_unicode_constructor_wrapper.common.function_name = "internal_construction_wrapper";
	winsystem_unicode_constructor_wrapper.common.scope = ce_winsystem_unicode;
	winsystem_unicode_constructor_wrapper.common.fn_flags = ZEND_ACC_PROTECTED;
	winsystem_unicode_constructor_wrapper.common.prototype = NULL;
	winsystem_unicode_constructor_wrapper.common.required_num_args = 0;
	winsystem_unicode_constructor_wrapper.common.arg_info = NULL;
	winsystem_unicode_constructor_wrapper.internal_function.handler = winsystem_unicode_construction_wrapper;
	winsystem_unicode_constructor_wrapper.internal_function.module = EG(current_module);

	return SUCCESS;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */