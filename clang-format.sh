# Run this before commiting

git ls-files '*.c' '*.cc' '*.h' '*.hh' '*.cpp' '*.hpp' | xargs clang-format -i -style=file