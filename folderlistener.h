#ifndef FOLDERLISTENER_H
#define FOLDERLISTENER_H

#include <memory>
#include <QString>

class FileInfoBD;

class FolderListener
{
public:
    virtual void folderChanged(std::weak_ptr<FileInfoBD> f) = 0;
    virtual void folderElapsed(std::weak_ptr<FileInfoBD> f) = 0;
    virtual void sortingChanged(std::weak_ptr<FileInfoBD> f) = 0;
    virtual void addDirectoryToWatcher(QString directory) = 0;
    virtual void removeDirectoryFromWatcher(QString directory) = 0;
};

#endif // FOLDERLISTENER_H
