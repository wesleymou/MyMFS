//
// Created by amigdalite on 24/11/2019.
//

#ifndef MYMFS_LIB_UTILS_H_
#define MYMFS_LIB_UTILS_H_

#include <string>
#include <vector>

#include <iterator>
#include <fstream>

#include <algorithm>

void find_all_occurances(std::vector<size_t> &vec, std::string data, std::string toSearch);

template<typename InputIterator1, typename InputIterator2>
bool range_equal(InputIterator1 first1, InputIterator1 last1,
                 InputIterator2 first2, InputIterator2 last2);

bool compare_files(const std::string &filename1, const std::string &filename2);

#endif //MYMFS_LIB_UTILS_H_
