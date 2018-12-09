#include "windowcoordinator.h"

QDataStream &operator<<(QDataStream &out, const WindowCoordSettings &s)
{
    out << s.paths << s.orientation << s.splitterRatios;
    return out;
}
QDataStream &operator>>(QDataStream &in, WindowCoordSettings &s)
{
    s = WindowCoordSettings();
    in >> s.paths >> s.orientation >> s.splitterRatios;
    return in;
}

//-----------------------------------------------------------------------------------------------------

WindowCoordinator::WindowCoordinator(QVector<QDir>* initPaths,
                                     QObject *parent)
    : QObject(parent)
{
//    startup();
    if(initPaths != nullptr)
    {
        m_windowCounter = 0;
        for(int i=0; i < initPaths->size() && i < m_maxWindows; ++i)
        {
            QDir dir = (*initPaths)[i];
            addWindowHelper( dir );
        }
    }
    else
    {
        addWindow();
    }
}

WindowCoordinator::~WindowCoordinator()
{
    for(int i=0; i < m_windows.size(); i++){
        if(m_windows[i])
            m_windows[i].reset();
    }
    m_windows.clear();

    if(m_fullScreenCaller)
        m_fullScreenCaller.reset();

    m_curntlyFocusedFilesCoord.reset();

    m_onOpenFoldersInNewTab = nullptr;
    m_onCloseTab = nullptr;
    m_onLabelChanged = nullptr;
}

void WindowCoordinator::removeWindow(int id)
{
    if(id < m_windowCounter && id >= 0
            && m_windows.size() > 1){

        emit clearWindowCoordinatorPanes_Splitter();
//        if(m_windowCoordPane != nullptr)
//            m_windowCoordPane->clearSplitter();

        if(m_windows[id]){
            m_windows[id].reset();
        }
        m_windows.remove(id);
        --m_windowCounter;

        this->revalidateLayout();
    }
}

void WindowCoordinator::addWindow()
{
    QDir initDir(tr(""));
    bool rootWasSet = false;
    if (auto locked = m_curntlyFocusedFilesCoord.lock())
    {
        QString absPath = locked->getCurRootPath();
        if( !absPath.isEmpty() && QFile(absPath).exists())
        {
            initDir = QDir(absPath);
            rootWasSet = true;
        }
    }
    if( !rootWasSet && m_initPath.size() > 0)
    {
        initDir = QDir(m_initPath[0]);
    }

    addWindowHelper(initDir);
}
void WindowCoordinator::addWindowHelper(QDir initPath)
{
    if(m_windowCounter < m_maxWindows){

        while(m_windows.size() < m_windowCounter+1){            
            std::shared_ptr<FilesCoordinator> filesCoord = std::make_shared<FilesCoordinator>(nullptr);
            filesCoord->setSelf(filesCoord);

            m_curntlyFocusedFilesCoord = filesCoord;

            filesCoord->setFocusCaller([=](std::weak_ptr<FilesCoordinator> filesCoordRequestingFocus){
                m_curntlyFocusedFilesCoord = filesCoordRequestingFocus;
                QDir newLabel("");
                if(auto locked = m_curntlyFocusedFilesCoord.lock())
                    newLabel = QDir(locked->getCurRootPath());

                emitLabelChanged( newLabel );
            });
            filesCoord->setRootChangedCaller([=](QDir newPath){
                this->saveSettings();

                emitLabelChanged(newPath);
            });
            filesCoord->setOnOpenFoldersInNewTab([=](QVector<QDir> folderToOpen){
                if(m_onOpenFoldersInNewTab)
                    m_onOpenFoldersInNewTab(folderToOpen);

                emit openFoldersInNewTab(folderToOpen);
            });
            filesCoord->setOnCloseTab([=](){
                if(m_onCloseTab)
                    m_onCloseTab();
                emit closeTab();
            });

            filesCoord->setRootFolder(initPath);
//            filesCoord->setRootFolder(QDir(QString("/home/bigdaddy/Documents")));

            m_windows.append(filesCoord);
        }

        ++m_windowCounter;

        revalidateLayout();
    }
}

void WindowCoordinator::setFullScreenCaller(std::shared_ptr<DynamicFunctionCaller<QString, std::function<void()>>> fullScreenCaller)
{
    m_fullScreenCaller = fullScreenCaller;
}

