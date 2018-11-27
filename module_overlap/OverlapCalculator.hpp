//
//  OverlapCalculator.hpp
//  module_overlap
//
//  Created by Denis Musatov on 05/07/2017.
//  Copyright © 2017 Frangou Lab. All rights reserved.
//

#ifndef OverlapCalculator_hpp
#define OverlapCalculator_hpp

#include <string>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <cassert>

//
// Returns a vector that is an intersection of two containers.
//
template <typename Container>
static inline
std::vector<std::string> GetDifference(const Container& l1, const Container& l2)
{
    std::vector<std::string> intersection;
    std::set_difference(l1.cbegin(), l1.cend(), l2.cbegin(), l2.cend(),
                        std::back_inserter(intersection));
    return intersection;
}

//
// Returns a vector that is an intersection of two containers.
//
template <typename Container>
static inline
std::vector<std::string> GetIntersection(const Container& l1,
                                         const Container& l2)
{
    std::vector<std::string> intersection;
    std::set_intersection(l1.cbegin(), l1.cend(), l2.cbegin(), l2.cend(),
                          std::back_inserter(intersection));
    return intersection;
}

//
// Calculates the percentage of the size of intersection using this formula:
// % = |A n B|/(|A| + |B| - |A n B|)*100
//   (this is Jaccard's coefficient of similarity)
static inline
double GetIntersectionPercentage(int64_t set1_size,
                                 int64_t set2_size,
                                 int64_t intersection_size)
{
    intersection_size = std::max(0LL, intersection_size);
    return 100*static_cast<double>(intersection_size)/
               (set1_size + set1_size - intersection_size);
}

#endif /* OverlapCalculator_hpp */
