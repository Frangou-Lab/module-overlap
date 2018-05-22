/*
 * Copyright 2018 Frangou Lab
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef Help_h
#define Help_h

#include <string>
#include <iostream>

inline void PrintHelp(FILE *destination)
{
    printf("\
OVERVIEW:\n\
    modules_overlap v0.1\n\
    For a matrix of ‘modules’ compares each module to all other entries and identifies %% of\n\
    similarity between them. In addition, generates columns summarizing overlapping and \n\
    non-overlapping values.\n\
");
    fprintf(destination, "\
USAGE:\n\
    modules_overlap <input table> [-o <output path>] [options]\n\
OPTIONS:\n\
    -h                     - Show this message.\n\
    -f                     - Override the output files even if they already exist.\n\
EXAMPLES:\n\
modules_overlap ~/input.csv                   - Using the matrix of 'modules' ~/input.csv calculate:\n\
                                                    - gene overlap table '~/input-overlap.csvc'.\n\
                                                    - gene percentage overlap '~/input-percentage.csvc'.\n\
                                                    - gene non-overlap table '~/input-non_overlap.csvc'.\n\
modules_overlap ~/input.csv -o ~/output.csvc  - Using the matrix of 'modules' ~/input.csv calculate:\n\
                                                    - gene overlap table '~/output-overlap.csvc'.\n\
                                                    - gene percentage overlap '~/output-percentage.csvc'.\n\
                                                    - gene non-overlap table '~/output-non_overlap.csvc'.\n\
");
}

class ArgumentsParser {
 public:
    std::string input_file_path;
    std::string output_file_path;
    
    bool verbose_output{false};
    bool override_output{false};
    
    ArgumentsParser(int argc, const char *argv[])
    {
        if (argc == 1) {
            PrintHelp(stdout);
            return;
        }
        
        for (int i = 1; i < argc; ++i) {
            std::string argument = argv[i];
            if (argument == "-v" || argument == "--verbose") {
                verbose_output = true;
            }
            else if (argument == "-o" || argument == "--output") {
                i++;
                if (i == argc || argv[i][0] == '-') {
                    fprintf(stderr, "No valid output path has been entered.\n");
                    std::abort();
                }
                output_file_path.assign(argv[i]);
            } else if (argument == "-h" || argument == "--help") {
                PrintHelp(stdout);
                std::exit(0);
            } else if (argument == "-f" || argument == "--force") {
                override_output = true;
            } else if (argument[0] != '-' && input_file_path.empty()) {
                input_file_path.assign(argument);
            } else {
                PrintHelp(stderr);
                fprintf(stderr, "Unknown flag '%s'\n", argument.c_str());
                std::exit(1);
            }
        }
    }
};

#endif /* Help_h */
