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

#ifndef PHP_TRIE_H
#define PHP_TRIE_H

extern zend_module_entry trie_module_entry;
#define phpext_trie_ptr &trie_module_entry

#ifdef PHP_WIN32
#	define PHP_TRIE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_TRIE_API __attribute__ ((visibility("default")))
#else
#	define PHP_TRIE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif



PHP_MINIT_FUNCTION(trie);
PHP_MSHUTDOWN_FUNCTION(trie);
PHP_RINIT_FUNCTION(trie);
PHP_RSHUTDOWN_FUNCTION(trie);
PHP_MINFO_FUNCTION(trie);

PHP_FUNCTION(confirm_trie_compiled);	/* For testing, remove later. */
PHP_FUNCTION(trie_set);
PHP_FUNCTION(trie_match);
   

ZEND_BEGIN_MODULE_GLOBALS(trie)
      zval     *count;
      zval     *tries;
ZEND_END_MODULE_GLOBALS(trie)

void trim_zval_dtor(void *pDest);


/* In every utility function you add that needs to use variables 
   in php_trie_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as TRIE_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define TRIE_G(v) TSRMG(trie_globals_id, zend_trie_globals *, v)
#else
#define TRIE_G(v) (trie_globals.v)
#endif

#endif	/* PHP_TRIE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
