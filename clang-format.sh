git ls-tree --full-tree --name-only -r HEAD | grep -e ".*\.\(c\|h\|hpp\|cc\|cpp\|hh\)\$" | grep -vf .clang-format-ignore | xargs clang-format -i -style=file --verbose