// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <string_view>
#include <memory>
#include <utility>
#include <algorithm>
#include <iterator>

#include "fastcws/bindings/containers.hpp"
#include "fastcws/rep_aware/string_view.hpp"

namespace fastcws {

namespace aho_corasick {

template <class Allocator = std::allocator<int>>
struct trie_node {
	using allocator_traits = std::allocator_traits<Allocator>;
	using string_view_type = rep_aware::basic_string_view<char, std::char_traits<char>,
		  typename allocator_traits::template rebind_alloc<char>>;

	size_t id;
	char ch;
	size_t parent;
	size_t fail;
	map<char, size_t,
		std::less<char>,
		typename allocator_traits::template rebind_alloc<
			std::pair<const char, size_t>>> children;
	string_view_type result;
};

template <class Allocator = std::allocator<int>>
struct trie {
	using allocator_traits = std::allocator_traits<Allocator>;
	using trie_node_type = trie_node<Allocator>;

	vector<trie_node_type,
		typename allocator_traits::template rebind_alloc<
			trie_node_type>> nodes_;
	size_t next_id_ = 1;

	trie() {
		trie_node_type node;
		node.id = 0;
		node.ch = '\0';
		node.parent = 0;
		node.fail = 0;
		nodes_.emplace_back(std::move(node));
	}

	trie_node_type& _add_node(trie_node_type& parent, const char ch) {
		trie_node_type node;
		node.id = next_id_++;
		parent.children[ch] = node.id;
		node.ch = ch;
		node.parent = parent.id;
		node.fail = 0;
		nodes_.emplace_back(std::move(node));
		return nodes_.back();
	}

	void add(const std::string_view sv) {
		if (sv.size() == 0) {
			return;
		}
		trie_node_type* node = &nodes_[0];
		for (size_t i = 0; i < sv.size(); i++) {
			if (node->children.count(sv[i]) == 0) {
				node = &_add_node(*node, sv[i]);
			} else {
				node = &nodes_[node->children[sv[i]]];
			}
		}
		node->result = {sv.data(), sv.size()};
	}

	void _finalize_fail() {
		queue<size_t> q;
		q.push(0);
		while (q.size()) {
			trie_node_type& node = nodes_[q.front()];
			q.pop();
			for (auto [ch, child_id] : node.children) {
				trie_node_type& child = nodes_[child_id];
				trie_node_type* curr = &node;
				for (;;) {
					if (curr->id == 0) {
						child.fail = 0;
						break;
					}
					if (nodes_[curr->fail].children.count(ch)) {
						child.fail = nodes_[curr->fail].children[ch];
						break;
					}
					curr = &nodes_[curr->fail];
				}
				q.push(child.id);
			}
		}
	}

	void finalize() {
		_finalize_fail();
	}

	struct scan_state_t {
		const trie_node_type* node;

		scan_state_t(const trie_node_type* node)
			: node(node) {}
	};

	scan_state_t initial_scan_state() const noexcept {
		return scan_state_t{&nodes_[0]};
	}

	template <class MatchCallback>
	void scan(const std::string_view haystack, MatchCallback matched, scan_state_t *state = nullptr) const {
		scan_state_t implicit_state = initial_scan_state();
		if (state == nullptr) {
			state = &implicit_state;
		}

		for (size_t i = 0; i < haystack.size();) {
			if (state->node->children.count(haystack[i])) {
				state->node = &nodes_[state->node->children.at(haystack[i])];
				i++;
				const auto* mnode = state->node;
				while (mnode->id != 0) {
					if (mnode->result.size() != 0) {
						matched(i, std::string_view{mnode->result.data(), mnode->result.size()});
					}
					mnode = &nodes_[mnode->fail];
				}
			} else {
				if (state->node->id == 0) {
					i++;
				} else {
					state->node = &nodes_[state->node->fail];
				}
			}
		}
	}

/**
	template <class MatchCallback>
	void scan(const std::string_view haystack, MatchCallback matched) const {
		const auto* node = &nodes_[0];
		for (size_t i = 0; i < haystack.size();) {
			if (node->children.count(haystack[i])) {
				node = &nodes_[node->children.at(haystack[i])];
				i++;
				const auto* mnode = node;
				while (mnode->id != 0) {
					if (mnode->result.size() != 0) {
						matched(i - mnode->result.size(), mnode->result.size());
					}
					mnode = &nodes_[mnode->fail];
				}
			} else {
				if (node->id == 0) {
					i++;
				} else {
					node = &nodes_[node->fail];
				}
			}
		}
	}
**/
};

}

}

