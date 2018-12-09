#ifndef FILEINFOREF_H
#define FILEINFOREF_H

#include <QFileInfo>
#include <memory>
#include "fileinfobd.h"

class FileInfoRef
{
public:
    explicit FileInfoRef(std::weak_ptr<FileInfoBD> fileInfoBD,
                std::weak_ptr<QFileInfo> file = std::weak_ptr<QFileInfo>());

    const std::weak_ptr<FileInfoBD> m_fileInfoBD;
    const std::weak_ptr<QFileInfo> m_file;
};

#endif // FILEINFOREF_H
