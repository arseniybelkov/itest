EXAMPLES_FOLDER := "examples"
TARGET_DIRECTORY := ".target"
CC := "clang"

# Configures and runs example `simple.c`
simple:
    @just CC={{ CC }} compile_debug_and_execute {{ EXAMPLES_FOLDER }}/simple.c

all_examples: simple

compile_debug_and_execute file: make_target_dir
    @echo "Building & Running {{ file }} ...\n"
    {{ CC }} -g -O0 {{ file }} -o {{ TARGET_DIRECTORY / file_stem(file) }}
    ./{{ TARGET_DIRECTORY / file_stem(file) }}

[private]
@make_target_dir:
    mkdir -p {{ TARGET_DIRECTORY }}

[private]
@default:
    just --list
