//  Copyright John Maddock 2024.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/config.hpp>

#include <quadmath.h>

int main()
{
   __float128 f = 0;
   f = sqrtq(f);

   return 0;
}