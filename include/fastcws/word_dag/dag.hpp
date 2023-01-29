// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <string_view>
#include <ostream>
#include <sstream>

#include "fastcws/bindings/containers.hpp"

namespace fastcws {

namespace word_dag {

template <class Weight = long double>
struct dag {
	using weight_t = Weight;

	std::string_view sentence_;
	vector<map<size_t, weight_t>> adjacents_;
	vector<size_t> in_degree_;

	dag(std::string_view sentence)
		: sentence_(sentence),
		  adjacents_(sentence_.size() + 1, map<size_t, weight_t>{}),
		  in_degree_(sentence.size() + 1, 0) {}

	size_t start() const noexcept {
		return 0;
	}

	size_t end() const noexcept {
		return sentence_.size();
	}

	std::string_view sentence() const noexcept {
		return sentence_;
	}

	const vector<size_t>& in_degree() const noexcept {
		return in_degree_;
	}

	const map<size_t, weight_t>& adjacents(size_t of) const noexcept {
		return adjacents_[of];
	}

	void add_edge(size_t from, size_t to, weight_t weight) {
		if (adjacents_[from].count(to)) {
			if (adjacents_[from][to] > weight) {
				adjacents_[from][to] = weight;
			}
		} else {
			adjacents_[from][to] = weight;
			in_degree_[to]++;
		}
	}

	static std::string graphviz_quote(std::string s) {
		std::ostringstream oss;
		oss << '\"';
		for (char ch : s) {
			if (ch == '\"') {
				oss << '\\' << '\"';
			} else {
				oss << ch;
			}
		}
		oss << '\"';
		return oss.str();
	}

	void dump_graphviz_dot(std::ostream& os) {
		os << "digraph {\n";
		for (size_t from = 0; from < adjacents_.size(); from++) {
			for (auto [to, weight] : adjacents_[from]) {
				os << "  "; //indent
				if (from == start()) {
					os << "start";
				} else {
					os << from;
				}
				os << " -> ";
				if (to == end()) {
					os << "end";
				} else {
					os << to;
				}
				std::ostringstream label_oss;
				label_oss << sentence_.substr(from, to - from);
				label_oss << "(weight=" << weight << ")";
				os << " [label=" << graphviz_quote(label_oss.str()) << "]\n";
			}
		}
		os << "}\n";
	}
};

}

}

