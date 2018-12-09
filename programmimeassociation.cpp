#include "programmimeassociation.h"

void ProgramMimeAssociation::parseLine(const std::string& line)
{
    std::string nm("Name=");
    std::string ex("Exec=");
    std::string ic("Icon=");
    std::string mi("MimeType=");

    if(line.compare(0, nm.length(), nm) == 0)
    {
        name = line.substr(nm.length(), line.length()-nm.length());
        name_set = true;
        return;
    }
    if(line.compare(0, ex.length(), ex) == 0)
    {
        execution = line.substr(ex.length(), line.length()-ex.length());
        exec_set = true;
        return;
    }
    if(line.compare(0, ic.length(), ic) == 0)
    {
        icon = line.substr(ic.length(), line.length()-ic.length());
        icon_set = true;
        return;
    }
    if(line.compare(0, mi.length(), mi) == 0)
    {
        mimes = line.substr(mi.length(), line.length()-mi.length());
        mime_set = true;
        return;
    }
}

bool ProgramMimeAssociation::supportsMime(const std::string& mime) const
{
    return (mimes.find(mime) != std::string::npos);
}

bool ProgramMimeAssociation::complete() const
{
    return name_set && exec_set && mime_set && icon_set;
}

QString ProgramMimeAssociation::toQString()
{
    std::string mimes_str = mimes.length() > 20 ? mimes.substr(0, 20) : mimes;
//        return QString("ex: %1")
//                .arg(QString::fromStdString(execution));
    return QString("ex: %1  icon: %2    mimes: %3")
            .arg(QString::fromStdString(execution))
            .arg(QString::fromStdString(icon))
            .arg(QString::fromStdString(mimes_str));
}

QString ProgramMimeAssociation::getIconPath()
{
    return FileHandler::getProgramsIconPath( QString::fromStdString(icon) );
}
