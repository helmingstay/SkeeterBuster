//    Copyright 2015-2020 Christian Gunning (code@x14n.org)
//
//    This file is part of Skeeter Buster.
#include "Link.h"

using std::string;
using std::pair;
using std::map;
using std::multimap;

// complex / nested typedefs
// used by csv reader
// house id, link, map
using hid_t = size_t;
using hrow_t = pair<hid_t, double>;
using hrows_t = multimap<hid_t, hrow_t>;

// neighbor member
using hlink_t = Link<hid_t>;

// same for block - csv reader
using bid_t = string;

// csv reader - block
using brow_t = pair<bid_t, double>;
using brows_t = multimap<bid_t, brow_t>;
