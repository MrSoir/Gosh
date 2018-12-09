#include "fileinforef.h"


FileInfoRef::FileInfoRef(std::weak_ptr<FileInfoBD> fileInfoBD,
                                  std::weak_ptr<QFileInfo> file)
    : m_fileInfoBD(fileInfoBD),
      m_file(file)
{}
