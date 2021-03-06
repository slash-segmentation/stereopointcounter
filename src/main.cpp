
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <utility>

#include <iostream>
#include <string>
#include <sstream>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTimeProbe.h"

#include "optionparser.h"
#include "ImageUtils.hpp"


struct Arg : public option::Arg {

    /**
     * Checks that arg in option is not NULL or 0
     * @param option option parameter to examine
     * @param msg if true then error messages will be output to standard error
     * @return option::ARG_OK upon success or option::ARG_ILLEGAL upon failure
     */
    static option::ArgStatus Required(const option::Option& option, bool msg) {

        if (option.arg != 0 || option.arg != NULL) {
            return option::ARG_OK;
        }

        if (msg) std::cerr << "Option '" << option.name << "' requires an argument" << std::endl;
        return option::ARG_ILLEGAL;
    }

    /**
     * Checks that arg in option is not NULL or 0 and that its a directory
     * @param option parameter to examine
     * @param msg if true then error messages will be output to standard error
     * @return option::ARG_OK if option.arg exists and is a directory otherwise option::ARG_ILLEGAL
     */
    static option::ArgStatus RequiredDir(const option::Option& option, bool msg) {
        if (option.arg == 0 || option.arg == NULL) {
            return option::ARG_ILLEGAL;
        }

        struct stat st;
        if (stat(option.arg, &st) != 0) {
            if (msg) std::cerr << "Error running stat on directory set under option '" << option.name << "'" << std::endl;
            return option::ARG_ILLEGAL;
        }
        if (!S_ISDIR(st.st_mode)) {
            if (msg) std::cerr << "Option '" << option.name << "' must be a directory" << std::endl;
            return option::ARG_ILLEGAL;
        }
        return option::ARG_OK;
    }

};

std::string usageStr = "usage: stereopointcounter [options]\n\n"
        "Performs automated stereology point counting on "
        "probability map images passed in via --images path. "
        "\n\nThis tool looks for *.png files and assumes they "
        "are 8-bit greyscale images all with the same "
        "size.\n\n"
        "Output is to standard out and format is comma separated variables "
        "in the following format:\n\n"
        "\tImage,GridSize,GridSizePixel,Positive,Total\n"
        "\t/../foo.png,12x8,120x80,10,,67\n"
        "\t...\n"
        "\t...\n"
        "\tSeconds,GrandTotalPositive,GrandTotal\n"
        "\t123,29342,234292\n\n";
        

std::string usageWithOpts = usageStr + "Options:";

/**
 * Used by optionparser to keep track of command line arguments
 */
enum optionIndex {
    UNKNOWN, HELP, VERSION, IMAGES, GRIDX, GRIDY, THRESHOLD, SAVEIMAGES
};

/**
 * Defines command line options
 */
const option::Descriptor usage[] = {
    {UNKNOWN, 0, "", "", option::Arg::None, usageWithOpts.c_str()},
    {HELP, 0, "h", "help", option::Arg::None, "  --help, -h  \tPrint usage and exit."},
    {VERSION, 0, "v", "version", option::Arg::None,
        "  --version, -v  \tPrint version and exit."},
    {IMAGES, 0, "i", "images", Arg::Required,
        "  --images, -m  \tCan be set to a single greyscale 8-bit image or directory of "
        " 8-bit greyscale *.png images"},
    {GRIDX, 0, "", "gridx", Arg::Required,
        "  --gridx,  \tGrid size in X.  A value of say 4 means to generate 4"
        "vertical lines evenly spaced across the image."},
    {GRIDY, 0, "", "gridy", Arg::Required,
        "  --gridy,  \tGrid size in Y.  A value of say 8 means to generate 8"
        "horizontal lines evenly spaced across the image."},
    {THRESHOLD, 0, "t", "threshold", Arg::Required,
        "  --threshold, -t  \tThreshold to for pixel intensity that denotes"
        " a given pixel intersection is a positive hit (0 - 255)"},
    {SAVEIMAGES, 0, "s", "saveimages", Arg::RequiredDir,
        "  --saveimages, -s  \tIf set to <dir>, writes out images as RGB with grid "
        "overlayed in red and green circles denoting intersections with matches"
        " to a file with format of "
        "grid(--gridx)x(--gridy)_pixel(pixelw)x(pixelh)_thresh(-t).(origname)"},
    {0, 0, 0, 0, 0, 0}
};

