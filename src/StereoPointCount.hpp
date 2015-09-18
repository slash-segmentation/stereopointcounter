/* 
 * File:   StereoPointCount.hpp
 * Author: Christopher Churas <churas@ncmir.ucsd.edu>
 *
 * Created on September 17, 2015, 3:37 PM
 */

#ifndef STEREOPOINTCOUNT_HPP
#define	STEREOPOINTCOUNT_HPP

#include <iostream>

namespace spc {

    class StereoPointCount {
    public:
        StereoPointCount();
        StereoPointCount(const StereoPointCount& orig);
        virtual ~StereoPointCount();
        int getNumberPositiveIntersections();
        
        void setNumberPositiveIntersections(int val);
        int getTotalIntersections();
        void setTotalIntersections(int val);
        
    private:
        int _num_positive;
        int _num_total;
        std::string _image_path;
    };
}
#endif	/* STEREOPOINTCOUNT_HPP */

