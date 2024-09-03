#include <iostream>
#include <vector>
#include "tinyxml2.h"

#define cimg_display 0
#define cimg_use_png 1
#include "CImg.h"

typedef std::pair<int, int> Coordinate;
typedef std::pair<Coordinate, Coordinate> Bounds;

// Utility function to enforce correct tool usage. Takes two arguments on command line;
// the first <path>.png and the second <path>.xml
bool validate(int argc, char *argv[])
{
    // If tool not invoked with 2 args, bad usage.
    if (argc != 3)
    {
        std::cout << "Incorrect usage; use as ./hl <path>.png <path>.xml" << std::endl;
        return false;
    }

    std::string pngPathString(argv[1]);
    std::string xmlPathString(argv[2]);

    char delim = '.';
    size_t dotLoc;

    // If first arg not in form of <path>.png, bad usage.
    dotLoc = pngPathString.find_last_of(delim);
    if (dotLoc == std::string::npos)
    {
        std::cout << "Incorrect usage; use as ./hl <path>.png <path>.xml" << std::endl;
        return false;
    }
    std::string pngName(pngPathString.substr(0, dotLoc));
    std::string pngSuffix(pngPathString.substr(dotLoc, pngPathString.length() - (dotLoc - 1)));
    if (pngSuffix != ".png")
    {
        std::cout << "Incorrect usage; use as ./hl <path>.png <path>.xml" << std::endl;
        return false;
    }

    // If second arg not in form of <path>.xml, bad usage.
    dotLoc = xmlPathString.find_last_of(delim);
    if (dotLoc == std::string::npos)
    {
        std::cout << "Incorrect usage; use as ./hl <path>.png <path>.xml" << std::endl;
        return false;
    }
    std::string xmlName(xmlPathString.substr(0, dotLoc));
    std::string xmlSuffix(xmlPathString.substr(dotLoc, xmlPathString.length() - (dotLoc - 1)));
    if (xmlSuffix != ".xml")
    {
        std::cout << "Incorrect usage; use as ./hl <path>.png <path>.xml" << std::endl;
        return false;
    }

    // If <path1> does not match <path2>, bad usage.
    if (pngName != xmlName)
    {
        std::cout << "Incorrect usage; use as ./hl <path>.png <path>.xml" << std::endl;
        return false;
    }

    return true;
}

// Tries to open XML document from filename. If successful, prints success message
// and returns true. Otherwise, prints informational error message and returns false.
// If unsuccessful, result signals to terminate program.
bool getXMLDoc(char* filename, tinyxml2::XMLDocument& doc)
{
    tinyxml2::XMLError errorCode = doc.LoadFile(filename);

    switch (errorCode)
    {
        case tinyxml2::XML_SUCCESS:
            std::cout << "Loaded: " << filename << std::endl;
            break;
        case tinyxml2::XML_ERROR_FILE_NOT_FOUND:
        case tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
        case tinyxml2::XML_ERROR_FILE_READ_ERROR:
            std::cout << "Failed to load: " << filename << std::endl;
            return false;
        default:
            std::cout << "Not well-formed XML: " << filename << std::endl;
            return false;
    }

    return true;
}

// Traverses XMLElement tree owned by doc and extracts leaf elements to vector of UI
// elements.
void getUIElemsFromXML(tinyxml2::XMLDocument& doc, std::vector<tinyxml2::XMLElement*>& UIElems)
{

    tinyxml2::XMLElement* elem = doc.RootElement();

    while (elem)
    {

        // Traverse to leftmost leaf.
        while (!elem->NoChildren())
        {
            elem = elem->FirstChildElement();
        }

        // Add to list.
        UIElems.push_back(elem);

        // If sibling exists, move to next sibling.
        if (elem->NextSibling())
        {
            elem = elem->NextSibling()->ToElement();
        }
        // Otherwise, backtrack to first ancestor with siblings, then move to next sibling.
        else
        {
            while (elem && !elem->NextSibling())
            {
                elem = elem->Parent()->ToElement();
            }
            if (elem)
            {
                elem = elem->NextSibling()->ToElement();
            }
        }
    }

}

