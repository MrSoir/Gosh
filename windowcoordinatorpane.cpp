#include "windowcoordinatorpane.h"

WindowCoordinatorPane::WindowCoordinatorPane(std::weak_ptr<WindowCoordinator> windowCoordinator,
                                             QList<QList<int>> splitterRatios,
                                             QWidget *parent)
    : QWidget(parent),
      m_windowCoordinator(windowCoordinator),
      m_splitterRatios(splitterRatios)
{
//    startup();
//    addWindow();
    revalidateLayout();
}

WindowCoordinatorPane::~WindowCoordinatorPane()
{
    clearSplitter();    

    this->setLayout(nullptr);

    m_toolBar = nullptr;
    m_vBox = nullptr;

    deleteMainLayout();

    m_windowCoordinator.reset();

    if(m_removeDialog != nullptr)
        m_removeDialog->deleteLater();
}


QSplitter* createSplitter(Qt::Orientation orientation){
    QSplitter* splitter = new QSplitter();
    splitter->setFixedWidth(1);
    splitter->setContentsMargins(0,0,0,0);
    splitter->setOrientation(orientation);
    return splitter;
}

QList<QList<int>> WindowCoordinatorPane::getSplitterRatios()
{
    QList<QList<int>> splitterRatios;
    for(int i=0; i < m_splitter.size(); ++i)
    {
        splitterRatios.append( m_splitter[i]->sizes() );
    }
    return splitterRatios;
}

void WindowCoordinatorPane::setSplitterRatios(QList<QList<int>> splitterRatios)
{
    for(int i=0; i < m_splitter.size() && i < splitterRatios.size(); ++i)
    {
        m_splitter[i]->setSizes(splitterRatios[i]);
    }
}

void WindowCoordinatorPane::clearSplitter()
{
    m_splitter.clear();
}

void WindowCoordinatorPane::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter* painter = new QPainter(this);
    painter->setBrush(QColor(255,255,255));
    painter->drawRect(QRect(0,0, this->width(), this->height()));
    QWidget::paintEvent(event);
}

void WindowCoordinatorPane::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
    // fullscreen wird jetzt vom TabCoordinator geregelt!
    if(event->key() == Qt::Key_F11)
    {
        emit requestFullScreen();
//        setFullScreen();
    }
}

void WindowCoordinatorPane::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if(m_removeDialog != nullptr)
    {
        int edge = std::min(this->width(), this->height()) * m_dialogSizeFactor;

        QSize newSize(edge, edge);

        QPoint newPos( (this->width()-m_removeDialog->width())  *0.5f,
                         (this->height()-m_removeDialog->height()) *0.5f);

        m_removeDialog->setPosition( newPos );

        m_removeDialog->setSize( newSize );
    }
    QWidget::resizeEvent(event);
}

QList<QList<int> > WindowCoordinatorPane::generateSplitterRatios()
{
    QList<QList<int>> splitterRatios;
    for(int i=0; i < m_splitter.size(); ++i)
    {
        if(m_splitter[i] != nullptr)
        {
            splitterRatios.append(m_splitter[i]->sizes());
        }
    }
    return splitterRatios;
}

void WindowCoordinatorPane::saveSplitterSizes()
{
    if(auto locked = m_windowCoordinator.lock())
        locked->setSplitterSizes( getSplitterRatios() );
}

void WindowCoordinatorPane::splitterRatiosChanged()
{
    qDebug() << "in WindowCoordinatorPane::splitterRatiosChanged";
    emit splitterRatiosChangedSGNL( generateSplitterRatios() );
}