int main(int argc, char *argv[]) {
    argc -= (argc > 0);
    argv += (argc > 0); // skip program name argv[0] if present

    if (argc < 2) {
        std::cerr << "Invalid arguments" << std::endl << std::endl;
        option::Stats stats(usage, argc, argv);
        option::Option options[stats.options_max], buffer[stats.buffer_max];
        option::Parser parse(usage, argc, argv, options, buffer);
        option::printUsage(std::cout, usage);
        return 1;
    }

    option::Stats stats(usage, argc, argv);
    option::Option options[stats.options_max], buffer[stats.buffer_max];
    option::Parser parse(usage, argc, argv, options, buffer);

    if (parse.error()) {
        std::cerr << "Error parsing arguments" << std::endl << std::endl;
        return 1;
    }

    if (options[UNKNOWN].count() > 0) {
        std::cerr << "Unknown option(s) found " << std::endl;
        for (option::Option* opt = options[UNKNOWN]; opt; opt = opt->next())
            std::cerr << "\t" << opt->name << std::endl;
        return 2;
    }

    if (options[HELP]) {
        option::printUsage(std::cout, usage);
        return 0;
    }

    int nonOptCount = parse.nonOptionsCount();
    if (nonOptCount > 0) {
        std::cerr << "Unexpected options found: " << std::endl;
        for (int i = 0; i < nonOptCount; ++i) {
            std::cerr << "#" << i << ": " << parse.nonOption(i) << "\n";
        }
        return 3;
    }

    if (options[GRIDX].arg == NULL) {
        std::cerr << "--gridx required.  Run with --help for more information"
                << std::endl;
        return 4;
    }
    if (options[GRIDY].arg == NULL) {
        std::cerr << "--gridy required.  Run with --help for more information"
                << std::endl;
        return 5;

    }
    if (options[IMAGES].arg == NULL) {
        std::cerr << "--images required.  Run with --help for more information"
                << std::endl;
        return 6;
    }
    if (options[THRESHOLD].arg == NULL) {
        std::cerr << "--threshold required.  Run with --help for more information"
                << std::endl;
        return 7;
    }
    std::string save_images_dir = "";
    if (options[SAVEIMAGES].arg != NULL){
        save_images_dir = std::string(options[SAVEIMAGES].arg);
    }
    itk::TimeProbe clock;
    clock.Start();

    int gridX = std::strtol(options[GRIDX].arg, (char **) NULL, 10);
    int gridY = std::strtol(options[GRIDY].arg, (char **) NULL, 10);
    int threshold = std::strtol(options[THRESHOLD].arg, (char **) NULL, 10);

    std::vector<std::string> images = spc::getImages(std::string(options[IMAGES].arg));
    
    int totalPCount = 0;
    int totalNCount = 0;
    std::string curImage;
    typedef unsigned char PixelType;
    typedef itk::Image<PixelType, 2> ImageType;
    ImageType::Pointer image;
    spc::RGBPixelType greenPixel;
    greenPixel.SetRed(0);
    greenPixel.SetBlue(0);
    greenPixel.SetGreen(255);
    spc::RGBPixelType redPixel;
    redPixel.SetRed(255);
    redPixel.SetBlue(0);
    redPixel.SetGreen(0);
    int image_total = 0;
    int imagePCount = 0;
    int imageNCount = 0;
    int grid_width = 0;
    int grid_height = 0;
    
    std::vector< std::pair<int,int> > positivePixels;
    std::cout << "Image,GridSize,GridSizePixel,Positive,Total" << std::endl;
    for (std::vector<std::string>::iterator it = images.begin(); it != images.end(); ++it) {
        
        curImage = *it;
        image = spc::readImage<ImageType>(curImage);
        
        positivePixels = spc::getIntersectionPixelsAboveThreshold<PixelType>(image,gridX,
                gridY,threshold,image_total,grid_width,grid_height);
    
        imagePCount = positivePixels.size();
        imageNCount = image_total - imagePCount;
        
        std::cout <<*it<<","<<gridX<<"x"<<gridY<<","<<grid_width<<"x"
                <<grid_height<<","<<imagePCount<<","
                  <<image_total<<std::endl;
        totalPCount += imagePCount;
        totalNCount += imageNCount;
        if (save_images_dir.length() > 0){
            std::ostringstream os;
            
            ImageType::Pointer imageCopy = spc::duplicateImage<ImageType>(image);
            spc::RGBImageType::Pointer rgbimage = spc::castImageToRGBImage
                    <ImageType>(imageCopy);
            rgbimage = spc::drawGridOnImage<spc::RGBPixelType>(rgbimage,
                    redPixel,grid_width,grid_height);
            rgbimage = spc::drawCirclesAroundPointsOnImage
                    <spc::RGBPixelType>(rgbimage,greenPixel,positivePixels,5);
            
            os << save_images_dir << "/grid" << gridX << "x" << gridY << "_pixel"
                    << grid_width <<"x"<< grid_height
                    << "_thresh" << threshold << "." 
                    << spc::getFileNameFromPath(curImage);
            
            spc::writeImage<spc::RGBImageType>(rgbimage,os.str());
        }
        positivePixels.clear();
    }
    clock.Stop();    
    std::cout <<std::endl<<"Seconds,GrandTotalPositive,GrandTotal"<<std::endl;
    std::cout << clock.GetTotal() << ","<< totalPCount << "," 
            << (totalPCount + totalNCount)<< std::endl;
    
    return EXIT_SUCCESS;
}
