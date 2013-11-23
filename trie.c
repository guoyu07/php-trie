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
	PHP_FE(confirm_trie_compiled,	NULL)		/* For testing, remove later. */
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

    #else
        TRIE_G(tries) = (zval *)pemalloc(sizeof(zval), 1);
        INIT_PZVAL(TRIE_G(tries));
        array_init(TRIE_G(tries));

        TRIE_G(count) = (zval *)pemalloc(sizeof(zval), 1);
        INIT_PZVAL(TRIE_G(count));
        ZVAL_LONG(TRIE_G(count), 0);
    #endif

	return SUCCESS;
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
PHP_FUNCTION(confirm_trie_compiled)
{
    RETURN_ZVAL(TRIE_G(tries), 1, 0);
	// RETURN_LONG(TRIE_G(count));
}
/* }}} */

/** {{{ destructor for persistent zval of trie local cache
 */
void trim_zval_dtor(void *pDest) {
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

/* {{{ PHP_FUNCTION(trie_set)
*/
PHP_FUNCTION(trie_set)
{
    zval            *src, **node_data;
    zval            *first_node_data,*new_node_data;
    zval            **ppzval;
    HashTable       *src_ht, *node_ht;
    ulong           i, child = 1;
    ulong           **child_idx;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &src) == FAILURE) {
        RETURN_FALSE;
    }

    src_ht = Z_ARRVAL_P(src);

    MAKE_STD_ZVAL(first_node_data);
    array_init(first_node_data);
    add_assoc_long_ex(first_node_data, "is_keyword_end", strlen("is_keyword_end")+1, 0);
    zval_add_ref(&first_node_data);
    add_next_index_zval(TRIE_G(tries), first_node_data);
    
    for(zend_hash_internal_pointer_reset(src_ht);
                        zend_hash_has_more_elements(src_ht) == SUCCESS;
                        zend_hash_move_forward(src_ht)) {

        if (zend_hash_get_current_data(src_ht, (void **)&ppzval) == FAILURE) {
            RETURN_FALSE;
        }

        ulong current = 0;
        
        for (i = 0; i < Z_STRLEN_PP(ppzval); i++) {

            char *key = estrndup(Z_STRVAL_PP(ppzval)+i, 1);

            if (zend_hash_index_find(Z_ARRVAL_P(TRIE_G(tries)), current, (void **)&node_data) == SUCCESS
                && zend_hash_find(Z_ARRVAL_PP(node_data), key, 2, (void **)&child_idx) == SUCCESS) {
                    current = (int)**child_idx;    
                    efree(key);  
                    continue;
            }

            
            MAKE_STD_ZVAL(new_node_data);
            array_init(new_node_data);

            if (i == Z_STRLEN_PP(ppzval) - 1) {
                add_assoc_long_ex(new_node_data, "is_keyword_end", strlen("is_keyword_end")+1, 1);
            } else {
                add_assoc_long_ex(new_node_data, "is_keyword_end", strlen("is_keyword_end")+1, 0);
            }
            // zval_copy_ctor(new_node_data);
            if (add_next_index_zval(TRIE_G(tries), new_node_data) == FAILURE) {
                efree(key);  
                RETURN_FALSE;
            }

            add_assoc_long_ex(*node_data, key, 2, child);

            current = child++;      

            efree(key);    
        }
    }

    RETURN_ZVAL(TRIE_G(tries), 1, 0);   
}
/* }}} */

/** {{{ PHP_FUNCTION(trie_match)
*/
PHP_FUNCTION(trie_match)
{
    ulong           current = 0, offset = 0, back = 0;
    char            *str;
    ulong           str_len;
    zval            **node_data;
    ulong           **child_idx;
    zval            **is_match;
    smart_str       str_result = {0};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &str_len) == FAILURE) {
        RETURN_FALSE;
    }
    
    str_len = (ulong)strlen(str);
    for (offset = 0; offset < str_len; offset++) {
        char *key = estrndup(str + offset, 1);    
        
        if (zend_hash_index_find(Z_ARRVAL_P(TRIE_G(tries)), current, (void **)&node_data) == SUCCESS
            && zend_hash_find(Z_ARRVAL_PP(node_data), key, 2, (void **)&child_idx) == SUCCESS) {
            
            current = (int)**child_idx;    

            zend_hash_index_find(Z_ARRVAL_P(TRIE_G(tries)), current, (void **)&node_data);

            zend_hash_find(Z_ARRVAL_PP(node_data), "is_keyword_end", strlen("is_keyword_end") + 1, (void **)&is_match);

            if (Z_LVAL_PP(is_match)) {
                char *word = estrndup(str + back, offset - back + 1);
                smart_str_appendc(&str_result, '<');
                smart_str_appendl(&str_result, word, strlen(word));
                smart_str_appendc(&str_result, '>');
                back = offset + 1;
                current = 0;

                efree(word);
            }
        } else {
            // printf("back = %ld, offset = %ld\n", back, offset);
            char *word = estrndup(str + back, offset - back + 1);
            // printf("else word=%s\n", word);
            smart_str_appendl(&str_result, word, strlen(word));

            current = 0;
            // offset = back;
            back = offset+1;

            efree(word);
        }

        efree(key);
        // printf("str_result.c = %s\n", str_result.c);
    }
     smart_str_0(&str_result);

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
