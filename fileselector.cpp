#include "fileselector.h"

FileSelector::FileSelector()
{

}

bool FileSelector::isSelected(QString path) const
{
    return m_selectedFolders.contains(path) ||
           m_selectedFiles.contains(path);
}

bool FileSelector::filesSelected() const
{
    return m_selectedFiles.size() > 0;
}

bool FileSelector::foldersSelected() const
{
    return m_selectedFolders.size() > 0;
}

bool FileSelector::singleFolderSelected() const
{
    return m_selectedFolders.size() == 1;
}

bool FileSelector::singleFileSelected() const
{
    return m_selectedFiles.size() == 1;
}

bool FileSelector::singleContentSelected() const
{
    return (m_selectedFiles.size() + m_selectedFolders.size()) == 1;
}

int FileSelector::selectionCounter() const
{
    return m_selectedFiles.size() + m_selectedFolders.size();
}
