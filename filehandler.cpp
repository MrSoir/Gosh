#include "filehandler.h"

void FileHandler::iterateOverFiles(const QDir& dir,
                     std::function<void(const QString&)> filePathHandler,
                     std::function<bool()> cancel)
{
    if(cancel && cancel())
        return;

    QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    foreach(const auto& file, files)
    {
        filePathHandler(file.absoluteFilePath());
        if(cancel && cancel())
            return;
    }

    QFileInfoList folders = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
    foreach (const auto& folder, folders)
    {
        iterateOverFiles(QDir(folder.absoluteFilePath()), filePathHandler, cancel);
        if(cancel && cancel())
            return;
    }
}


void FileHandler::getProgram_MimeType_Association(const std::string& filePath, ProgramMimeAssociation& prgrmMimeAssoc)
{
    std::ifstream infile(filePath);
    if(infile)
    {
        std::string line;
        while (std::getline(infile, line))
        {
            prgrmMimeAssoc.parseLine(line);
            if(prgrmMimeAssoc.complete())
                return;
        }
    }
}

std::vector<ProgramMimeAssociation> FileHandler::getAllPrograms()
{
    std::vector<ProgramMimeAssociation> programs;
    QFileInfoList allPrgrms = StaticFunctions::getFilesInDirectory(QDir("/usr/share/applications"), true);

    for(auto it = allPrgrms.begin(); it != allPrgrms.end(); ++it)
    {
        ProgramMimeAssociation program;

        getProgram_MimeType_Association(it->absoluteFilePath().toStdString(), program);

        if(program.complete())
            programs.push_back(program);
    }
    return programs;
}
std::vector<ProgramMimeAssociation> FileHandler::getProgramsThatCanHandleMimeType(const std::string& mime)
{
    std::vector<ProgramMimeAssociation> handlers;
    for(int i=0; i < installedPrograms.size(); ++i)
    {
        if( installedPrograms[i].supportsMime(mime) )
            handlers.push_back( installedPrograms[i] );
    }
    return handlers;
}


QSet<QString> FileHandler::getIconFileExtensions()
{
    QDir dir = programIconsRootDir;

    QSet<QString> imageFileExtensions;
    iterateOverFiles(dir, [&](const QString& filePath){
        QFileInfo fi(filePath);
        QString mimeType = getMimeOf(fi);

        if(mimeType.startsWith(QString("image")))
        {
            QString fileExtension = StaticFunctions::getFileTypeWithDot( fi.fileName() );
            if( !fileExtension.isEmpty() &&
                    !imageFileExtensions.contains(fileExtension) )
                imageFileExtensions.insert( fileExtension );
        }
    });
    return imageFileExtensions;
}

QString FileHandler::getProgramsIconPath(QString programName)
{
    if(QFileInfo(programName).exists())
        return programName;

    QString programsIconPath;
    bool* cancel = new bool(false);
    iterateOverFiles(programIconsRootDir, [&](const QString& filePath){
        QFileInfo fi(filePath);
        QString fileExtension = StaticFunctions::getFileTypeWithDot(fi.fileName());
        if(fi.fileName().startsWith(programName) &&
                programIconsFileExtensions.contains(fileExtension))
        {
            programsIconPath = filePath;
            *cancel = true;
        }
    }, [&](){return *cancel;});

    delete cancel;

    return programsIconPath;
}


QString FileHandler::getMimeOf(const QFileInfo &fi)
{
    return mimeDatabase.mimeTypeForFile( fi.absoluteFilePath() ).name();
}


void FileHandler::initialize()
{
    programIconsFileExtensions = getIconFileExtensions();
    installedPrograms = getAllPrograms();
}
