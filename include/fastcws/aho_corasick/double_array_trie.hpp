// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <string_view>
#include <memory>
#include <utility>
#include <limits>

#include "fastcws/bindings/containers.hpp"
#include "fastcws/rep_aware/string_view.hpp"

namespace fastcws {

namespace aho_corasick {

template <class Allocator = std::allocator<int>>
struct double_array_trie {
	using offset_t = uint32_t;
	using allocator_traits = std::allocator_traits<Allocator>;
	using string_view_type = rep_aware::basic_string_view<char, std::char_traits<char>,
		  typename allocator_traits::template rebind_alloc<char>>;

	struct unit_type {
		static constexpr size_t not_used = std::numeric_limits<offset_t>::max();
		offset_t base = not_used;
		offset_t check = not_used;
		offset_t fail = 0;
		offset_t tail = 0;

		bool used() const noexcept {
			return (base != not_used) || (check != not_used);
		}
	};

	struct tail_type {
		string_view_type match;
		size_t tail_size = 0;
	};

	vector<unit_type,
		typename allocator_traits::template rebind_alloc<
			unit_type>> units_;
	vector<tail_type,
		typename allocator_traits::template rebind_alloc<
			tail_type>> tails_;

	double _calc_load_factor(size_t begin, size_t end) {
		size_t used = 0;
		for (size_t i = begin; i != end; i++) {
			if (units_.size() <= i) {
				break;
			}
			if (units_[i].used()) {
				used++;
			}
		}
		return static_cast<double>(used) / (end - begin);
	}

	template <class Trie>
	void build_from(const Trie& tr, bool quiet=false) {
		units_ = {};
		tails_ = {};

		enum class merge_status { none, merged_to, merged_from };
		vector<merge_status> mstatus(tr.nodes_.size(), merge_status::none);
		size_t need_process_nodes = tr.nodes_.size();

		vector<size_t> nodes_to_tails(tr.nodes_.size(), 0);
		tails_.emplace_back(); // sentinel
		for (const auto& node : tr.nodes_) {
			if (node.result.size() != 0) {
				tail_type tail;
				tail.match = {node.result.data(), node.result.size()};
				const auto* curr_node = &node;
				if (node.children.empty()) {
					const auto* parent = &tr.nodes_[curr_node->parent];
					while((parent->fail == curr_node->fail) &&
						(parent->result.size() == 0) &&
						(parent->children.size() == 1) &&
						(parent->id != 0)) {
						mstatus[curr_node->id] = merge_status::merged_from;
						mstatus[parent->id] = merge_status::merged_to;
						tail.tail_size++;
						curr_node = parent;
						parent = &tr.nodes_[curr_node->parent];
						need_process_nodes--;
					}
				}
				nodes_to_tails[curr_node->id] = tails_.size();
				tails_.emplace_back(std::move(tail));
			}
		}
		assert(tails_.size() < std::numeric_limits<offset_t>::max());

		vector<size_t> node_id_to_unit_idx(tr.nodes_.size(), 0);
		queue<size_t> q;
		q.push(0);
		size_t count = 0;
		size_t skip = 0;
		while (!q.empty()) {
			if (!quiet) {
				if ((count % 1000) == 0) {
					double pct = count * 100.0 / need_process_nodes;
					std::cout << "\r" // remove curr line
						<< pct << "% (" << count << "/" << need_process_nodes << ") ...     ";
					std::flush(std::cout);
				}
			}
			count++;

			size_t id = q.front();
			q.pop();

			if (mstatus[id] == merge_status::merged_from) {
				continue;
			}

			const auto& node = tr.nodes_[id];
			size_t new_base = skip;
			for (;;) {
				if (units_.size() <= new_base) {
					break;
				}
				if (units_[new_base].used()) {
					new_base++;
					continue;
				}
				bool good = true;
				for (auto [ch, child_id] : node.children) {
					size_t place_child = new_base + static_cast<uint8_t>(ch);
					if (units_.size() <= place_child) {
						break;
					}
					if (units_[place_child].used()) {
						good = false;
						break;
					}
				}
				if (good) {
					break;
				}
				new_base++;
			}
			if ((_calc_load_factor(skip, new_base) > 0.80) || ((new_base - skip) > 5000)) {
				skip = new_base;
			}
			if (units_.size() <= (new_base + 0xff)) {
				units_.resize(new_base + 0xff + 1);
				assert(units_.size() < std::numeric_limits<offset_t>::max());
			}
			units_[node_id_to_unit_idx[id]].base = static_cast<offset_t>(new_base);
			for (auto [ch, child_id] : node.children) {
				size_t place_child = new_base + static_cast<uint8_t>(ch);
				if (units_.size() <= place_child) {
					units_.resize(place_child + 1);
					assert(units_.size() < std::numeric_limits<offset_t>::max());
				}
				units_[place_child].check = static_cast<offset_t>(node_id_to_unit_idx[id]);
				units_[place_child].fail = static_cast<offset_t>(node_id_to_unit_idx[tr.nodes_[child_id].fail]);
				units_[place_child].tail = static_cast<offset_t>(nodes_to_tails[child_id]);
				node_id_to_unit_idx[child_id] = place_child;

				if (mstatus[id] != merge_status::merged_to) {
					q.push(child_id);
				}
			}
		}

		if (!quiet) {
			std::cout << "\r" // remove curr line
				<< "done.      " << std::endl;
		}
	}

	template <class MatchCallback>
	void scan(const std::string_view haystack, MatchCallback matched) const {
		size_t status = 0;
		for (size_t i = 0; i < haystack.size();) {
			const size_t base = units_[status].base;
			const size_t to = base + static_cast<uint8_t>(haystack[i]);
			if (units_[to].check == status) {
				status = to;
				i++;
				size_t mstatus = status;
				while (mstatus != 0) {
					if (units_[mstatus].tail != 0) {
						const auto& tail = tails_[units_[mstatus].tail];
						std::string_view match_conv = {tail.match.data(), tail.match.size()};
						if (haystack.compare(i, tail.tail_size, match_conv, 0, tail.tail_size) == 0) {
							matched(i + tail.tail_size, std::string_view{tail.match.data(), tail.match.size()});
						}
					}
					mstatus = units_[mstatus].fail;
				}
			} else {
				if (status == 0) {
					i++;
				} else {
					status = units_[status].fail;
				}
			}
		}
	}
};

}

}

