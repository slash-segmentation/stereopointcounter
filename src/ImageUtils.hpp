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
#include "itkImageDuplicator.h"


namespace spc {

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

    template < typename TPixelType >
    void
    drawGridOnImage(typename itk::Image< TPixelType, 2 >::Pointer &image, 
            std::string const& destImageFile,
            int gridWidth,
            int gridHeight) {
        const int Dimension = 2;
        typedef itk::RGBPixel< unsigned char > RGBPixelType;
        typedef itk::Image< RGBPixelType, Dimension > RGBImageType;
        RGBImageType::IndexType pixelLoc;
        
        typedef itk::ImageDuplicator< RGBImageType > DuplicatorType;
        DuplicatorType::Pointer duplicator = DuplicatorType::New();
        duplicator->SetInputImage(image);
        duplicator->Update();
        RGBImageType::Pointer rgbImage = duplicator->GetOutput();

        RGBPixelType redPixel;
        redPixel.SetRed(255);
        redPixel.SetBlue(0);
        redPixel.SetGreen(0);

        typedef itk::Image< TPixelType, Dimension > ImageType;
	typename ImageType::RegionType region;
        
        region = image->GetLargestPossibleRegion();
        
        typename ImageType::SizeType size = region.GetSize();
        int imageWidth = size[0];
        int imageHeight =size[1];

        for (int x = gridWidth; x < imageWidth; x += gridWidth) {
            for (int y = 0; y < imageHeight; y++) {
                pixelLoc[0] = x;
                pixelLoc[1] = y;
                rgbImage->SetPixel(pixelLoc, redPixel);
            }
        }
        for (int y = gridHeight; y < imageHeight; y += gridHeight) {
            for (int x = 0; x < imageWidth; x++) {
                pixelLoc[0] = x;
                pixelLoc[1] = y;
                rgbImage->SetPixel(pixelLoc, redPixel);
            }
        }
        spc::writeImage<RGBImageType>(rgbImage,destImageFile);
    }

}



#endif	/* IMAGEUTILS_HPP */