void addLayoutToSplitter(QSplitter* splitter, QLayout* widget){
    QWidget* wrpr = new QWidget();
    wrpr->setLayout(widget);
    splitter->addWidget(wrpr);
}
void addWidgetToSplitter(QSplitter* splitter, QWidget* widget){
    QVBoxLayout* vBox = new QVBoxLayout();
    vBox->addWidget(widget);
    vBox->setContentsMargins(3,3,3,3);
    addLayoutToSplitter(splitter, vBox);
}
void WindowCoordinatorPane::revalidateLayout()
{
    resetMainLayout();

    setToolBar();

    clearSplitter();

    if(auto locked = m_windowCoordinator.lock())
    {
        int windowCount = locked->getWindowCount();
        Orientation::ORIENTATION orientation = locked->getOrientation();

        if(windowCount == 1){
            m_vBox->addLayout(locked->getWindowLayout(0));
        }else if(windowCount == 2){
            QSplitter* splitter;
            if(orientation == Orientation::ORIENTATION::HORIZONTAL){
                splitter = createSplitter(Qt::Vertical);
            }else{
                splitter = createSplitter(Qt::Horizontal);

            }
            addLayoutToSplitter(splitter, locked->getWindowLayout(0));
            addLayoutToSplitter(splitter, locked->getWindowLayout(1));

            m_splitter.push_back(splitter);

//            connect(splitter, &QSplitter::splitterMoved, this, &WindowCoordinatorPane::saveSplitterSizes);

            m_vBox->addWidget(splitter);
        }else if(windowCount == 3){
            QSplitter* hSplitter = createSplitter(Qt::Vertical);
            QSplitter* vSplitter = createSplitter(Qt::Horizontal);
            if(orientation == Orientation::ORIENTATION::VERTICAL){
                addLayoutToSplitter(hSplitter, locked->getWindowLayout(1));
                addLayoutToSplitter(hSplitter, locked->getWindowLayout(2));

                addLayoutToSplitter(vSplitter, locked->getWindowLayout(0));
                addWidgetToSplitter(vSplitter, hSplitter);
                m_vBox->addWidget(vSplitter);
            }else{
                addLayoutToSplitter(vSplitter, locked->getWindowLayout(1));
                addLayoutToSplitter(vSplitter, locked->getWindowLayout(2));

                addLayoutToSplitter(hSplitter, locked->getWindowLayout(0));
                addWidgetToSplitter(hSplitter, vSplitter);
                m_vBox->addWidget(hSplitter);
            }

            m_splitter.push_back(hSplitter);
            m_splitter.push_back(vSplitter);

//            connect(hSplitter, &QSplitter::splitterMoved, this, &WindowCoordinatorPane::saveSplitterSizes);
//            connect(vSplitter, &QSplitter::splitterMoved, this, &WindowCoordinatorPane::saveSplitterSizes);
        }else if (windowCount == 4){
            QSplitter* vSplitter = createSplitter(Qt::Horizontal);
            QSplitter* hSplitter1 = createSplitter(Qt::Vertical);
            QSplitter* hSplitter2 = createSplitter(Qt::Vertical);

            addLayoutToSplitter(hSplitter1, locked->getWindowLayout(0));
            addLayoutToSplitter(hSplitter1, locked->getWindowLayout(1));

            addLayoutToSplitter(hSplitter2, locked->getWindowLayout(2));
            addLayoutToSplitter(hSplitter2, locked->getWindowLayout(3));

            addWidgetToSplitter(vSplitter, hSplitter1);
            addWidgetToSplitter(vSplitter, hSplitter2);

            m_splitter.push_back(vSplitter);
            m_splitter.push_back(hSplitter1);
            m_splitter.push_back(hSplitter2);

//            connect(vSplitter,  &QSplitter::splitterMoved, this, &WindowCoordinatorPane::saveSplitterSizes);
//            connect(hSplitter1, &QSplitter::splitterMoved, this, &WindowCoordinatorPane::saveSplitterSizes);
//            connect(hSplitter2, &QSplitter::splitterMoved, this, &WindowCoordinatorPane::saveSplitterSizes);

            m_vBox->addWidget(vSplitter);
        }
    }

    if(m_splitterRatios.size() == m_splitter.size())
    {
        for(int i=0; i < m_splitterRatios.size() && i < m_splitter.size(); ++i)
        {
            if(m_splitter[i] != nullptr)
            {
                m_splitter[i]->setSizes(m_splitterRatios[i]);
            }
        }
    }
    for(int i=0; i < m_splitter.size(); ++i)
    {
        connect(m_splitter[i], &QSplitter::splitterMoved, this, &WindowCoordinatorPane::splitterRatiosChanged);
    }
}

void WindowCoordinatorPane::resetMainLayout()
{
    deleteMainLayout();

    m_vBox = new QVBoxLayout();
    this->setLayout(m_vBox);
    m_vBox->setContentsMargins(2, 2, 2, 2);
    m_vBox->setSpacing(0);
}

