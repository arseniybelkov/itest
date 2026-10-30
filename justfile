EXAMPLES_FOLDER := "examples"
TARGET_DIRECTORY := ".target"

[private]
@default:
    just --list

# Configures and runs example `simple.c`
simple:
    @just compile_debug_and_execute {{ EXAMPLES_FOLDER }}/simple.c

all_examples: simple

compile_debug_and_execute file:
    @mkdir -p {{ TARGET_DIRECTORY }}
    @echo "Building & Running {{ file }} ...\n"
    gcc -g -O0 {{ file }} -o {{ TARGET_DIRECTORY / file_stem(file) }}
    ./{{ TARGET_DIRECTORY / file_stem(file) }}
