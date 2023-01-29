// SPDX-License-Identifier: BSD-2-Clause

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <list>
#include <vector>

#include "fastcws/hmm.hpp"

void usage(std::string_view program) {
	std::cerr << "usage: " << program << " <wseg_model.hmm> [training00.txt] [training01.txt] ..\n"
		<< "\n"
		<< "examples:\n"
		<< "$" << program << " wseg_model.hmm\n"
		<< "dump model parameters\n"
		<< "\n"
		<< "$" << program << " wseg_model.hmm training00.txt training01.txt\n"
		<< "create model if not exist, train model with provided data\n"
		<< "if model already exist, will train progressively\n"
		<< "results will be saved to model file, and dumped to console\n"
		<< std::endl;
}

int main(int argc, char** argv) {
	namespace hmm = fastcws::hmm;

	if (argc < 2) {
		usage({argv[0]});
		return 1;
	}

	size_t training_count = argc - 2;
	hmm::wseg_4tag::model<> model;
	{
		std::ifstream ifs{argv[1]};
		if (ifs.good()) {
			model = hmm::wseg_4tag::load<decltype(model)>(ifs);
		} else {
			if (!training_count) {
				usage({argv[0]});
				return 1;
			}
		}
	}

	for (size_t i = 0; i < training_count; i++) {
		std::ifstream ifs{argv[2 + i]};
		if (!ifs.good()) {
			std::cerr << "error: " << argv[2 + i] << " good() == false, skipping .." << std::endl;
			continue;
		}

		std::list<std::string> lines;
		std::vector<std::string_view> characters;
		std::vector<hmm::wseg_4tag::state> tags;
		bool ended = false;
		while (!ended) {
			for (;;) {
				std::string line;
				if(!std::getline(ifs, line, '\n')) {
					ended = true;
					break;
				}
				if (line == "") continue;
				if (line == "SENTENCE END") break;
				if (line == "TEXT END") break;
				assert(line.size() >= 3);
				assert(line[line.size() - 2] == ' ');

				char tag = line[line.size() - 1];
				lines.emplace_back(std::move(line));
				std::string_view character = std::string_view{lines.back()}.substr(0, lines.back().size() - 2);
				characters.push_back(character);

				hmm::wseg_4tag::state tag_e;
				switch (tag) {
					case 'B':
						tag_e = decltype(tag_e)::B;
						break;
					case 'M':
						tag_e = decltype(tag_e)::M;
						break;
					case 'E':
						tag_e = decltype(tag_e)::E;
						break;
					case 'S':
						tag_e = decltype(tag_e)::S;
						break;
					default:
						std::cerr << "invalid tag : " << tag << std::endl;
						return 1;
				}
				tags.push_back(tag_e);
			}
			model.train(characters.cbegin(), characters.cend(), tags.cbegin(), tags.cend());
			characters.clear();
			tags.clear();
			lines.clear();
		}
	}
	
	if (training_count > 0) {
		std::ofstream ofs{argv[1]};
		hmm::wseg_4tag::save(model, ofs);
	}

	model.normalize();
	model.dump(std::cout);
	if (model.trival()) {
		std::cerr << "warning: model is trival" << std::endl;
	}
	std::flush(std::cout);
	return 0;
}