QString WindowCoordinator::getCurrentFocusedDir() const
{
    if(auto locked = m_curntlyFocusedFilesCoord.lock())
        return locked->getCurRootPath();
    return QString("");
}

WindowCoordinatorPane* WindowCoordinator::getWindowCoordinatorPane()
{
    m_windowCoordPane = new WindowCoordinatorPane(m_self, m_splitterRatios);

    // this -> windowCoordinatorPane
    connect(this, &WindowCoordinator::revalidateWindowCoordinatorPane,      m_windowCoordPane, &WindowCoordinatorPane::revalidateLayout);
    connect(this, &WindowCoordinator::clearWindowCoordinatorPanes_Splitter, m_windowCoordPane, &WindowCoordinatorPane::clearSplitter);

    // windowCoordinatorPane -> this
    connect(m_windowCoordPane, &WindowCoordinatorPane::addWindow,         this, &WindowCoordinator::addWindow);
    connect(m_windowCoordPane, &WindowCoordinatorPane::removeWindow,      this, &WindowCoordinator::removeWindow);
    connect(m_windowCoordPane, &WindowCoordinatorPane::changeOrientation, this, &WindowCoordinator::changeOrientation);
    connect(m_windowCoordPane, &WindowCoordinatorPane::requestFullScreen, this, &WindowCoordinator::setFullScreen);
    connect(m_windowCoordPane, &WindowCoordinatorPane::splitterRatiosChangedSGNL, this, &WindowCoordinator::splitterRatiosChanged);

    return m_windowCoordPane;
}
void WindowCoordinator::resetWindowCoordinatorPane()
{
    m_windowCoordPane = nullptr;
    resetFilesCoordinatorWidgets();
}

void WindowCoordinator::changeOrientation()
{
    m_orientation = (m_orientation == Orientation::ORIENTATION::VERTICAL) ?
                    Orientation::ORIENTATION::HORIZONTAL :
                    Orientation::ORIENTATION::VERTICAL;
    revalidateLayout();
}

int WindowCoordinator::getWindowCount()
{
    return m_windowCounter;
}

Orientation::ORIENTATION WindowCoordinator::getOrientation()
{
    return m_orientation;
}

QLayout *WindowCoordinator::getWindowLayout(int windowId)
{
    return m_windows[windowId]->getLayout();
}

void WindowCoordinator::setSplitterSizes(QList<QList<int> > splitterRatios)
{
    m_splitterRatios = splitterRatios;
    saveSettings();
}

void WindowCoordinator::setSelf(std::weak_ptr<WindowCoordinator> self)
{
    m_self = self;
}

WindowCoordSettings WindowCoordinator::generateSettings()
{
    WindowCoordSettings settings;
    settings.orientation = (m_orientation == Orientation::ORIENTATION::VERTICAL) ? 0 : 1;

    QList<QString> paths;
    for(int i=0; i < m_windows.size(); ++i)
    {
        if(m_windows[i])
        {
            paths.push_back( m_windows[i]->getCurRootPath() );
        }
    }
    settings.paths = paths;

    settings.splitterRatios = m_splitterRatios;

    return settings;
}
void WindowCoordinator::saveSettings()
{
//    // if resources-dir does not exist, create it:
//    QDir dir;
//    if( !dir.exists(tr("resources"))){
//        dir.mkdir(tr("resources"));
//    }
//    // if dir/resources.res does not exist, create it:
//    if( !QFileInfo(m_starup_res_path).exists() ){
//        QFile file(m_starup_res_path);
//        file.open(QIODevice::WriteOnly);
//        file.close();
//    }

//    QFile file(m_starup_res_path);

//    if(!file.open(QIODevice::WriteOnly))
//    {
//        qDebug() << "Could not open " << m_starup_res_path;
//        return;
//    }

//    QDataStream out(&file);
//    out.setVersion(QDataStream::Qt_5_1);

//    WindowCoordSettings settings = generateSettings();

//    out << settings;

//    file.flush();
//    file.close();
}
void WindowCoordinator::loadSettings()
{
//    WindowCoordSettings settings;

//    QFile file(m_starup_res_path);

//    if(!file.open(QIODevice::ReadOnly))
//    {
//        qDebug() << "Could not open " << m_starup_res_path;
//        return;
//    }

//    QDataStream in(&file);
//    in.setVersion(QDataStream::Qt_5_1);

//    in >> settings;

//    file.close();

//    QList<QString> paths = settings.paths;
//    for(int i=0; i < paths.size(); ++i)
//    {
//        if(m_windowCounter < i+1)
//            addWindow();

//        if( m_windows[i])
//            m_windows[i]->setRootFolder(QDir(paths[i]));
//    }
//    while(m_windowCounter > paths.size() && m_windowCounter > 1)
//    {
//        removeWindow();
//    }

//    if(m_windowCoordPane != nullptr)
//        m_windowCoordPane->setSplitterRatios(settings.splitterRatios);
//    m_splitterRatios = settings.splitterRatios;

//    m_orientation = settings.orientation == 0 ?
//                    Orientation::ORIENTATION::VERTICAL :
//                    Orientation::ORIENTATION::HORIZONTAL;
}

