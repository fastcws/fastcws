// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <algorithm>

#include "fastcws/bindings/containers.hpp"

namespace fastcws {

namespace word_dag {

template <class Dag>
struct kahn_impl {
	using dag_t = Dag;
	using weight_t = typename dag_t::weight_t;

	struct result_t {
		vector<size_t> path;
		weight_t score; // lower the better
	};

	struct node_t {
		bool visited = false;
		weight_t lowest_score;
		size_t from = 0;
	};

	static result_t run(const dag_t& dag) {
		vector<size_t> in_degree{dag.in_degree()};
		vector<node_t> nodes{in_degree.size(), node_t{}};

		queue<size_t> s;
		nodes[dag.start()].visited = true;
		nodes[dag.start()].lowest_score = 0;
		s.push(dag.start());
		while(!s.empty()) {
			size_t from = s.back();
			s.pop();
			for (auto [to, weight] : dag.adjacents(from)) {
				weight_t new_score = nodes[from].lowest_score + weight;
				if (nodes[to].visited) {
					if (nodes[to].lowest_score > new_score) {
						nodes[to].from = from;
						nodes[to].lowest_score = new_score;
					}
				} else {
					nodes[to].from = from;
					nodes[to].lowest_score = new_score;
					nodes[to].visited = true;
				}
				in_degree[to]--;
				if (in_degree[to] == 0) {
					s.push(to);
				}
			}
		}

		result_t ret;
		size_t m = dag.end();
		for (;;) {
			m = nodes[m].from;
			if (m == dag.start()) {
				break;
			}
			ret.path.push_back(m);
		}
		std::reverse(ret.path.begin(), ret.path.end());
		ret.score = nodes[dag.end()].lowest_score;
		return ret;
	}

};

template <class Dag>
typename kahn_impl<Dag>::result_t kahn(const Dag& dag) {
	return kahn_impl<Dag>::run(dag);
}

}

}

