dnl $Id$
dnl config.m4 for extension trie

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(trie, for trie support,
dnl Make sure that the comment is aligned:
dnl [  --with-trie             Include trie support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(trie, whether to enable trie support,
Make sure that the comment is aligned:
[  --enable-trie           Enable trie support])

if test "$PHP_TRIE" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-trie -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/trie.h"  # you most likely want to change this
  dnl if test -r $PHP_TRIE/$SEARCH_FOR; then # path given as parameter
  dnl   TRIE_DIR=$PHP_TRIE
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for trie files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       TRIE_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$TRIE_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the trie distribution])
  dnl fi

  dnl # --with-trie -> add include path
  dnl PHP_ADD_INCLUDE($TRIE_DIR/include)

  dnl # --with-trie -> check for lib and symbol presence
  dnl LIBNAME=trie # you may want to change this
  dnl LIBSYMBOL=trie # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $TRIE_DIR/lib, TRIE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_TRIELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong trie lib version or lib not found])
  dnl ],[
  dnl   -L$TRIE_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(TRIE_SHARED_LIBADD)

  PHP_NEW_EXTENSION(trie, trie.c, $ext_shared)
fi
