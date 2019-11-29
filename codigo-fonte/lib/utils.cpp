//
// Created by amigdalite on 24/11/2019.
//

#include "utils.h"

#include <string>
#include <vector>

#include <algorithm>
#include <iterator>
#include <fstream>

void find_all_occurances(std::vector<size_t> &vec, std::string data, std::string toSearch) {
    // Get the first occurrence
    size_t pos = data.find(toSearch);

    // Repeat till end is reached
    while (pos != std::string::npos) {
        // Add position to the vector
        vec.push_back(pos);

        // Get the next occurrence from the current position
        pos = data.find(toSearch, pos + toSearch.size());
    }
}

template<typename InputIterator1, typename InputIterator2>
bool range_equal(InputIterator1 first1, InputIterator1 last1,
                 InputIterator2 first2, InputIterator2 last2) {
    while (first1 != last1 && first2 != last2) {
        if (*first1 != *first2) return false;
        ++first1;
        ++first2;
    }
    return (first1 == last1) && (first2 == last2);
}

bool compare_files(const std::string &filename1, const std::string &filename2) {
    std::ifstream file1(filename1);
    std::ifstream file2(filename2);

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin2(file2);

    std::istreambuf_iterator<char> end;

    return range_equal(begin1, end, begin2, end);
}