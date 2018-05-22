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

#include "Help.hpp"
#include "OverlapCalculator.hpp"

#include <libgene/file/TxtFile.hpp>
#include <libgene/file/sequence/SequenceFile.hpp>
#include <libgene/utils/Tokenizer.hpp>
#include <libgene/utils/StringUtils.hpp>
#include <libgene/def/Flags.hpp>

#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <utility>

std::vector<std::string>
GetModuleList(std::unique_ptr<SequenceFile>& input_file,
              char delimiter)
{
    std::vector<std::string> module_list;
    SequenceRecord input_record;

    while (!(input_record = input_file->Read()).Empty()) {
        Tokenizer splitter(delimiter);
        splitter.SetText(input_record.seq);
        splitter.ReadNext();
        module_list.push_back(splitter.GetNextToken());
    }
    input_file->ResetFilePointer();
    return module_list;
}

int main(int argc, const char *argv[])
{
    ArgumentsParser arguments(argc, argv);
    
    if (arguments.input_file_path.empty()) {
        fprintf(stderr, "No input file provided. Terminating\n");
        return 1;
    }
    
    std::unique_ptr<SequenceFile> input_file;
    try {
        auto flags = std::make_unique<CommandLineFlags>();
        flags->SetSetting(Flags::kInputFormat, "txt");
        input_file = SequenceFile::FileWithName(arguments.input_file_path,
                                                flags,
                                                OpenMode::Read);
    } catch (std::runtime_error err) {
        if (!input_file) {
            fprintf(stderr, "File \'%s\' couldn't be opened. Either it doesn't\
 exist, or you don't have permissions to read it.\n",
                    arguments.input_file_path.c_str());
            return 1;
        }
    }
    
    size_t dot_position = arguments.input_file_path.find('.');
    std::string extension = utils::GetExtension(arguments.input_file_path);
    
    if (extension != "csvc" &&  extension != "tsvc") {
        // Force the output file to be interpreted as a 'columns-defined' file.
        extension += 'c';
    }
    
    if (arguments.output_file_path.empty())
        arguments.output_file_path = arguments.input_file_path.substr(0, dot_position);
    else {
        int64_t dot_position = arguments.output_file_path.rfind('.');
        if (dot_position != std::string::npos)
            arguments.output_file_path.erase(dot_position);
    }
    
    std::string overlap_file = arguments.output_file_path + "-overlap" + "." + extension;
    std::string percentage_overlap_file = arguments.output_file_path + "-percentage_overlap" + "." + extension;
    std::string non_overlap_file = arguments.output_file_path + "-non_overlap" + "." + extension;
    
    char delimiter = ',';
    if (extension == "tsv" || extension == "tsvc") {
        delimiter = '\t';
    }
    auto module_list = GetModuleList(input_file, delimiter);
    
    std::unique_ptr<SequenceFile> percentage_overlap_out_file;
    std::unique_ptr<SequenceFile> gene_overlap_out_file;
    std::unique_ptr<SequenceFile> non_overlap_out_file;
    
    auto CheckFileExistence = [&arguments](const std::string& file_path)
    {
        if (!arguments.override_output) {
            FILE *test_out_file = fopen(file_path.c_str(), "wx");
            if (test_out_file == nullptr) {
                fprintf(stdout, "File \'%s\' already exists. Do you wish to override it? [Y/n] ", file_path.c_str());
                char response;
                std::cin >> response;
                if (std::tolower(response) != 'y') {
                    fprintf(stderr, "Skipping file \'%s\'\n", file_path.c_str());
                    exit(1);
                }
            } else
                fclose(test_out_file);
        }
    };
    
    CheckFileExistence(percentage_overlap_file);
    CheckFileExistence(overlap_file);
    CheckFileExistence(non_overlap_file);
    
    auto OpenFileForWriting = [](const std::string& file_path)
    {
        auto flags = std::make_unique<CommandLineFlags>();
        flags->SetSetting(Flags::kOutputFormat, "txt");
        
        std::unique_ptr<SequenceFile> out_file;
        if (!(out_file = SequenceFile::FileWithName(file_path,
                                                    flags,
                                                    OpenMode::Write))) {
            fprintf(stderr, "Couldn't open the output file \'%s\'\n", file_path.c_str());
            exit(1);
        }
        return out_file;
    };
    
    percentage_overlap_out_file = OpenFileForWriting(percentage_overlap_file);
    gene_overlap_out_file = OpenFileForWriting(overlap_file);
    non_overlap_out_file = OpenFileForWriting(non_overlap_file);
    
    // Write header to output file.
    SequenceRecord header;
    header.seq = "Module";
    for (int i = 0; i < module_list.size(); ++i) {
        header.seq += delimiter;
        header.seq += module_list[i];
    }
    percentage_overlap_out_file->Write(header);
    gene_overlap_out_file->Write(header);
    non_overlap_out_file->Write(header);
    
    // A pair represents a module: module name and its gene set.
    std::vector<std::pair<std::string, std::set<std::string>>> modules;
    SequenceRecord input_record;
    while (!(input_record = input_file->Read()).Empty()) {
        Tokenizer splitter(delimiter);
        splitter.SetText(input_record.seq);
        
        splitter.ReadNext();
        std::string module_name = splitter.GetNextToken();
    
        std::set<std::string> gene_set;
        while (splitter.ReadNext()) {
            gene_set.insert(splitter.GetNextToken());
        }
        modules.push_back({std::move(module_name), std::move(gene_set)});
    }
    
    for (int i = 0; i < modules.size(); ++i) {
        SequenceRecord overlap_record;
        SequenceRecord overlap_percentage_record;
        SequenceRecord non_overlap_record;
        
        overlap_record.seq += modules[i].first;
        overlap_percentage_record.seq += modules[i].first;
        non_overlap_record.seq += modules[i].first;
        
        for (int j = 0; j < modules.size(); ++j) {
            overlap_record.seq += delimiter;
            overlap_percentage_record.seq += delimiter;
            non_overlap_record.seq += delimiter;
            
            if (i == j)
                continue;
 
            // Overlap reporting
            auto overlap = GetIntersection(modules[i].second, modules[j].second);
            overlap_record.seq += "\"";
            for (const std::string& gene : overlap) {
                if (gene.empty())
                    // I don't know why empty 'genes' sometimes end up in this vector. Fix later?
                    continue;
                
                overlap_record.seq += gene;
                overlap_record.seq += delimiter;
            }
            // Erase the last delimiter
            if (overlap.size() > 1)
                overlap_record.seq.erase(overlap_record.seq.size() - 1);
            
            overlap_record.seq += "\"";

            // Overlap percentage reporting
            double percentage = GetIntersectionPercentage(modules[i].second.size(), modules[j].second.size(), overlap.size() - 1);
            overlap_percentage_record.seq += std::to_string(percentage);
            
            // Non-overlap reporting
            auto non_overlap = GetDifference(modules[i].second, modules[j].second);
            non_overlap_record.seq += "\"";
            for (const std::string& gene : non_overlap) {
                if (gene.empty()) {
                    continue;
                }
                
                non_overlap_record.seq += gene;
                non_overlap_record.seq += delimiter;
            }
            // Erase the last delimiter
            if (non_overlap.size() > 1)
                non_overlap_record.seq.erase(non_overlap_record.seq.size() - 1);
            non_overlap_record.seq += "\"";
        }
        gene_overlap_out_file->Write(overlap_record);
        percentage_overlap_out_file->Write(overlap_percentage_record);
        non_overlap_out_file->Write(non_overlap_record);
    }
    
    fprintf(stdout, "The output files are located in \n\t%s\n\t%s\n\t%s\n", percentage_overlap_file.c_str(),
            overlap_file.c_str(), non_overlap_file.c_str());
    return 0;
}
