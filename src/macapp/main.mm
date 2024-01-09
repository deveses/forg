
#include <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#include <MacTypes.h>
#include <CoreFoundation/CoreFoundation.h>
#include "forg/script/xml/XMLParser.h"
#include <iostream>
int main(int argc, char* argv[])
{
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resBundle = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char bundlePath[PATH_MAX];
    if (CFURLGetFileSystemRepresentation(resBundle, TRUE, (UInt8*)bundlePath, PATH_MAX))
    {
        CFRelease(resBundle);
        std::cout << "resource path: " << bundlePath << "\n";
        chdir(bundlePath);
    }

    forg::script::xml::XMLParser config;
    config.Open("config.xml");
    forg::script::xml::XMLDocument* xml_doc = config.Parse();

    if (xml_doc)
    {
        std::cout << "File loaded!\n";
    }
    else
    {
        std::cout << "Unable to load xml file!\n";
    }

    int result = NSApplicationMain(argc, (const char**)argv);

    return result;
}