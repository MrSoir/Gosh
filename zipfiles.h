#ifndef ZIPFILES_H
#define ZIPFILES_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QStringList>
#include <QFileInfoList>
#include <QProcess>

#include "staticfunctions.h"
#include "canceller.h"

namespace ZipFiles {

    void zipFiles(const QFileInfo& tarZipFilePath,
                  const QList<QFileInfo>& filePathsToZip,
                  bool waitForFinished = true,
                  std::function<void()> callAfterProgress = nullptr,
                  std::shared_ptr<bool> cancelled = std::shared_ptr<bool>());

    void zipFiles(QString tarZipFilePath,
                  const QList<QString>& filePathsToZip,
                  bool waitForFinished = true,
                  std::function<void()> callAfterProgress = nullptr,
                  std::shared_ptr<bool> cancelled = std::shared_ptr<bool>());

    QString argumentsToStr(const QString& executable, const QStringList& arguments);
    QList<QString> createRelativePaths(const QDir& rootDir, const QList<QString>& paths);
    bool checkIfTargetZipFilePathIsSubFileOfFoldersToZip(const QString& tarZipFilePath,
                                                         const QList<QString>& zipFilePaths);
    QString getTarZipFilePathThatIsNotSubFileOfFilesToZip(const QString& tarZipFilePath,
                                                      const QList<QString>& filePathsToZip,
                                                      bool& success);


//    -------------------------------------------------------------------------------------

    void unzipFile(const QFileInfo& tarZipFilePath,
                   const QFileInfo& extractionPath,
                   bool waitForFinished,
                   std::function<void()> callAfterProgress,
                   std::shared_ptr<bool> cancelled);
}


#endif // ZIPFILES_H