void WindowCoordinator::startup()
{
//    // if resources-dir does not exist, create it:
//    QDir dir;
//    if( !dir.exists(tr("resources"))){
//        dir.mkdir(tr("resources"));
//    }
//    // if dir/resources.res does not exist, create it:
//    if( !QFileInfo(m_starup_res_path).exists() ){
//        QFile file(m_starup_res_path);
//        file.open(QIODevice::WriteOnly | QIODevice::Append);
//        file.close();
//    }

    //    loadSettings();
}

void WindowCoordinator::splitterRatiosChanged(QList<QList<int> > splitterRatios)
{
    qDebug() << "in WindowCoordinator::splitterRatiosChanged: splitterRatios.size: " << splitterRatios.size();
    m_splitterRatios = splitterRatios;
}

void WindowCoordinator::setIncludeHiddenFiles(bool includeHiddenFiles)
{
    for(int i=0; i <  m_windows.size(); ++i)
    {
        m_windows[i]->setIncludeHiddenFiles(includeHiddenFiles);
    }
}

void WindowCoordinator::revalidateLayout(){
    resetFilesCoordinatorWidgets();

    emit revalidateWindowCoordinatorPane();
}

void WindowCoordinator::resetFilesCoordinatorWidgets()
{
    for(int i=0; i < m_windows.size(); i++){
        if(m_windows[i]){
            m_windows[i]->resetWidgets();
        }
    }
}

void WindowCoordinator::emitLabelChanged(QDir newLabel)
{
    if(m_onLabelChanged)
        m_onLabelChanged(newLabel);

    emit labelChanged(newLabel);
}

void WindowCoordinator::setFullScreen()
{
    m_isFullScreen = !m_isFullScreen;
    if(m_isFullScreen){
        if(m_fullScreenCaller && m_fullScreenCaller->containsFunction("setFullScreen")){
            m_fullScreenCaller->getFunction(QString("setFullScreen"))();
        }
    }else{
        if(m_fullScreenCaller && m_fullScreenCaller->containsFunction("setMaximized")){
            m_fullScreenCaller->getFunction(QString("setMaximized"))();
        }
    }
}

void WindowCoordinator::setOnLabelChanged(std::function<void(QDir)> onLabelChanged)
{
    m_onLabelChanged = onLabelChanged;
}

void WindowCoordinator::setOnOpenFoldersInNewTab(std::function<void (QVector<QDir>)> onOpenFoldersInNewTab)
{
    m_onOpenFoldersInNewTab = onOpenFoldersInNewTab;
}

void WindowCoordinator::setOnCloseTab(std::function<void ()> onCloseTab)
{
    m_onCloseTab = onCloseTab;
}

void WindowCoordinator::focusedWindowChanged(std::weak_ptr<FilesCoordinator> filesCoordRequestingFocus)
{
    m_curntlyFocusedFilesCoord = filesCoordRequestingFocus;
    if(auto locked = m_curntlyFocusedFilesCoord.lock())
    {
        QDir newLabel( locked->getCurRootPath() );
        emitLabelChanged(newLabel);
    }
}

void WindowCoordinator::rootChanged(QDir newPath)
{
    this->saveSettings();
    emitLabelChanged(newPath);
}
