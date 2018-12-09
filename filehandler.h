#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QString>
#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QVector>
#include <QProcess>
#include <QObject>
#include <QMimeDatabase>
#include <QPixmap>
#include <QIcon>

#include <memory>
#include <functional>
#include <sstream>
#include <string>
#include <fstream>

#include "programmimeassociation.h"
#include "staticfunctions.h"

class ProgramMimeAssociation;

namespace FileHandler
{
    static QDir programIconsRootDir("/usr/share/icons");
    static QMimeDatabase mimeDatabase;

    static QSet<QString> programIconsFileExtensions;
    static std::vector<ProgramMimeAssociation> installedPrograms;

//    --------------------------------------------------------------------------------

    void initialize();

    void iterateOverFiles(const QDir& dir,
                         std::function<void(const QString&)> filePathHandler,
                         std::function<bool()> cancel = nullptr);


    void getProgram_MimeType_Association(const std::string& filePath,
                                         ProgramMimeAssociation& prgrmMimeAssoc);

    std::vector<ProgramMimeAssociation> getAllPrograms();
    std::vector<ProgramMimeAssociation> getProgramsThatCanHandleMimeType(const std::string& mime);


    QSet<QString> getIconFileExtensions();

    QString getProgramsIconPath(QString programName);

    QString getMimeOf(const QFileInfo& fi);
}

#endif // FILEHANDLER_H
