#ifndef FILESELECTOR_H
#define FILESELECTOR_H

#include <QFileInfo>
#include <QString>
#include <QDir>
#include <QVector>
#include <QList>
#include <QSet>

class FileSelector
{
public:
    FileSelector();

    bool isSelected(QString path) const;

    bool filesSelected() const;
    bool foldersSelected() const;

    bool singleFolderSelected() const;
    bool singleFileSelected() const;
    bool singleContentSelected() const;
    int selectionCounter() const;
private:
    QSet<QString> m_selectedFiles;
    QSet<QString> m_selectedFolders;
};

#endif // FILESELECTOR_H
