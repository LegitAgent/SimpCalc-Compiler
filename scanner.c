#include <stdio.h>
#include "scanner.h"

void scanner(FILE* file_ptr_r, FILE* file_ptr_w) {
    int c; // cur char
    // character buffer for digits (string)
    char buffer[256]; // so max digit len would be 255, + 1 for null terminator
    int buffer_idx = 0;

    // https://stackoverflow.com/questions/4823177/reading-a-file-character-by-character-in-c
    // iterate over file character by character
    while ((c = fgetc(file_ptr_r)) != EOF) {
        
    }
}
