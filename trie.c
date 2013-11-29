/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
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
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_trie.h"
#include "ext/standard/php_smart_str.h"

/** {{{ declare globals
*/
ZEND_DECLARE_MODULE_GLOBALS(trie)
/* }}} */


/* {{{ trie_functions[]
 *
 * Every user visible function must have an entry in trie_functions[].
 */
const zend_function_entry trie_functions[] = {
	PHP_FE(get_tries,	            NULL)		/* For testing, remove later. */
    PHP_FE(trie_set,                NULL)
    PHP_FE(trie_match,              NULL)
	PHP_FE_END	/* Must be the last line in trie_functions[] */
};
/* }}} */

/* {{{ trie_module_entry
 */
zend_module_entry trie_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"trie",
	trie_functions,
	PHP_MINIT(trie),
	PHP_MSHUTDOWN(trie),
	PHP_RINIT(trie),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(trie),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(trie),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_TRIE
ZEND_GET_MODULE(trie)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("trie.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_trie_globals, trie_globals)
    STD_PHP_INI_ENTRY("trie.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_trie_globals, trie_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_trie_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_trie_init_globals(zend_trie_globals *trie_globals)
{
	trie_globals->global_value = 0;
	trie_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(trie)
{
    #ifdef ZTS
        return FALSE;
    #else
        TRIE_G(tries) = (zval *)pemalloc(sizeof(zval), 1);
        INIT_PZVAL(TRIE_G(tries));
        array_init(TRIE_G(tries));

        TRIE_G(count) = (zval *)pemalloc(sizeof(zval), 1);
        INIT_PZVAL(TRIE_G(count));
        ZVAL_LONG(TRIE_G(count), 0);

        return SUCCESS;
    #endif

	
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(trie)
{
    
    pefree(TRIE_G(tries), 1);

    pefree(TRIE_G(count), 1);

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(trie)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(trie)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(trie)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "trie support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* {{{ proto string confirm_trie_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(get_tries)
{
    RETURN_ZVAL(TRIE_G(tries), 1, 0);
	// RETURN_LONG(TRIE_G(count));
}
/* }}} */

/** {{{ int splite_string_to_array(char *str, int len, char **array)
*/
static int splite_string_to_array(char *str, int len, char **array)
{
    int i = 0, counter = 0;
    char **p = array;

    while (i < len) {
        if ((long)(unsigned char)str[i] >= 224) {
            char *s = strndup(str+i, 3);
            *p = s;
            i += 3;
        } else if ((long)(unsigned char)str[i] >= 192) {
            char *s = strndup(str+i, 2);
            *p = s;
            i += 2;
        } else {
            char *s = strndup(str+i, 1);
            *p = s;
            i += 1;
        }
        p++;
    }
    
    return p - array;
}
/* }}} */

/** {{{ destructor for persistent zval of clm local cache
 */
static void tries_zval_dtor(void *pDest) 
{
        zval *val;
        val = (zval *)pDest;
        switch (Z_TYPE_P(val) & IS_CONSTANT_TYPE_MASK){
                case IS_STRING:
                        CHECK_ZVAL_STRING(val);
                        pefree(Z_STRVAL_P(val), 1);
                        break;
                case IS_ARRAY:
                        zend_hash_destroy(Z_ARRVAL_P(val));
                        pefree(Z_ARRVAL_P(val), 1);
                        break;
        }
}
/* }}} */

/** {{{ persistent zval for tries local cache
 */
static zval * tries_zval_persistent(zval *val TSRMLS_DC)
{
        zval *pval;

        pval = (zval *)pemalloc(sizeof(zval), 1);
        INIT_PZVAL(pval);
        
        switch (Z_TYPE_P(val) & IS_CONSTANT_TYPE_MASK){
                case IS_BOOL:
                        ZVAL_BOOL(pval, Z_BVAL_P(val));
                        break;
                case IS_LONG:
                        ZVAL_LONG(pval, Z_LVAL_P(val));
                        break;
                case IS_DOUBLE:
                        ZVAL_DOUBLE(pval, Z_DVAL_P(val));
                        break;
                case IS_STRING:
                        CHECK_ZVAL_STRING(val);
                        Z_TYPE_P(pval) = IS_STRING;
                        Z_STRLEN_P(pval) = Z_STRLEN_P(val);
                        Z_STRVAL_P(pval) = pestrndup(Z_STRVAL_P(val), Z_STRLEN_P(val), 1);
                        break;
                case IS_ARRAY: {
                        char *str_index;
                        ulong num_index;
                        uint str_index_len;
                        zval **ele_val, *ele_pval;

                        Z_TYPE_P(pval) = IS_ARRAY;
                        Z_ARRVAL_P(pval) = (HashTable *)pemalloc(sizeof(HashTable), 1);
                        if (Z_ARRVAL_P(pval) == NULL){
                                return NULL;
                        }
                        zend_hash_init(Z_ARRVAL_P(pval), zend_hash_num_elements(Z_ARRVAL_P(val)), NULL, tries_zval_dtor, 1);

                        for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(val));
                                        !zend_hash_has_more_elements(Z_ARRVAL_P(val));
                                        zend_hash_move_forward(Z_ARRVAL_P(val))){
                                if (FAILURE == zend_hash_get_current_data(Z_ARRVAL_P(val), (void **)&ele_val)){
                                        continue;
                                }
                                ele_pval = clm_zval_persistent(*ele_val TSRMLS_CC);
                                if (ele_pval == NULL){
                                        continue;
                                }
                                if (HASH_KEY_IS_LONG == zend_hash_get_current_key_ex(Z_ARRVAL_P(val), &str_index, &str_index_len, &num_index, 0, NULL)){
                                        zend_hash_index_update(Z_ARRVAL_P(pval), num_index, (void **)&ele_pval, sizeof(zval), NULL);
                                } else {
                                        zend_hash_update(Z_ARRVAL_P(pval), str_index, str_index_len, (void **)&ele_pval, sizeof(zval), NULL);
                                }
                        }
                        break;
                }
                case IS_NULL:
                case IS_OBJECT:
                case IS_RESOURCE:
                        // ignore object and resource
                        ZVAL_NULL(pval);
                        break;
        }
        return pval;
}
/* }}} */

/* {{{ PHP_FUNCTION(trie_set)
*/
PHP_FUNCTION(trie_set)
{
    zval            *src, **node_data;
    zval            *first_node_data,*new_node_data;
    zval            **ppzval;
    HashTable       *node_ht;
    ulong           i, child = 1;
    ulong           **child_idx;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &src) == FAILURE) {
        RETURN_FALSE;
    }

    first_node_data = (zval *)pemalloc(sizeof(zval), 1);
    INIT_PZVAL(first_node_data);
    array_init(first_node_data);

    add_assoc_long_ex(first_node_data, "is_keyword_end", strlen("is_keyword_end")+1, 0);

    add_next_index_zval(TRIE_G(tries), first_node_data);

    zval_ptr_dtor(first_node_data);
    
    for(zend_hash_internal_pointer_reset(Z_ARRVAL_P(src));
                        zend_hash_has_more_elements(Z_ARRVAL_P(src)) == SUCCESS;
                        zend_hash_move_forward(Z_ARRVAL_P(src))) {

        if (zend_hash_get_current_data(Z_ARRVAL_P(src), (void **)&ppzval) == FAILURE) {
            RETURN_FALSE;
        }

        ulong current = 0;
        int count = 0;
        char **array = NULL, **p = NULL;

        array = (char **)ecalloc(sizeof(char *), Z_STRLEN_PP(ppzval));
        p = array;

        count = splite_string_to_array(Z_STRVAL_PP(ppzval), Z_STRLEN_PP(ppzval), p);

        int i = 0;
        for (i = 0; i < count; i++) {

            if (zend_hash_index_find(Z_ARRVAL_P(TRIE_G(tries)), current, (void **)&node_data) == SUCCESS
                && zend_hash_find(Z_ARRVAL_PP(node_data), *(p+i), strlen(*(p+i))+1, (void **)&child_idx) == SUCCESS) {
                    current = (int)**child_idx;    
                    continue;
            }

            
            MAKE_STD_ZVAL(new_node_data);
            array_init(new_node_data);

            if (i == count - 1) {
                add_assoc_long_ex(new_node_data, "is_keyword_end", strlen("is_keyword_end")+1, 1);
            } else {
                add_assoc_long_ex(new_node_data, "is_keyword_end", strlen("is_keyword_end")+1, 0);
            }

            if (add_next_index_zval(TRIE_G(tries), new_node_data) == FAILURE) {
                zval_ptr_dtor(new_node_data);  
                RETURN_FALSE;
            }

            add_assoc_long_ex(*node_data, *(p+i), strlen(*(p+i))+1, child);

            current = child++;       

            zval_ptr_dtor(new_node_data);  
        }

        efree(array);
    }

    RETURN_ZVAL(TRIE_G(tries), 1, 0);   
}
/* }}} */

/** {{{ PHP_FUNCTION(trie_match)
*/
PHP_FUNCTION(trie_match)
{
    ulong           current = 0, offset = 0, back = 0, count = 0;
    char            *str;
    char            **array = NULL, **p = NULL;
    ulong           str_len;
    zval            **node_data;
    ulong           **child_idx;
    zval            **is_match;
    smart_str       str_result = {0};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &str_len) == FAILURE) {
        RETURN_FALSE;
    }

    array = (char **)ecalloc(sizeof(char *), strlen(str));
    p = array;

    count = splite_string_to_array(str, str_len, p);
    
    for (offset = 0; offset < count; offset++) {  
        if (zend_hash_index_find(Z_ARRVAL_P(TRIE_G(tries)), current, (void **)&node_data) == SUCCESS
            && zend_hash_find(Z_ARRVAL_PP(node_data), *(p+offset), strlen(*(p+offset))+1, (void **)&child_idx) == SUCCESS) {
            
            current = (int)**child_idx;    

            zend_hash_index_find(Z_ARRVAL_P(TRIE_G(tries)), current, (void **)&node_data);

            zend_hash_find(Z_ARRVAL_PP(node_data), "is_keyword_end", strlen("is_keyword_end") + 1, (void **)&is_match);

            if (Z_LVAL_PP(is_match)) {
                smart_str_appendc(&str_result, '<');
                int i;
                for (i = back; i < offset + 1; i++) {
                    smart_str_appendl(&str_result, *(p+i), strlen(*(p+i))+1);    
                }
                smart_str_appendc(&str_result, '>');
                back = offset + 1;
                current = 0;
            }
        } else {
            int i;
                for (i = back; i < offset + 1; i++) {
                    smart_str_appendl(&str_result, *(p+i), strlen(*(p+i))+1);    
                }

            current = 0;
            back = offset+1;
        }
    }
    
    smart_str_0(&str_result);
    efree(array);

    ZVAL_STRINGL(return_value, str_result.c, str_result.len, 1);
    smart_str_free(&str_result);
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
