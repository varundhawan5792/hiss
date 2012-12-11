(echo -n $PATH | tr : '\0' | xargs -0 -n 1 ls; alias | sed 's/alias \([^=]*\)=.*/\1/') | sort -u > compgen.out
