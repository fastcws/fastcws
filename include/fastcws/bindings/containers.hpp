// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#ifndef FASTCWS_NO_BOOST
#if __has_include("boost/version.hpp")
#define FASTCWS_HAS_BOOST
#endif
#endif

#include <queue>

#ifdef FASTCWS_HAS_BOOST

#include "boost/container/list.hpp"
#include "boost/container/vector.hpp"
#include "boost/container/map.hpp"
#include "boost/unordered_map.hpp"
#include "boost/container/set.hpp"
#include "boost/container/string.hpp"
#include "boost/container/deque.hpp"

#else

#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <string>
#include <deque>

#endif

namespace fastcws {

#ifdef FASTCWS_HAS_BOOST

using boost::container::list;
using boost::container::vector;
using boost::container::map;
using boost::unordered_map;
using boost::container::set;
using boost::container::basic_string;
using boost::container::deque;

#else

using std::list;
using std::vector;
using std::map;
using std::unordered_map;
using std::set;
using std::basic_string;
using std::deque;

#endif

template <class T>
using queue = std::queue<T, deque<T>>;

}

