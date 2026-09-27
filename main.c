#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

int main() {
    // https://www.geeksforgeeks.org/c/c-program-list-files-sub-directories-directory/
    DIR *dir_ptr = opendir("."); // open current directory
    if (dir_ptr == NULL) {
        printf("Unknown directory, could not locate file.\n");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir_ptr)) != NULL) {
        // https://www.w3schools.com/c/ref_string_strstr.php
        char *input_pos = strstr(entry->d_name, "input"); // needle in haystack substr is "input" in this case
        // no needle (input) substr
        if (input_pos == NULL) {
            continue;
        }

        // find file
        // https://www.geeksforgeeks.org/c/basics-file-handling-c/
        // https://stackoverflow.com/questions/32674141/if-file-pointer-is-null-do-i-have-to-use-fclose-c
        FILE* file_ptr_r = fopen(entry->d_name, "r");
        if (file_ptr_r == NULL) {
            printf("Unknown file name, could not locate file.\n");
            return 1;
        }

        // https://www.geeksforgeeks.org/c/snprintf-c-library/
        // https://www.geeksforgeeks.org/c/format-specifiers-in-c/
        char output_name[256]; // name max = 256 characters
        size_t before_input_len = input_pos - entry->d_name; // subtraction of mem addresses

        // write to output_name with format (TODO: scanner part)
        snprintf(output_name, // buffer to append
            sizeof(output_name), // size of buffer
            "%.*soutput%s", // string format - length (int) + string1 + literal "output" + string2, .* = int length of str
            (int) before_input_len,
            entry->d_name,
            input_pos + strlen("input") // everything after "input"
        );

        FILE* file_ptr_w = fopen(output_name,"w");
        if (file_ptr_w == NULL) {
            printf("Could not create file.\n");
            return 1;
        }

        int c; // cur char
        // character buffer for digits (string)
        char buffer[256]; // so max digit len would be 255, + 1 for null terminator
        int buffer_idx = 0;

        // https://stackoverflow.com/questions/4823177/reading-a-file-character-by-character-in-c
        // iterate over file character by character
        // scanner while
        while ((c = fgetc(file_ptr_r)) != EOF) {
            
        }

        // get the file we just wrote, read that
        // read off of scanner

        fclose(file_ptr_r); // close for no memory leak
        fclose(file_ptr_w);
    }
    closedir(dir_ptr);
}
