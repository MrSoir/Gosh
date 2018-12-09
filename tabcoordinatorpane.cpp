#include "tabcoordinatorpane.h"

TabCoordinatorPane::TabCoordinatorPane(std::weak_ptr<TabCoordinator> tabCoordinator,
                                       int activeTabId,
                                       QVector<QDir> tabLabels,
                                       QWidget* parent)
    : QWidget(parent),
      m_curActiveTabId(activeTabId),
      m_tabLabels(tabLabels),
      m_tabCoordinator(tabCoordinator)
{
    revalidate();
}

TabCoordinatorPane::~TabCoordinatorPane()
{
    m_tabCoordinator.reset();

    m_mainLayout = nullptr;
    deleteMainLayout();
}

void TabCoordinatorPane::setTabCoordinator(std::weak_ptr<TabCoordinator> tabCoordinator)
{
    m_tabCoordinator = tabCoordinator;
}

void TabCoordinatorPane::revalidate()
{
    revalidateMainWidget();
}

void TabCoordinatorPane::updateTabLabels(QVector<QDir> tabLabels)
{
    m_tabLabels = tabLabels;

    emit updateTabLabelsSGNL(tabLabels);
}

void TabCoordinatorPane::activeTabIdChanged(int id)
{
    if(id == m_curActiveTabId)
        return;

    m_curActiveTabId = id;

    revalidateMainWidget();

    emit activeTabIdChangedSGNL(id);
}
void TabCoordinatorPane::revalidateMainWidget()
{
    clearLayout();
    setTabBar();

    if(auto locked = m_tabCoordinator.lock())
    {
        QWidget* mainWidget = locked->getCenterWidgetToDisplay();
        if( mainWidget != nullptr )
            m_mainLayout->addWidget( mainWidget );
    }
}

void TabCoordinatorPane::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_F11){
        emit setFullScreen();
    }
}

void TabCoordinatorPane::clearLayout()
{
    if(m_tabPane != nullptr)
    {
        m_tabPaneScrollOffsetX = m_tabPane->getScrollOffsetX();
    }
    if(auto locked = m_tabCoordinator.lock())
    {
        locked->resetCenterWidgetToDisplay();
    }

    deleteMainLayout();

    m_mainLayout = new QVBoxLayout();
    this->setLayout(m_mainLayout);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
}

void TabCoordinatorPane::deleteMainLayout()
{
    if(this->layout())
        StaticFunctions::clearLayout_and_DeleteContainingWidgets(this->layout(), true);

    QLayout* oldLayout = this->layout();
    delete oldLayout;
}


void TabCoordinatorPane::setTabBar()
{
    m_tabPane = new TabSelectorPane( m_curActiveTabId, m_tabLabels, m_tabPaneScrollOffsetX );
    connect(m_tabPane, &TabSelectorPane::addClicked, this, &TabCoordinatorPane::tabAddClicked, Qt::QueuedConnection);
    connect(m_tabPane, &TabSelectorPane::tabClicked, this, &TabCoordinatorPane::tabOkClicked, Qt::QueuedConnection);
    connect(m_tabPane, &TabSelectorPane::tabCloseClicked, this, &TabCoordinatorPane::tabCloseClicked, Qt::QueuedConnection);

    connect(this, &TabCoordinatorPane::updateTabLabelsSGNL, m_tabPane, &TabSelectorPane::setTabs, Qt::QueuedConnection);
    connect(this, &TabCoordinatorPane::activeTabIdChangedSGNL, m_tabPane, &TabSelectorPane::setActiveTabId, Qt::QueuedConnection);
    connect(this, &TabCoordinatorPane::revalidateTabSelectorPane, m_tabPane, &TabSelectorPane::revalidate, Qt::QueuedConnection);

    m_mainLayout->addWidget(m_tabPane);
}
