#include "tabcoordinator.h"

TabCoordinator::TabCoordinator(QObject* parent)
    : QObject(parent)
{
    addTab();
}

TabCoordinator::~TabCoordinator()
{
    for(int i=0; i < m_windows.size(); ++i)
        m_windows[i].reset();
    m_windows.clear();

    m_currentlyDisplWindow.reset();

    m_fullScreenCaller = nullptr;
}

void TabCoordinator::setFullScreenCaller(std::shared_ptr<DynamicFunctionCaller<QString, std::function<void ()> > > fullScreenCaller)
{
    m_fullScreenCaller = fullScreenCaller;
    for(int i=0; i < m_windows.size(); ++i)
    {
        m_windows[i]->setFullScreenCaller(m_fullScreenCaller);
    }
}


QWidget* TabCoordinator::getCenterWidgetToDisplay()
{
    QWidget* widget = nullptr;
    if(auto locked = m_currentlyDisplWindow.lock())
    {
        return locked->getWindowCoordinatorPane();
    }
    return widget;
}
QWidget* TabCoordinator::resetCenterWidgetToDisplay()
{
    if(auto locked = m_currentlyDisplWindow.lock())
    {
        locked->resetWindowCoordinatorPane();
    }
}

TabCoordinatorPane* TabCoordinator::getTabCoordinatorPane()
{
    QVector<QDir> labels = generateLabels();
    TabCoordinatorPane* tabCoordinatorPane = new TabCoordinatorPane( m_self, m_curWindowId, labels );

    QObject::connect(tabCoordinatorPane, &TabCoordinatorPane::tabOkClicked,    this, &TabCoordinator::setTab, Qt::QueuedConnection);
    QObject::connect(tabCoordinatorPane, &TabCoordinatorPane::tabCloseClicked, this, &TabCoordinator::removeTab, Qt::QueuedConnection);
    QObject::connect(tabCoordinatorPane, &TabCoordinatorPane::tabAddClicked,   this, &TabCoordinator::addTab, Qt::QueuedConnection);

    QObject::connect(this, &TabCoordinator::revalidateTabCoordinatorPane, tabCoordinatorPane, &TabCoordinatorPane::revalidate, Qt::QueuedConnection);
    QObject::connect(this, &TabCoordinator::labelsChanged,                tabCoordinatorPane, &TabCoordinatorPane::updateTabLabels, Qt::QueuedConnection);
    QObject::connect(this, &TabCoordinator::activeTabIdChanged,           tabCoordinatorPane, &TabCoordinatorPane::activeTabIdChanged, Qt::QueuedConnection);

    return tabCoordinatorPane;
}

void TabCoordinator::setSelf(std::weak_ptr<TabCoordinator> self)
{
    m_self = self;
}

void TabCoordinator::setFullScreen()
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

void TabCoordinator::addTab()
{
    addTabHelper(nullptr);
}



void TabCoordinator::setTab(int id)
{
    qDebug() << "in TabCoordinator.setTab: m_tabs.size: " << m_windows.size()
             << "   id: " << id << "    m_curWindowId: " << m_curWindowId;

    if(id == m_curWindowId)
        return;

//    der WindowCoordinatorPane im WindowCoordinator wird gleich durch removeWindowFromLayout()
//    geloescht. Damit der WindowCoordinator nicht weiter funktionen auf das geloeschte objekt
//    aufruft, muss diese Referenz auf nullptr durch resetWindowCoordinatorPane()
//    gesetzt werden. Sonst schmiert das programm ab -> haesslich, aber da
//    qt direkt mit pointern arbeitet und nicht mit smartpointern bleibt nichts anderes uebrig:
    if(auto locked = m_currentlyDisplWindow.lock())
        locked->resetWindowCoordinatorPane();

    m_currentlyDisplWindow = std::weak_ptr<WindowCoordinator>(m_windows[id]);
    m_curWindowId = id;

    revalidateLabels();
    emit activeTabIdChanged(m_curWindowId);
}

void TabCoordinator::removeTab(int id)
{
    // wenn nur ein tab existiert, darf dieser tab natuerlich nicht geloescht werden:
    if(m_windows.size() <= 1)
        return;

    if(id > -1 && id < m_windows.size())
    {
        if(m_windows[id] && id == m_curWindowId)
        {
            m_windows[id]->resetWindowCoordinatorPane();
        }
        m_windows[id].reset();
        m_windows.removeAt(id);

        if(m_curWindowId >= id)
        {
            if(m_curWindowId > id)
            {
                int newActiveWindowId = m_curWindowId-1;
                m_curWindowId = -1; // damit auf jeden fall das setTab durchgefuehrt wird und nicht gleich am anfang abwuergt
                setTab(newActiveWindowId);


            }else // m_curWindowId == id
            {
                int newActiveWindowId = (id >= m_windows.size()) ? m_windows.size()-1 : id;
                m_curWindowId = -1; // damit auf jeden fall das setTab durchgefuehrt wird und nicht gleich am anfang abwuergt
                setTab(newActiveWindowId);
            }
        }
    }
}

void TabCoordinator::revalidateLabels()
{
    QVector<QDir> newLabels = generateLabels();
    emit labelsChanged( newLabels );
}

QVector<QDir> TabCoordinator::generateLabels()
{
    QVector<QDir> labels;
    for(int i=0; i < m_windows.size(); ++i)
    {
        if(m_windows[i])
        {
            labels.append( QDir(m_windows[i]->getCurrentFocusedDir()) );
        }
    }
    return labels;
}

void TabCoordinator::openFoldersInNewTab(QVector<QDir> foldersToOpen)
{
    addTabHelper( &foldersToOpen );
}

void TabCoordinator::addTabHelper(QVector<QDir>* initPaths)
{
    WindowCoordinator* windowPtr = new WindowCoordinator(initPaths);
    connect(windowPtr, &WindowCoordinator::closeTab, this, &TabCoordinator::closeActiveTab, Qt::QueuedConnection);
    connect(windowPtr, &WindowCoordinator::openFoldersInNewTab, this, &TabCoordinator::openFoldersInNewTab, Qt::QueuedConnection);

    std::shared_ptr<WindowCoordinator> newWindow = std::shared_ptr<WindowCoordinator>( windowPtr );

    newWindow->setSelf( std::weak_ptr<WindowCoordinator>(newWindow) );
    newWindow->setOnLabelChanged([=](QDir newLabel){
        Q_UNUSED(newLabel);
        revalidateLabels();
    });
//    newWindow->setOnOpenFoldersInNewTab([=](QVector<QDir> foldersToOpen){
//        openFoldersInNewTab(foldersToOpen);
//    });
//    newWindow->setOnCloseTab([=](){
//        closeActiveTab();
//    });

    m_windows.push_back(newWindow);

    setTab( m_windows.size()-1 );
}

void TabCoordinator::closeActiveTab()
{
    if(m_windows.size() > 1)
    {
        removeTab(m_curWindowId);
    }
}
