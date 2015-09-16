
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTimeProbe.h"

#include "optionparser.h"

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

std::string usageStr = "usage: stereopointcounter [options]\n\n"
        "Performs automated Stereology point counting on "
        " probability images passed in via --images path\n\n";

std::string usageWithOpts = usageStr + "Options:";

/**
 * Used by optionparser to keep track of command line arguments
 */
enum optionIndex {
    UNKNOWN, HELP, VERSION, IMAGES, GRIDX, GRIDY, THRESHOLD
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
        "  --images, -m  \tSingle greyscale image or directory of *.png images"},
    {GRIDX, 0, "", "gridx", Arg::Required,
        "  --gridx,  \tGrid size in X.  A value of say 4 means to generate 4"
        "vertical lines evenly spaced across the image."},
    {GRIDY, 0, "", "gridy", Arg::Required,
        "  --gridy,  \tGrid size in Y.  A value of say 8 means to generate 8"
        "horizontal lines evenly spaced across the image."},
    {THRESHOLD, 0, "t", "threshold", Arg::Required,
        "  --threshold, -t  \tThreshold to for pixel intensity that denotes"
        " a given pixel intersection is a positive hit (0 - 255)"},
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
    itk::TimeProbe clock;
    clock.Start();

    int gridX = std::strtol(options[GRIDX].arg, (char **) NULL, 10);
    int gridY = std::strtol(options[GRIDY].arg, (char **) NULL, 10);
    int threshold = std::strtol(options[THRESHOLD].arg, (char **) NULL, 10);

    std::vector<std::string> images = getImages(std::string(options[IMAGES].arg));

    typedef unsigned char PixelType;
    typedef itk::Image<PixelType, 2> ImageType;
    ImageType::IndexType pixelLoc;
    ImageType::Pointer image;
    ImageType::RegionType region;
    typedef itk::ImageFileReader<ImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    int gridWidth = -1;
    int gridHeight = -1;
    int imageWidth = -1;
    int imageHeight = -1;
    int imagePCount = 0;
    int imageNCount = 0;
    int totalPCount = 0;
    int totalNCount = 0;
    short pixel;
    std::string curImage;
    std::cout << "Image,GridSize,GridSizePixel,#Points,Positive,Negative" << std::endl;
    for (std::vector<std::string>::iterator it = images.begin(); it != images.end(); ++it) {
        curImage = *it;
        reader->SetFileName(curImage.c_str());
        reader->Update();
        image = reader->GetOutput();
        region = image->GetLargestPossibleRegion();

        ImageType::SizeType size = region.GetSize();
        imageWidth = size[0];
        imageHeight = size[1];
        gridWidth = floor((float) imageWidth / (float) gridX);
        gridHeight = floor((float) imageHeight / (float) gridY);

        imagePCount = 0;
        imageNCount = 0;
        gridWidth = floor((float) imageWidth / (float) gridX);
        gridHeight = floor((float) imageHeight / (float) gridY);
        for (int x = gridWidth; x < imageWidth; x += gridWidth) {
            for (int y = gridHeight; y < imageHeight; y += gridHeight) {
                pixelLoc[0] = x;
                pixelLoc[1] = y;
                pixel = image->GetPixel(pixelLoc);
                if (pixel >= threshold) {
                    imagePCount++;
                } else {
                    imageNCount++;
                }
            }
        }
        std::cout <<*it<<","<<gridX<<"x"<<gridY<<","<<gridWidth<<"x"
                  <<gridHeight<<","<<(imagePCount + imageNCount)<<","
                  <<imagePCount<<","<<imageNCount<<std::endl;
        totalPCount += imagePCount;
        totalNCount += imageNCount;
    }
    clock.Stop();    
    
    std::cout <<std::endl<< clock.GetTotal() << ","<< totalPCount << "/" 
            << (totalPCount + totalNCount) << " = " 
            << ((float) totalPCount / (float) (totalPCount + totalNCount)) 
            << std::endl;
    /*
    for (int x = gridWidth; x < imageWidth; x+= gridWidth){
        for (int y = 0; y < imageHeight; y++){
            pixelLoc[0] = x;
            pixelLoc[1] = y;
            image->SetPixel(pixelLoc,redPixel);
        }
    }
    for (int y = gridHeight; y < imageHeight; y+= gridHeight){
        for (int x = 0; x < imageWidth; x++){
            pixelLoc[0] = x;
            pixelLoc[1] = y;
            image->SetPixel(pixelLoc,redPixel);
        }
    }
  
    typedef  itk::ImageFileWriter< RGBImageType  > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName("ha.png");
    writer->SetInput(image);
    writer->Update();
     */
    return EXIT_SUCCESS;
}
