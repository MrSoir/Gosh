#include "zipfiles.h"

void ZipFiles::zipFiles(const QFileInfo& tarZipFilePath, const QList<QFileInfo>& filePathsToZip,
                        bool waitForFinished,
                        std::function<void()> callAfterProgress,
                        std::shared_ptr<bool> cancelled)
{
    QList<QString> filePathsToZip_Str;
    foreach(const QFileInfo& entry, filePathsToZip)
        filePathsToZip_Str.push_back( entry.absoluteFilePath() );

    zipFiles(tarZipFilePath.absoluteFilePath(), filePathsToZip_Str, waitForFinished,
             callAfterProgress, cancelled);
}

// zipFiles kuemmert sich komplett um alls. d.h.: falls tarZipFilePath ein subfile eines der
// zu zippenden ordner ist, kuemmert sich zipFiles darum und verhindert eine endlosschleife
// (== zip-datei versucht ordner zu zippen, in dem sich die zip-datei selbst befindet (zip-datei
// muss dummerweise zuerst erstellt werden, bevor gezippt werden kann)):

void ZipFiles::zipFiles(QString tarZipFilePath, const QList<QString>& filePathsToZip,
                        bool waitForFinished,
                        std::function<void()> callAfterProgress,
                        std::shared_ptr<bool> cancelled)
{
    qDebug() << "tarZipFilePath: " << tarZipFilePath;

    QTimer* progressTimer = nullptr;
    if(waitForFinished && callAfterProgress)
    {
        progressTimer = new QTimer();
        QObject::connect(progressTimer, &QTimer::timeout, [=](){
            if(callAfterProgress)
                callAfterProgress();
        });
        progressTimer->start(100);
    }

    // checken, ob einer der zu zippenden dateien(oder eben ordner), ein parent-folder des
    // tarZipFilePath ist. falls ja, wuerde das angestrebte python-scripte in einer
    // endlosschleife versuchen einen ordner in sich selbst zu zippen:

    QString tarZipFilePathTemp = StaticFunctions::generateRandomFilePath(QDir("temp"), QString(".zip"));

    qDebug() << "tarZipFilePathTemp: " << tarZipFilePathTemp;

    QDir targetDir = StaticFunctions::getDir(tarZipFilePathTemp);

    qDebug() << "filePathsToZip:";
    foreach(const QString& path, filePathsToZip)
        qDebug() << "   " << path;

    QFileInfo executable( QString("scripts%1zip_files.py").arg(QDir::separator()) );
    QString executablePath = executable.absoluteFilePath();

    qDebug() << "executable exists: " << QFileInfo( QString("scripts%1zip_files.py")
                                                    .arg(QDir::separator()) ).exists();

    QStringList arguments;
    arguments << executablePath;
    arguments << tarZipFilePathTemp;
    foreach(const auto& path, filePathsToZip)
        arguments << path;

    QString program("python3");

    QString command = argumentsToStr(program, arguments);
    qDebug() << "command:\n" << command;

    QTimer* cancelTimer = nullptr;
    QProcess* process = new QProcess();
    if(waitForFinished)
    {
        cancelTimer = new QTimer();
        QObject::connect(cancelTimer, &QTimer::timeout, [=](){
            if(cancelled && *cancelled)
                process->kill();
        });
        cancelTimer->start(100);
    }
    process->setWorkingDirectory( targetDir.absolutePath() );
    process->start(program, arguments);
    if(waitForFinished)
        process->waitForFinished( -1 );

    if(waitForFinished)
    {
        if(cancelTimer->isActive())
            cancelTimer->stop();
        cancelTimer->deleteLater();
    }

    if(tarZipFilePathTemp != tarZipFilePath)
    {
        if(QFileInfo(tarZipFilePath).exists())
        {
            tarZipFilePath = StaticFunctions::getUniqueFileName(tarZipFilePath);
        }
        StaticFunctions::moveFile(tarZipFilePathTemp, tarZipFilePath);
    }

    if(waitForFinished && callAfterProgress)
    {
        if(progressTimer->isActive())
            progressTimer->stop();
        progressTimer->deleteLater();
    }
}

QString ZipFiles::argumentsToStr(const QString &executable, const QStringList &arguments)
{
    QString str;
    str += executable + " ";
    foreach (const auto& argument, arguments) {
        str += argument + " ";
    }
    return str.trimmed();
}
QList<QString> ZipFiles::createRelativePaths(const QDir &rootDir, const QList<QString> &paths)
{
    QList<QString> relPaths;
    for(int i=0; i < paths.size(); ++i)
    {
        relPaths.push_back( rootDir.relativeFilePath(paths[i]) );
    }
    return relPaths;
}

bool ZipFiles::checkIfTargetZipFilePathIsSubFileOfFoldersToZip(const QString& tarZipFilePath,
                                                               const QList<QString>& filesPathsToZip)
{
    foreach (const QString& fileToZip, filesPathsToZip)
    {
        if(StaticFunctions::isSubDirectory(tarZipFilePath, fileToZip))
            return true;
    }
    return false;
}
QString ZipFiles::getTarZipFilePathThatIsNotSubFileOfFilesToZip(const QString& tarZipFilePath,
                                                            const QList<QString>& filePathsToZip,
                                                            bool& success)
{
    QString fileName = QFileInfo(tarZipFilePath).fileName();

    QDir targetDir = StaticFunctions::getDir(tarZipFilePath);
    QString zipPath = StaticFunctions::getAbsoluteFilePathFromDir(targetDir);
    while( checkIfTargetZipFilePathIsSubFileOfFoldersToZip(zipPath, filePathsToZip) )
    {
        if( !targetDir.cdUp() )
        {
            success = false;
            return QString("");
        }
    }

    success = false;

    QString appendix = QString("%1%2").arg(QDir::separator())
                                      .arg(fileName);

    QString dirPath = StaticFunctions::getAbsoluteFilePathFromDir(targetDir);

    return QString("%1%2").arg(dirPath)
                          .arg(appendix);
}

void ZipFiles::unzipFile(const QFileInfo &tarZipFilePath, const QFileInfo &extractionPath,
                         bool waitForFinished,
                         std::function<void()> callAfterProgress,
                         std::shared_ptr<bool> cancelled)
{
    qDebug() << "tarZipFilePath: " << tarZipFilePath;

    QFileInfo executable( QString("scripts%1unzip_files.py").arg(QDir::separator()) );
    QString executablePath = executable.absoluteFilePath();

    qDebug() << "executable exists: " << QFileInfo( QString("scripts%1unzip_files.py")
                                                    .arg(QDir::separator()) ).exists();

    QStringList arguments;
    arguments << executablePath;
    arguments << tarZipFilePath.absoluteFilePath();
    arguments << extractionPath.absoluteFilePath();

    QString program("python3");

    QString command = argumentsToStr(program, arguments);
    qDebug() << "command:\n" << command;

    QTimer* cancelTimer = nullptr;
    QProcess* process = new QProcess();
    if(waitForFinished)
    {
        cancelTimer = new QTimer();
        QObject::connect(cancelTimer, &QTimer::timeout, [=](){
            if(cancelled && *cancelled)
                process->kill();
        });
        cancelTimer->start(100);
    }
    process->setWorkingDirectory( StaticFunctions::getDir(tarZipFilePath).absolutePath() );
    process->start(program, arguments);
    if(waitForFinished)
        process->waitForFinished( -1 );

    if(waitForFinished)
    {
        if(cancelTimer->isActive())
            cancelTimer->stop();
        cancelTimer->deleteLater();
    }

    if(waitForFinished && callAfterProgress)
    {
        callAfterProgress();
    }
}
