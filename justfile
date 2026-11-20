EXAMPLES_FOLDER := "examples"
TARGET_DIRECTORY := ".target"
CC := "clang"

# Configures and runs example `examples/simple.c`
simple:
    @just CC={{ CC }} compile_debug_and_execute simple {{ EXAMPLES_FOLDER }}/simple.c

# Configures and runs example `examples/two_files_static_linking`
two_files_static_linking:
    @just CC={{ CC }} compile_debug_and_execute two_files_static_linking \
    {{ EXAMPLES_FOLDER }}/two_files_static_linking/m.c \
    {{ EXAMPLES_FOLDER }}/two_files_static_linking/main.c 
    
all_examples: simple two_files_static_linking

compile_debug_and_execute exec_name *files: make_target_dir
    @echo "Building & Running {{ files }} ...\n"
    {{ CC }} -g -O0 {{ files }} -o {{ TARGET_DIRECTORY / exec_name }}
    ./{{ TARGET_DIRECTORY / exec_name }}

[private]
@make_target_dir:
    mkdir -p {{ TARGET_DIRECTORY }}

[private]
@default:
    just --list
