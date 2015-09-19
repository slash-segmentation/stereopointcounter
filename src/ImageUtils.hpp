/* 
 * File:   ImageUtils.hpp
 * Author: Christopher Churas <churas@ncmir.ucsd.edu>
 *
 * Created on September 17, 2015, 3:07 PM
 */

#ifndef IMAGEUTILS_HPP
#define	IMAGEUTILS_HPP

#include <sys/types.h>
#include <dirent.h>

#include "itkImage.h"
#include "itkIndex.h"
#include "itkCastImageFilter.h"
#include "itkImageDuplicator.h"


namespace spc {

    const int Dimension = 2;
    typedef itk::RGBPixel< unsigned char > RGBPixelType;
    typedef itk::Image< RGBPixelType, Dimension > RGBImageType;
    
    
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

    ///////////////////////////////////////////////////////////////////////////
    // Read an image via its address
    ///////////////////////////////////////////////////////////////////////////

    template < typename TImageType >
    typename TImageType::Pointer readImage(const std::string& address) {

        typedef itk::ImageFileReader< TImageType > ReaderType;
        typename ReaderType::Pointer reader = ReaderType::New();

        reader->SetFileName(address);
        reader->Update();

        return reader->GetOutput();
    }

    ///////////////////////////////////////////////////////////////////////////
    // Write an image to the given address
    ///////////////////////////////////////////////////////////////////////////

    template < typename TImageType >
    void writeImage(typename TImageType::Pointer const& image,
            std::string const& address) {

        typedef itk::ImageFileWriter< TImageType > WriterType;
        typename WriterType::Pointer writer = WriterType::New();

        writer->SetFileName(address);
        writer->SetInput(image);
        writer->Update();

    }

    template < typename TImageType> 
    typename TImageType::Pointer 
    duplicateImage(typename TImageType::Pointer const &image){
        typedef itk::ImageDuplicator< TImageType > DuplicatorType;
        typename DuplicatorType::Pointer duplicator = DuplicatorType::New();
        duplicator->SetInputImage(image);
        duplicator->Update();
        return duplicator->GetOutput();
    }
    
    template < typename TImageType>
    RGBImageType::Pointer
    castImageToRGBImage(typename TImageType::Pointer &image){
        typedef itk::CastImageFilter< TImageType, RGBImageType > CastFilterType;
        typename CastFilterType::Pointer castFilter = CastFilterType::New();
        castFilter->SetInput(image);
        castFilter->Update();
        return castFilter->GetOutput();
    }
    
    RGBImageType::Pointer
    drawRedGridOnImage(RGBImageType::Pointer &image, 
            int gridWidth,
            int gridHeight) {
        
        RGBImageType::RegionType region;
        RGBImageType::IndexType pixelLoc;

        RGBPixelType redPixel;
        redPixel.SetRed(255);
        redPixel.SetBlue(0);
        redPixel.SetGreen(0);
        
        region = image->GetLargestPossibleRegion();
        
        RGBImageType::SizeType size = region.GetSize();
        int imageWidth = size[0];
        int imageHeight =size[1];

        for (int x = gridWidth; x < imageWidth; x += gridWidth) {
            for (int y = 0; y < imageHeight; y++) {
                if (y % gridHeight != 0 &&
                    (y-1) % gridHeight != 0 &&
                    (y+1) % gridHeight != 0){
                    pixelLoc[0] = x;
                    pixelLoc[1] = y;
                    image->SetPixel(pixelLoc, redPixel);
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
                    image->SetPixel(pixelLoc, redPixel);
                }
            }
        }
        return image;
    }
    
    RGBImageType::Pointer
    drawGreenCirclesAroundPointsOnImage(RGBImageType::Pointer &image,
            std::vector< std::pair<int,int> > locations){
        std::vector< std::pair<int,int> >::iterator itr;
        RGBImageType::IndexType pixelLoc;
        RGBPixelType greenPixel;
        greenPixel.SetRed(0);
        greenPixel.SetBlue(0);
        greenPixel.SetGreen(255);
        
        for (itr = locations.begin(); itr != locations.end();itr++){
            pixelLoc[0] = itr->first;
            pixelLoc[1] = itr->second;
            image->SetPixel(pixelLoc,greenPixel);
        }
        return image;
    }

}



#endif	/* IMAGEUTILS_HPP */

