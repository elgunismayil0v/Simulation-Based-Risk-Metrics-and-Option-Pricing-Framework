// Copyright (c) 2016-2024 Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// To compile manually use a command like the folowing:
// clang++ -std=c++20 -fmodule-file=pfr.pcm pfr.pcm usage_sample.cpp

#include <iostream>
#include <iomanip>

import Boost.PFR;

struct some_person {
    std::string name;
    unsigned birth_year;
};

void mu1_act();

int main() {
    mu1_act();

    some_person val{"Joseph Brodsky", 1940};

    std::cout << boost::pfr::get<0>(val)                // No macro!
        << " was born in " << boost::pfr::get<1>(val);  // Works with any aggregate!

    std::cout << '\n' << boost::pfr::io(val);
    std::cout << "\n." << boost::pfr::get_name<0, some_person>()
        << '=' << val.name << '\n';
}