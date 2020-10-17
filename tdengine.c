/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
  phpize
  ./configure
  make && make install
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_tdengine.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "inttypes.h"
#include "taos.h"

/* If you declare any globals in php_tdengine.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(tdengine)
*/

/* True global resources - no need for thread safety here */
static int le_tdengine;
static void _close_tdengine_link(zend_resource *rsrc)
{
	TAOS *taos = (TAOS *)rsrc->ptr;
	taos_close(taos);
}
static void _free_ptr(zend_resource *rsrc)
{
	// pgLofp *lofp = (pgLofp *)rsrc->ptr;
	// efree(lofp);
}
static void _free_result(zend_resource *rsrc)
{
	TAOS_RES *res = (TAOS_RES *)rsrc->ptr;
	taos_free_result(res);
}


/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("tdengine.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_tdengine_globals, tdengine_globals)
    STD_PHP_INI_ENTRY("tdengine.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_tdengine_globals, tdengine_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_tdengine_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_tdengine_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "11111Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "tdengine", arg);

	RETURN_STR(strg);
}
static int le_link, le_plink, le_result, le_presult, le_lofp, le_string;

PHP_FUNCTION(taos_connect)
{
	TAOS *    taos;
	TAOS_RES *result;
	zend_string *strg;
	char *host = NULL;
	char *username = NULL;
	char *password = NULL;
	char *db = NULL;
	zend_long port = 0;
	size_t host_len, username_len,password_len,db_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ssssl", &host, &host_len,&username, &username_len,&password, &password_len,&db, &db_len,&port) == FAILURE) {
		RETURN_NULL();
	}
	taos_init();
  	taos = taos_connect(host, username, password, db, port);
	RETVAL_RES(zend_register_resource(taos, le_plink));
}

PHP_FUNCTION(taos_query)
{
	TAOS *taos;
	TAOS_RES *result;
	zval *taos_link = NULL;
	zend_resource *link;
	char *query;
	size_t query_len;
	int  argc = ZEND_NUM_ARGS();
	char *arg = NULL;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", &taos_link, &query, &query_len) == FAILURE) {
		RETURN_BOOL(FAILURE);
	}
	link = Z_RES_P(taos_link);
	if ((taos = (TAOS *)zend_fetch_resource2(link, "TDengine link", le_link, le_plink)) == NULL) {
		RETURN_NULL();
	}
	result = taos_query(taos, query);
	if (result == NULL) {
		RETURN_NULL();
	}
	RETVAL_RES(zend_register_resource(result, le_result));
}

PHP_FUNCTION(taos_select_db)
{
	TAOS *taos;
	TAOS_RES *result;
	zval *taos_link = NULL;
	zend_resource *link;
	char *db;
	size_t db_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", &taos_link, &db, &db_len) == FAILURE) {
		RETURN_BOOL(FAILURE);
	}
	link = Z_RES_P(taos_link);
	if ((taos = (TAOS *)zend_fetch_resource2(link, "TDengine link", le_link, le_plink)) == NULL) {
		RETURN_NULL();
	}
	int res = taos_select_db(taos,db);
	RETURN_LONG(res)
}
PHP_FUNCTION(taos_fetch_all)
{
	TAOS *taos;
	TAOS_RES *result;
	zval *taos_link = NULL;
	zend_resource *link;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &taos_link) == FAILURE) {
		RETURN_NULL();
	}
	link = Z_RES_P(taos_link);
	if ((result = (TAOS_RES *)zend_fetch_resource2(link, "TDengine link", le_result, le_presult)) == NULL) {
		RETURN_NULL();
	}
	TAOS_ROW row;
	int rows = 0;
	int num_fields = taos_field_count(result);
	TAOS_FIELD *fields = taos_fetch_fields(result);
	char temp[1024];
	zend_string *strg;
	while ((row = taos_fetch_row(result))) {
		rows++;
		taos_print_row(temp, row, fields, num_fields);
		strg = strpprintf(0, "%.78s",temp);
	}
	taos_free_result(result);
	if(rows == 0){
		RETURN_NULL();
	}
	RETURN_STR(strg);
}
PHP_FUNCTION(taos_affected_rows)
{
	TAOS *taos;
	TAOS_RES *result;
	zval *taos_link = NULL;
	zend_resource *link;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &taos_link) == FAILURE) {
		RETURN_NULL();
	}
	link = Z_RES_P(taos_link);
	if ((result = (TAOS_RES *)zend_fetch_resource2(link, "TDengine link", le_result, le_presult)) == NULL) {
		RETURN_NULL();
	}
	int affected = taos_affected_rows(result);
	RETURN_LONG(affected)
}

/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_tdengine_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_tdengine_init_globals(zend_tdengine_globals *tdengine_globals)
{
	tdengine_globals->global_value = 0;
	tdengine_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(tdengine)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	le_link = zend_register_list_destructors_ex(_close_tdengine_link, NULL, "tdengine link", module_number);
	le_plink = zend_register_list_destructors_ex(NULL, _close_tdengine_link, "tdengine link persistent", module_number);
	le_result = zend_register_list_destructors_ex(_free_result, NULL, "tdengine result", module_number);
	le_presult = zend_register_list_destructors_ex(NULL, _free_result, "tdengine result", module_number);
	le_lofp = zend_register_list_destructors_ex(_free_ptr, NULL, "tdengine large object", module_number);
	le_string = zend_register_list_destructors_ex(_free_ptr, NULL, "tdengine string", module_number);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(tdengine)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	taos_cleanup();
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(tdengine)
{
#if defined(COMPILE_DL_TDENGINE) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(tdengine)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(tdengine)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "tdengine support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ tdengine_functions[]
 *
 * Every user visible function must have an entry in tdengine_functions[].
 */
const zend_function_entry tdengine_functions[] = {
	PHP_FE(confirm_tdengine_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(taos_connect,	NULL)
	PHP_FE(taos_query,	NULL)
	PHP_FE(taos_select_db,	NULL)
	PHP_FE(taos_fetch_all,	NULL)
	PHP_FE(taos_affected_rows,	NULL)
	
	
	
	PHP_FE_END	/* Must be the last line in tdengine_functions[] */
};
/* }}} */

/* {{{ tdengine_module_entry
 */
zend_module_entry tdengine_module_entry = {
	STANDARD_MODULE_HEADER,
	"tdengine",
	tdengine_functions,
	PHP_MINIT(tdengine),
	PHP_MSHUTDOWN(tdengine),
	PHP_RINIT(tdengine),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(tdengine),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(tdengine),
	PHP_TDENGINE_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_TDENGINE
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(tdengine)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */


