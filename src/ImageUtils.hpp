/* 
 * File:   ImageUtils.hpp
 * Author: Christopher Churas <churas@ncmir.ucsd.edu>
 *
 * Created on September 17, 2015, 3:07 PM
 */

#ifndef IMAGEUTILS_HPP
#define	IMAGEUTILS_HPP

#include <math.h>
#include <sys/types.h>
#include <dirent.h>

#include "itkImage.h"
#include "itkIndex.h"
#include "itkCastImageFilter.h"
#include "itkImageDuplicator.h"


namespace spc {

    const double PI = 3.14159;
    const int DIMENSION = 2;
    typedef itk::RGBPixel< unsigned char > RGBPixelType;
    typedef itk::Image< RGBPixelType, DIMENSION > RGBImageType;
    
    typedef itk::RGBAPixel< unsigned char > RGBAPixelType;
    typedef itk::Image< RGBAPixelType, DIMENSION > RGBAImageType;
    
    
        /**
     * Checks if path is a directory
     * @param path
     * @return true if yes, false otherwise.
     */
    bool is_dir(const char *path) {
        struct stat st;
        if (stat(path, &st) != 0) {
            return false;
        }
        return S_ISDIR(st.st_mode);
    }

    /**
     * Checks if path is a file
     * @param path
     * @return true if yes, false otherwise
     */
    bool is_file(const char *path) {
        struct stat st;
        if (stat(path, &st) != 0) {
            return false;
        }
        return S_ISREG(st.st_mode);
    }

    /**
     * Return list of png files in directory passed in
     * @param directory
     * @return 
     */
    std::vector<std::string> getImageFileNamesInDir(const std::string& directory) {

        std::string pngSuffix = ".png";
        std::size_t suffixLen = pngSuffix.length();
        std::vector<std::string> fileNames;
        struct dirent *dirEnt;
        DIR *dir = opendir(directory.c_str());
        if (dir == NULL) {
            return fileNames;
        }
        while ((dirEnt = readdir(dir)) != NULL) {
            std::string entry = dirEnt->d_name;
            if (entry.length() < suffixLen) {
                continue;
            }
            std::string ending = entry.substr(entry.length() - suffixLen, suffixLen);
            if (ending.compare(pngSuffix) == 0) {
                fileNames.push_back(directory + "/" + entry);
            }
        }
        return fileNames;
    }

    /**
     * Returns a list of images from path specified by <b>arg</b> parameter
     * @param arg either a directory or a path to a single image
     * @return If <b>arg</b> is a directory then a list of full path png files in 
     *         the directory will be returned. Else just the value of <b>arg</b>
     *         will be returned.
     */
    std::vector<std::string> getImages(const std::string& arg) {
        if (is_dir(arg.c_str())) {
            return getImageFileNamesInDir(arg);
        }
        std::vector<std::string> imageFile;
        imageFile.push_back(arg);
        return imageFile;
    }

    /**
     * Given full path to a file strip path to left of last forward slash
     * @param path
     * @return path to file
     */
    std::string getFileNameFromPath(const std::string& path){
        std::size_t last_slash = path.find_last_of("/");
        if (last_slash == std::string::npos){
            return path;
        }
        if (last_slash == path.npos){
            return "";
        }
        return path.substr(last_slash+1);
    }
    /**
     * Reads image from file path
     * @param path full path to image file to read
     * @return TImageType::Pointer pointing to image
     */
    template < typename TImageType >
    typename TImageType::Pointer readImage(const std::string& path) {

        typedef itk::ImageFileReader< TImageType > ReaderType;
        typename ReaderType::Pointer reader = ReaderType::New();

        reader->SetFileName(path);
        reader->Update();

        return reader->GetOutput();
    }

    /**
     * write image to file
     * @param image the image to write to a file
     * @param path output file, should be full path and extension denotes
     *             image format type.  
     */
    template < typename TImageType >
    void writeImage(typename TImageType::Pointer const& image,
            std::string const& path) {

        typedef itk::ImageFileWriter< TImageType > WriterType;
        typename WriterType::Pointer writer = WriterType::New();

        writer->SetFileName(path);
        writer->SetInput(image);
        writer->Update();

    }

    /**
     * Duplicates image
     * @param image
     * @return makes a copy of image
     */
    template < typename TImageType> 
    typename TImageType::Pointer 
    duplicateImage(typename TImageType::Pointer const &image){
        typedef itk::ImageDuplicator< TImageType > DuplicatorType;
        typename DuplicatorType::Pointer duplicator = DuplicatorType::New();
        duplicator->SetInputImage(image);
        duplicator->Update();
        return duplicator->GetOutput();
    }
    
    /**
     * Convert image to RGBImageType
     * @param image
     * @return converted image
     */
    template < typename TImageType>
    RGBImageType::Pointer
    castImageToRGBImage(typename TImageType::Pointer &image){
        typedef itk::CastImageFilter< TImageType, RGBImageType > CastFilterType;
        typename CastFilterType::Pointer castFilter = CastFilterType::New();
        castFilter->SetInput(image);
        castFilter->Update();
        return castFilter->GetOutput();
    }
    
    /**
     * Convert image to RGBAImageType
     * @param image
     * @return converted image
     */
    template < typename TImageType>
    RGBAImageType::Pointer
    castImageToRGBAImage(typename TImageType::Pointer &image){
        typedef itk::CastImageFilter< TImageType, RGBAImageType > CastFilterType;
        typename CastFilterType::Pointer castFilter = CastFilterType::New();
        castFilter->SetInput(image);
        castFilter->Update();
        return castFilter->GetOutput();
    }
    
    /**
     * Draws a grid on image using pixel passed in.  Note this implementation
     * omits the pixels at the intersections and +-1 pixel around those
     * intersections
     * @param image image to draw on
     * @param pixel pixel to do drawing with
     * @param gridWidth desired spacing in pixels between vertical gridlines
     * @param gridHeight desired spacing in pixels between horizontal gridlines
     * @return 
     */
    template<typename TPixelType>
    typename itk::Image<TPixelType,spc::DIMENSION>::Pointer 
    drawGridOnImage(
    typename itk::Image<TPixelType,spc::DIMENSION>::Pointer &image, 
            TPixelType pixel,
            int gridWidth,
            int gridHeight) {
        
        typedef itk::Image<TPixelType,spc::DIMENSION> ImageType;
        typename ImageType::IndexType pixelLoc;
        typename ImageType::RegionType region;
        
        region = image->GetLargestPossibleRegion();
        
        typename ImageType::SizeType size = region.GetSize();
        int imageWidth = size[0];
        int imageHeight = size[1];

        for (int x = gridWidth; x < imageWidth; x += gridWidth) {
            for (int y = 0; y < imageHeight; y++) {
                if (y % gridHeight != 0 &&
                    (y-1) % gridHeight != 0 &&
                    (y+1) % gridHeight != 0){
                    pixelLoc[0] = x;
                    pixelLoc[1] = y;
                    image->SetPixel(pixelLoc, pixel);
                }
            }
        }
        for (int y = gridHeight; y < imageHeight; y += gridHeight) {
            for (int x = 0; x < imageWidth; x++) {
                if (x % gridWidth != 0 &&
                    (x-1) % gridWidth != 0 &&
                    (x+1) % gridWidth != 0){
                    pixelLoc[0] = x;
                    pixelLoc[1] = y;
                    image->SetPixel(pixelLoc, pixel);
                }
            }
        }
        return image;
    }

    /**
     * Draws a single circle on image passed in
     * @param image image to draw on
     * @param pixel pixel to draw with
     * @param x center of circle x coordinate
     * @param y center of circle y coordinate
     * @param radius desired radius in pixels of circle
     * @return image with circle drawn.
     */
    template<typename TPixelType>
    typename itk::Image<TPixelType,spc::DIMENSION>::Pointer 
    drawCircle(typename itk::Image<TPixelType,spc::DIMENSION>::Pointer &image,
            TPixelType pixel,
            int x,int y, double radius){
        
        double pi_double = 2*spc::PI;
        typedef itk::Image<TPixelType,spc::DIMENSION> ImageType;
        typename ImageType::IndexType pixelLoc;
        for (double angle = 0; angle < pi_double; angle+=0.1 ){
            pixelLoc[0] = x + floor(radius * cos(angle));
            pixelLoc[1] = y + floor(radius * sin(angle));
            image->SetPixel(pixelLoc,pixel);
        }
        return image;
    }
    
    /**
     * Draws a cirle around every location specified in the locations parameter
     * using the pixel passed in
     * @param image image to draw circles on 
     * @param pixel pixel to draw with
     * @param locations  std::vector of std::pair<int,int> denoting each location
     *                   to draw the circle.  The first value in pair is X and
     *                   second is Y coordinate.
     * @param circle_radius Radius of circle, 5 means 5 pixels.
     * @return input image with circles draw on it.
     */
    template<typename TPixelType>
    typename itk::Image<TPixelType,spc::DIMENSION>::Pointer 
    drawCirclesAroundPointsOnImage(
            typename itk::Image<TPixelType,spc::DIMENSION>::Pointer &image,
            TPixelType pixel,
            std::vector< std::pair<int,int> > locations,double circle_radius){
        
        std::vector< std::pair<int,int> >::iterator itr;
        typedef itk::Image<TPixelType,spc::DIMENSION> ImageType;
        typename ImageType::IndexType pixelLoc;
        
        for (itr = locations.begin(); itr != locations.end();itr++){
            pixelLoc[0] = itr->first;
            pixelLoc[1] = itr->second;
            image->SetPixel(pixelLoc,pixel);
            spc::drawCircle<TPixelType>(image,pixel,pixelLoc[0],
                    pixelLoc[1],circle_radius);
        }
        return image;
    }

    /**
     * First generates a grid across image with gridx vertical grid lines and
     * gridy horizontal grid lines.  Function then examines intersections
     * and every intersection whose pixel value is above threshold is added
     * to std::vector<std::pair<int,int>> returned by this method.  In addition,
     * total_pixels, grid_width,grid_height are set by this method.
     * 
     * @param image the image being examined.  Should be a 8-bit greyscale image
     * @param gridx number of vertical grid lines to break image into
     * @param gridy number of horizontal grid lines to break image into
     * @param threshold 
     * @param total_pixels set by function to # pixels examined
     * @param grid_width set by function to vertical grid spacing in pixels
     * @param grid_height set by function to horizontal grid spacing in pixels
     * @return vector if pairs denoting locations where intersections >= threshold
     */
    template<typename TPixelType>
    std::vector< std::pair<int,int> >
    getIntersectionPixelsAboveThreshold
    (typename itk::Image<TPixelType,spc::DIMENSION>::Pointer const &image,
            int gridx,int gridy,int threshold,int &total_pixels,int &grid_width,
    int &grid_height){
        
        std::vector< std::pair<int,int> > positivePixels;
        
        typedef itk::Image<TPixelType,spc::DIMENSION> ImageType;
        typename ImageType::IndexType pixelLoc;
        typename ImageType::RegionType region;
        
        typename ImageType::PixelType pixel;
        
        region = image->GetLargestPossibleRegion();
        
        typename ImageType::SizeType size = region.GetSize();
        
        int image_width = size[0];
        int image_height = size[1];
        
        grid_width = floor((float) image_width / (float) gridx);
        grid_height = floor((float) image_height / (float) gridy);
        total_pixels = 0;
        for (int x = grid_width; x < image_width; x += grid_width) {
            for (int y = grid_height; y < image_height; y += grid_height) {
                pixelLoc[0] = x;
                pixelLoc[1] = y;
                
                pixel = image->GetPixel(pixelLoc);
                if (pixel >= threshold) {
                    positivePixels.push_back(std::make_pair(x,y));
                }
                total_pixels++;
            }
        }
        return positivePixels;
    }
    
}

#endif	/* IMAGEUTILS_HPP */

