#ifndef WINDOWCOORDINATOR_H
#define WINDOWCOORDINATOR_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QDialog>
#include <QString>
#include <QFile>
#include <QDataStream>

#include <memory>
#include <functional>
#include <vector>
#include <sstream>

#include "orientation.h"
#include "filescoordinator.h"
#include "staticfunctions.h"
#include "windowcoordinatorpane.h"

class FilesCoordinator;
class WindowCoordinatorPane;

//namespace Orientation
//{
//    enum ORIENTATION{HORIZONTAL, VERTICAL};
//}

class WindowCoordSettings
{
public:
    QList<QString> paths;
    int orientation; // 0 == VERTICAL | 1 == HORIZONTAL
    QList<QList<int>> splitterRatios;
};
QDataStream &operator<<(QDataStream &out, const WindowCoordSettings &s);
QDataStream &operator>>(QDataStream &in, WindowCoordSettings &s);

class WindowCoordinator : public QObject
{
    Q_OBJECT
public:
    explicit WindowCoordinator(QVector<QDir>* initPaths = nullptr,
                               QObject* parent = nullptr);
    ~WindowCoordinator();

    void removeWindow(int id = -1);
    void addWindow();

    void setFullScreenCaller(std::shared_ptr<DynamicFunctionCaller<QString, std::function<void()>>> fullScreenCaller);

    QString getCurrentFocusedDir() const;

    void changeOrientation();
    int getWindowCount();
    Orientation::ORIENTATION getOrientation();

    QLayout* getWindowLayout(int windowId);

    WindowCoordinatorPane* getWindowCoordinatorPane();
    void resetWindowCoordinatorPane();

    void setSplitterSizes(QList<QList<int>> splitterRatios);

    void setSelf(std::weak_ptr<WindowCoordinator> self);

    void setFullScreen();

    void setOnLabelChanged(std::function<void(QDir)> onLabelChanged);
    void setOnOpenFoldersInNewTab(std::function<void(QVector<QDir>)> onOpenFoldersInNewTab);
    void setOnCloseTab(std::function<void()> onCloseTab);
signals:
    void labelChanged(QDir newLabel);
    void openFoldersInNewTab(QVector<QDir> foldersToOpen);
    void closeTab();

    void revalidateWindowCoordinatorPane();
    void clearWindowCoordinatorPanes_Splitter();

    void setIncludeHiddenFilesSGNL(bool includeHiddenFiles);
public slots:
    void focusedWindowChanged(std::weak_ptr<FilesCoordinator> filesCoordRequestingFocus);
    void rootChanged(QDir newPath);

    void saveSettings();
    void loadSettings();

    void startup();

    void splitterRatiosChanged(QList<QList<int>> splitterRatios);

    void setIncludeHiddenFiles(bool includeHiddenFiles);
protected:
private:
    void addWindowHelper(QDir initDir);

    void revalidateLayout();
    void resetFilesCoordinatorWidgets();

    void emitLabelChanged(QDir newLabel);
    std::function<void(QDir)> m_onLabelChanged = nullptr;

    WindowCoordSettings generateSettings();

    int m_windowCounter = 1;
//    int m_windowCounter = 0;//1;
    int m_maxWindows = 4;
    QVector<std::shared_ptr<FilesCoordinator>> m_windows;
    Orientation::ORIENTATION m_orientation = Orientation::ORIENTATION::VERTICAL;

//    QString m_initPath = QString("/home/bigdaddy/Documents/Python/Fluent_Code/testFold");
    QStringList m_initPath = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::DocumentsLocation);

    std::shared_ptr<DynamicFunctionCaller<QString, std::function<void()>>> m_fullScreenCaller
             = std::shared_ptr<DynamicFunctionCaller<QString, std::function<void()>>>();
    bool m_isFullScreen = false;

    QString m_starup_res_path = QString("%1%2%3").arg("resources").arg(QDir::separator()).arg("/startup.res");

    std::weak_ptr<FilesCoordinator> m_curntlyFocusedFilesCoord = std::weak_ptr<FilesCoordinator>();

    WindowCoordinatorPane* m_windowCoordPane = nullptr;
    std::weak_ptr<WindowCoordinator> m_self = std::weak_ptr<WindowCoordinator>();

    std::function<void(QVector<QDir>)> m_onOpenFoldersInNewTab;
    std::function<void()> m_onCloseTab;

    QList<QList<int>> m_splitterRatios;
};

#endif // WINDOWCOORDINATOR_H
