#ifndef __FILEMONITOR_H
#define __FILEMONITOR_H
/**
 * FileMonitor.h
 *
 * File monitoring module
 *
 * Used when wanting to monitor changes in files.
 *
 * Example application could be to monitor configuration files
 * in an application that needs to react to changes at run-time.
 */

#include <sys/stat.h>

/**
 * File monitor class
 */
class FileMonitor
{
public:
    FileMonitor(const char * fileName);
    ~FileMonitor();
    
    bool hasBeenModified();

private:
    char * fileName_;
    char * dirName_;
    struct stat stat_;
};

#endif /* __FILEMONITOR_H */

