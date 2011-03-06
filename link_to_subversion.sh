#!/bin/sh

# check whether svn config is present in git config
echo "Checking git-svn configuration..."
if grep -qE "^\[svn-remote" .git/config; then
        echo "   git-svn configuration is up to date."
else
        echo "   updating git-svn configuration:"
        echo
        cat git/config.add
        echo ">>.git/config"
        cat git/config.add >>.git/config
fi
echo

echo
echo "Copying subversion references and metadata..."
cp -a git/svn .git/
echo "done."

echo
echo "Copying svn remote branch refs..."
cp -av git/refs/remotes/* .git/refs/remotes/
echo "done."

echo
echo "You can now commit local changes with"
echo
echo "  git svn dcommit"
echo
echo "as well as use any other subversion related git commands."
echo

