#!/bin/bash
set -e

# Default values
OUTPUT=""
ENTRY="main"

# Function to show help
show_help() {
    echo "Usage: $0 -i input.tc [-o output.asm] [-e entry_function] [-h]"
    echo "Options:"
    echo "  -i <file>     Input file to compile (required)"
    echo "  -o <file>     Output file (default: [Input File Name].asm)"
    echo "  -e <func>     Entry point function (default: main)"
    echo "  -h            Show this help message"
    exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -i)
            INPUT="$2"
            shift 2
            ;;
        -o)
            OUTPUT="$2"
            shift 2
            ;;
        -e)
            ENTRY="$2"
            shift 2
            ;;
        -h)
            show_help
            ;;
        *)
            echo "Unknown argument: $1"
            show_help
            ;;
    esac
done

# Validate input
if [ -z "$INPUT" ]; then
    echo "Input file is required (-i input.tc)"
    show_help
fi

# Set default output if not provided
if [ -z "$OUTPUT" ]; then
    BASE="${INPUT%.tc}"
    OUTPUT="$BASE.asm"
else
    BASE="${OUTPUT%.asm}"
fi

# Build compiler
echo "Building compiler..."
gcc -O2 -Wall -std=c11 -o tc2 compiler.c

# Run compiler
echo "Running compiler..."
./tc2 -i "$INPUT" -o "$OUTPUT" -e "$ENTRY"

# Assemble
echo "Assembling..."
nasm -f elf64 "$OUTPUT" -o "$BASE.o"

# Link
echo "Linking..."
ld "$BASE.o" -o "$BASE"

# Run compiled program
echo -e "\nOUTPUT PROGRAM:\n"
./"$BASE"
echo -e "\n"

# make clean