void WindowCoordinatorPane::deleteMainLayout()
{
    if(this->layout())
        StaticFunctions::clearLayout_and_DeleteContainingWidgets(this->layout(), true);

    QLayout* oldLayout = this->layout();
    delete oldLayout;
}

//void WindowCoordinatorPane::requestFullScreen()
//{
//    emit requestFullScreen();
////    if(auto locked = m_windowCoordinator.lock())
////        locked->setFullScreen();
//}

void WindowCoordinatorPane::setToolBar()
{
    m_toolBar = new QHBoxLayout();
    m_toolBar->setAlignment(Qt::AlignLeft);
    m_toolBar->setSpacing(10);

    int windowCount = 0;
    Orientation::ORIENTATION orientation = Orientation::ORIENTATION::VERTICAL;
    if(auto locked = m_windowCoordinator.lock())
    {
        windowCount = locked->getWindowCount();
        orientation = locked->getOrientation();
    }


    if(windowCount < 4){
        QPushButton* addBtn = StaticFunctions::PixmapButtonFromPicsResources("frame_add_scld.png", &TOOLBAR_ICON_SIZE);
        if(addBtn != nullptr){
            connect(addBtn, &QPushButton::clicked, this, &WindowCoordinatorPane::addWindow);
            m_toolBar->addWidget(addBtn);
        }
    }

    if(windowCount > 1){
        QPushButton* closeBtn = StaticFunctions::PixmapButtonFromPicsResources("frame_remove_scld.png", &TOOLBAR_ICON_SIZE);
        if( closeBtn != nullptr){
            QObject::connect(closeBtn, &QPushButton::clicked, this, [=](){
                auto caller = [=](int i){
                    emit removeWindow(i);
                };
                int edge = int(float(std::min(this->width(), this->height()) * m_dialogSizeFactor));
                QSize size(edge, edge);
                QPoint position( (this->width()-size.width())  *0.5f,
                                 (this->height()-size.height())*0.5f);
                m_removeDialog = new RemoveWindowDialog(windowCount, caller, orientation,
                                        size, position,
                                        this);
                connect(m_removeDialog, &RemoveWindowDialog::finished, this, [=](){m_removeDialog = nullptr;});
                connect(m_removeDialog, &RemoveWindowDialog::destroyed, this, [=](){m_removeDialog = nullptr;});
                m_removeDialog->show();
                m_removeDialog->raise();
                m_removeDialog->activateWindow();
            }
            );
            m_toolBar->addWidget(closeBtn);
        }

        if(windowCount < 4){
            QPushButton* orientationBtn = StaticFunctions::PixmapButtonFromPicsResources("compass_BD_scld.png", &TOOLBAR_ICON_SIZE);
            if(orientationBtn != nullptr){
                connect(orientationBtn, &QPushButton::clicked, this, &WindowCoordinatorPane::changeOrientation);
                m_toolBar->addWidget(orientationBtn);
                }
        }
    }

    QPushButton* fullscreenBtn = StaticFunctions::PixmapButtonFromPicsResources("fullscreen_scld.png", &TOOLBAR_ICON_SIZE);
    if(fullscreenBtn != nullptr){
        connect(fullscreenBtn, &QPushButton::clicked, this, &WindowCoordinatorPane::requestFullScreen);
        m_toolBar->addWidget(fullscreenBtn);
    }

    QSize infoSize(20,20);
    QPushButton* helpBtn = StaticFunctions::PixmapButtonFromPicsResources("help.png", &infoSize);
    if( helpBtn != nullptr){
        QObject::connect(helpBtn, &QPushButton::clicked,[=](){
                HelpDialog* helpDiallog = new HelpDialog(QSize(600,600));
                helpDiallog->exec();
            }
        );
        m_toolBar->addWidget(helpBtn);
    }
    QPushButton* infoBtn = StaticFunctions::PixmapButtonFromPicsResources("info_scld.png", &infoSize);
    if( infoBtn != nullptr){
        QObject::connect(infoBtn, &QPushButton::clicked,[=](){
                InfoDialog* infoDialog = new InfoDialog(QSize(600,600));
                infoDialog->exec();
            }
        );
        m_toolBar->addWidget(infoBtn);
    }

    m_vBox->addLayout(m_toolBar);
}
