/* 
 * File:   StereoPointCount.cpp
 * Author: Christopher Churas <churas@ncmir.ucsd.edu>
 * 
 * Created on September 17, 2015, 3:37 PM
 */

#include "StereoPointCount.hpp"

using namespace spc;

StereoPointCount::StereoPointCount() {
}

StereoPointCount::StereoPointCount(const StereoPointCount& orig) {
}

StereoPointCount::~StereoPointCount() {
}

int StereoPointCount::getNumberPositiveIntersections(){
    return _num_positive;
}

void StereoPointCount::setNumberPositiveIntersections(int val) {
    _num_positive = val;
}

int StereoPointCount::getTotalIntersections() {
    return _num_total;
}

void StereoPointCount::setTotalIntersections(int val) {
    _num_total = val;
}