// Extracts UI element "bounds" data from XML element and parses from string into
// int coordinate pairs. The uiautomator framework dumps view information with "bounds"
// attribute describing top left and bottom right corners of each XML node, with
// the value having the form "[x1,y1][x2,y2]".
Bounds getIntBounds(tinyxml2::XMLElement* elem)
{
    // Get value of XMLAttribute named "bounds".
    const char *data = elem->Attribute("bounds");
    std::string boundsString(data);

    std::vector<int> nums;

    int start = 0;
    int end = 0;

    // Scan through value string and extract numerical values separated by non-numerical delimiters.
    for (; start < boundsString.length(); start++)
    {
        if (isdigit(boundsString[start]))
        {
            end = start;
            while (isdigit(boundsString[end + 1]))
            {
                end++;
            }
            std::string num = boundsString.substr(start, end - (start - 1));
            start = end + 1;
            nums.push_back(std::stoi(num));
        }
    }

    // Construct and return a Bounds with two Coordinate pairs constructed based on number order from
    // value string.
    return Bounds(Coordinate(nums[0], nums[1]), Coordinate(nums[2], nums[3]));
}

// Constructs a list of Bounds for each XMLElement in the list of UI elements.
void getCoordinatesFromUIElems(std::vector<tinyxml2::XMLElement*>& UIElems, std::vector<Bounds>& bounds)
{
    for (int e = 0; e < UIElems.size(); e++)
    {
        Bounds elemBounds = getIntBounds(UIElems[e]);
        bounds.push_back(elemBounds);
    }
}

// Uses a Bounds to draw a yellow highlighting box on an image.
void drawRect(cimg_library::CImg<unsigned char>& image, Bounds bounds)
{
    // The x and y values for outside points on the highlight box.
    int outerLeftX = bounds.first.first - 3;
    int outerRightX = bounds.second.first + 3;
    int outerTopY = bounds.first.second - 3;
    int outerBottomY = bounds.second.second + 3;

    const unsigned char yellow[4] = {255, 234, 0, 255};

    // Top line.
    image.draw_rectangle(outerLeftX, outerTopY, outerRightX, bounds.first.second, yellow);
    // Left line.
    image.draw_rectangle(outerLeftX, outerTopY, bounds.first.first, outerBottomY, yellow);
    // Right line.
    image.draw_rectangle(bounds.second.first, outerTopY, outerRightX, outerBottomY, yellow);
    // Bottom line.
    image.draw_rectangle(outerLeftX, bounds.second.second, outerRightX, outerBottomY, yellow);
}

// Draws highlighting boxes on an image for each Bounds in a list.
void outlineBoundedElems(cimg_library::CImg<unsigned char>& image, std::vector<Bounds>& bounds)
{
    for (int b = 0; b < bounds.size(); b++)
    {
        drawRect(image, bounds[b]);
    }
}

int main(int argc, char *argv[])
{

    // Enforce correct tool usage.
    if (!validate(argc, argv))
    {
        return 1;
    }

    tinyxml2::XMLDocument doc;
    std::vector<tinyxml2::XMLElement*> UIElems;
    std::vector<Bounds> bounds;

    // Unsuccessful image file open throws exception, in which case exit.
    try
    {
        cimg_library::CImg<unsigned char> image(argv[1]);

        // If XML doc cannot be opened or if XML is not well-formed, exit.
        if(!getXMLDoc(argv[2], doc))
        {
            return 1;
        }

        getUIElemsFromXML(doc, UIElems);
        getCoordinatesFromUIElems(UIElems, bounds);
        UIElems.clear();
        outlineBoundedElems(image, bounds);
        bounds.clear();

        // Construct filepath for highlighted copy of .png
        std::string pngPathString(argv[1]);
        std::string hlImagePath = pngPathString.substr(0, pngPathString.length() - 4) + "-hl.png";

        image.save(hlImagePath.c_str());
    }
    catch(cimg_library::CImgIOException)
    {
        std::cout << "Failed to load: " << argv[1] << std::endl;
        return 1;
    }

    return 0;

}