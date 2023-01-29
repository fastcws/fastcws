// SPDX-License-Identifier: BSD-2-Clause

#include <iostream>
#include <fstream>
#include <string_view>
#include <optional>

#include "fastcws.hpp"
#include "fastcws_defaults.hpp"

#ifdef _WIN32
#include "nowide/iostream.hpp"
#endif

int usage() {
	std::cerr
		<< "usage: echo \'想要分词的内容\' | fastcws [options]\n"
		<< "\n"
		<< "options:\n"
		<< "  --sep <separator>        specify separator added to output between\n"
		<< "  -s <separator>           words, defaults to -s '/' if not present\n"
		<< "\n"
		<< "  -0                       sets the separator to '\\0', byte 0x00\n"
		<< "\n"
		<< "  --dict <path/to/dict>    load and use a custom frequency dictionary\n"
		<< "  -d <path/to/dict>\n"
		<< "\n"
		<< "  --model <path/to/model>  load and use a custom HMM model, refer to help\n"
		<< "  -m <path/to/model>       text for utility hmm_train on how to train your\n"
		<< "                           own hmm model\n"
		<< "\n"
		<< "  --help                   show this help message\n"
		<< std::endl;
	return EXIT_FAILURE;
}

int main(int argc, char** argv) {
#ifdef _WIN32
	using nowide::cin;
	using nowide::cout;
#else
	using std::cin;
	using std::cout;
#endif
	cin.sync_with_stdio(false);
	cout.sync_with_stdio(false);

	const char* dict_filename = nullptr;
	const char* model_filename = nullptr;
	std::string_view sep = "/";

	for (int i = 1; i < argc;) {
		std::string_view sv{argv[i]};
		if ((sv == "-s") || (sv == "--sep")) {
			if ((i + 1) >= argc) {
				return usage();
			}
			sep = argv[i + 1];
			i++;
		} else if (sv == "-0") {
			sep = std::string_view{"\0", 1};
		} else if ((sv == "-d") || (sv == "--dict")) {
			if ((i + 1) >= argc) {
				return usage();
			}
			dict_filename = argv[i + 1];
			i++;
		} else if ((sv == "-m") || (sv == "--model")) {
			if ((i + 1) >= argc) {
				return usage();
			}
			model_filename = argv[i + 1];
			i++;
		} else if (sv == "--help") {
			(void) usage();
			return EXIT_SUCCESS;
		} else {
			return usage();
		}
		i++;
	}

	std::optional<fastcws::freq_dict::dict<>> custom_dict = std::nullopt;
	if (dict_filename == nullptr) {
		fastcws::defaults::init_freq_dict();
	} else {
		std::ifstream f{dict_filename};
		if (!f.good()) {
			std::cerr << "failed to load custom freq dict : " << dict_filename << std::endl;
			return EXIT_FAILURE;
		}
		custom_dict.emplace(fastcws::freq_dict::load_dict(f));
	}

	std::optional<fastcws::hmm::wseg_4tag::model<>> custom_model = std::nullopt;
	if (model_filename == nullptr) {
		fastcws::defaults::init_hmm_model();
	} else {
		std::ifstream f{model_filename};
		if (!f.good()) {
			std::cerr << "failed to load custom hmm model: " << dict_filename << std::endl;
			return EXIT_FAILURE;
		}
		custom_model.emplace(fastcws::hmm::wseg_4tag::load(f));
	}

	fastcws::istream_sentence_tokenizer tok{cin};
	std::string sentence;
	size_t i = 0;
	while (tok >> sentence) {
		std::vector<std::string_view> words;

		if (custom_dict.has_value() && custom_model.has_value()) {
			fastcws::word_break(sentence, std::back_inserter(words), *custom_dict, *custom_model);
		} else if (!custom_dict.has_value() && custom_model.has_value()) {
			fastcws::word_break(sentence, std::back_inserter(words), *fastcws::defaults::freq_dict, *custom_model);
		} else if (custom_dict.has_value() && !custom_model.has_value()) {
			fastcws::word_break(sentence, std::back_inserter(words), *custom_dict, *fastcws::defaults::hmm_model);
		} else {
			fastcws::word_break(sentence, std::back_inserter(words), *fastcws::defaults::freq_dict, *fastcws::defaults::hmm_model);
		}

		for (auto it = words.begin(); it != words.end(); it++) {
			if (i != 0) {
				cout << sep;
			}
			cout << *it;
			i++;
		}
		std::flush(cout);
	}
	return 0;
}

