#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "parser.h"

int main() {
    // https://www.geeksforgeeks.org/c/c-program-list-files-sub-directories-directory/
    DIR *dir_ptr = opendir("."); // open current directory
    if (dir_ptr == NULL) {
        printf("Unknown directory, could not locate file.\n");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir_ptr)) != NULL) {
        // prevent infinite loop, like input_input, just don't name sample files those lmao.
        if (strstr(entry->d_name, "output_scan") != NULL || 
            strstr(entry->d_name, "output_parser") != NULL) {
            continue;
        }
        
        // https://www.w3schools.com/c/ref_string_strstr.php
        char *input_pos = strstr(entry->d_name, "input"); // needle in haystack substr is "input" in this case
        // no needle (input) substr
        if (input_pos == NULL) {
            continue;
        }

        // find file
        // https://www.geeksforgeeks.org/c/basics-file-handling-c/
        // https://stackoverflow.com/questions/32674141/if-file-pointer-is-null-do-i-have-to-use-fclose-c
        FILE* file_ptr_r_main = fopen(entry->d_name, "r");
        if (file_ptr_r_main == NULL) {
            printf("Unknown file name, could not locate file.\n");
            return 1;
        }

        // https://www.geeksforgeeks.org/c/snprintf-c-library/
        // https://www.geeksforgeeks.org/c/format-specifiers-in-c/
        char output_name_scanner[256]; // name max = 256 characters
        size_t before_input_len = input_pos - entry->d_name; // subtraction of mem addresses

        // write to output_name with format
        snprintf(output_name_scanner, // buffer to append
            sizeof(output_name_scanner), // size of buffer
            "%.*soutput_scan%s", // string format - length (int) + string1 + literal "output" + string2, .* = int length of str
            (int) before_input_len,
            entry->d_name,
            input_pos + strlen("input") // everything after "input"
        );

        FILE* file_ptr_w_scanner = fopen(output_name_scanner,"w");
        if (file_ptr_w_scanner == NULL) {
            printf("Could not create file.\n");
            fclose(file_ptr_r_main);
            return 1;
        }

        char output_name_parser[256]; // name max = 256 characters

        // write to output_name with format
        snprintf(output_name_parser,
            sizeof(output_name_parser),
            "%.*soutput_parse%s",
            (int) before_input_len,
            entry->d_name,
            input_pos + strlen("input")
        );

        FILE* file_ptr_w_parser = fopen(output_name_parser,"w");
        if (file_ptr_w_parser == NULL) {
            printf("Could not create file.\n");
            return 1;
        }

        bool success = parser(file_ptr_r_main, file_ptr_w_scanner, file_ptr_w_parser);
        if (success) fprintf(file_ptr_w_parser, "%s is a valid SimpCalc program\n", entry->d_name);
        
        // close for no memory leak
        fclose(file_ptr_w_scanner);
        fclose(file_ptr_r_main);
        fclose(file_ptr_w_parser);
    }
    closedir(dir_ptr);
}
