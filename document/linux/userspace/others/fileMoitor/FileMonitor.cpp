#include "FileMonitor.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>

FileMonitor::FileMonitor(const char * fileName)
{
    if (fileName == 0) {
       throw std::runtime_error( "fileName is a null pointer" );
    }
    fileName_ = new char[strlen(fileName)+1];
    std::strcpy(fileName_, fileName);
    
    // find the dirName = "/dir/dir" from the fileName = "/dir/dir/file"
    char * pos = std::strrchr(fileName_, '/');
    if (pos == 0) {
        std::cerr << "ERROR  : " << __func__ << "(): Can not find directory in fileName " << fileName_ << std::endl;
        throw std::runtime_error( "should full path for fileName" );
    }
    dirName_ = new char[(pos - fileName_)+1];
    memcpy(dirName_, fileName_, (size_t)(pos - fileName_));
    dirName_[(size_t)(pos-fileName_)] = '\0';

    std::memset((void*)&stat_, 0, sizeof(stat_));
}

FileMonitor::~FileMonitor()
{
    delete[] fileName_;
    delete[] dirName_;
}

bool FileMonitor::hasBeenModified()
{
    struct stat curStat;
    
    // "stat" does not sense directly if a file has been removed when the file is mounted via nfs
    // to make "stat" work we need to update the cached "stat" data by opening and closing the directory
    DIR* dir = opendir( dirName_ );
    if( dir == 0 ) {
        return false;
    } 
    else {
        closedir(dir);   
    }
    
    if (stat(fileName_, &curStat) == 0)
    {
        if (curStat.st_mtime != stat_.st_mtime)
        {
            stat_ = curStat;
			printf("bb\n");
            return true;
        }
    }
    else
    {
        if (stat_.st_mtime != 0)
        {
            // A previously existing file has been removed
            // Return true only once for a removed file
			printf("aa\n");
            stat_.st_mtime = 0;
            return true;
        }
    }

    return false;
}

