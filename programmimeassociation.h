#ifndef PROGRAMMIMEASSOCIATION_H
#define PROGRAMMIMEASSOCIATION_H

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

#include "filehandler.h"

class ProgramMimeAssociation
{
public:
    void parseLine(const std::string& line);

    bool supportsMime(const std::string& mime) const;

    bool complete() const;
    QString toQString();

    QString getIconPath();

    std::string name;
    std::string execution;
    std::string mimes;
    std::string icon;
private:
    bool name_set = false;
    bool exec_set = false;
    bool mime_set = false;
    bool icon_set = false;
};
#endif // PROGRAMMIMEASSOCIATION_H
