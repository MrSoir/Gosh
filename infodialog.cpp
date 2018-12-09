#include "infodialog.h"


InfoDialog::InfoDialog(QSize size, QPoint pos, QWidget *parent)
    : QDialog(parent)
{
    setMouseTracking(true);

    StaticFunctions::setIconToWidget(this);

    this->setModal(true);
//    this->setWindowFlags(Qt::FramelessWindowHint);

    this->setMaximumWidth(size.width());
    this->setMaximumHeight(size.height());

    this->move(pos.x(), pos.y());


    QVBoxLayout* vBox = new QVBoxLayout(this);
    setMarginsOfLayout(vBox);

    QScrollArea* scrollPane = new QScrollArea();
    scrollPane->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

//    wrapper-widget/helper-widget, das den content fuer das scrollpane traegt:
    QWidget* widget = new QWidget(scrollPane);
    StaticFunctions::setBackgroundColorToWidget(widget);

    QGridLayout* mainLayout = new QGridLayout(widget);
    setMarginsOfLayout(mainLayout, 5);
    widget->setLayout(mainLayout);
    widget->setMinimumHeight(size.height());
    widget->setMinimumWidth(size.width());
    scrollPane->setWidget(widget);

    vBox->addWidget(scrollPane);
    this->setLayout(vBox);

    mainLayout->setColumnMinimumWidth(1, this->width()-100);

    int rowId = 0;
    int colId = 0;

    QLabel* headingLbl = new QLabel(this);
    headingLbl->setText(QString("Credits"));
    headingLbl->setAlignment(Qt::AlignHCenter);
    mainLayout->addWidget(headingLbl, rowId++,0, 1, 2);

    mainLayout->addWidget(genSeparator(true), rowId++, 0, 1, 2);

    QLabel* goshLicenseLabel = new QLabel(this);
    goshLicenseLabel->setText(getGoshLicenseText());
    goshLicenseLabel->setAlignment(Qt::AlignHCenter);
    mainLayout->addWidget(goshLicenseLabel, rowId++,0, 1,2);

    mainLayout->addWidget(genSeparator(true), rowId++, 0, 1, 2);

    addPixmapToGridPane(QString("frame_add_scld.png"),      rowId++,colId, mainLayout);
    addPixmapToGridPane(QString("frame_remove_scld.png"),   rowId++,colId, mainLayout);
    addPixmapToGridPane(QString("compass_BD_scld.png"),     rowId++,colId, mainLayout);
    addPixmapToGridPane(QString("fullscreen_scld.png"),     rowId++,colId, mainLayout);
    addPixmapToGridPane(QString("root_scld.png"),           rowId++,colId, mainLayout);
    addPixmapToGridPane(QString("terminal.png"),            rowId++,colId, mainLayout);
    addPixmapToGridPane(QString("empty_fold_icon.png"),     rowId++,colId, mainLayout);
    addPixmapToGridPane(QString("incognito.png"),           rowId++,colId, mainLayout);

    rowId = 4;
    colId = 1;

    setLicenseTextToButton(QString("those-icons"),              rowId++, colId, mainLayout); // frame_add_scld
    setLicenseTextToButton(QString("those-icons"),              rowId++, colId, mainLayout); // frame_remove_scld
    setLicenseTextToButton(QString("Gregor Cresnar"),           rowId++, colId, mainLayout); // compass_BD_scld
    setLicenseTextToButton(QString("becris"),                   rowId++, colId, mainLayout); // fullscreen_scld
    setLicenseTextToButton(QString("http://www.freepik.com"),   rowId++, colId, mainLayout); // root_scld
    setLicenseTextToButton(QString("http://www.freepik.com"),   rowId++, colId, mainLayout); // terminal
    setLicenseTextToButton(QString("Elegant Themes"),           rowId++, colId, mainLayout); // empty_fold_icon
    setLicenseTextToButton(QString("http://www.freepik.com"),   rowId++, colId, mainLayout); // incognito



    QLabel* picturesLicenseLbl = new QLabel(this);
    picturesLicenseLbl->setText(getIconsLicenseText());
    picturesLicenseLbl->setAlignment(Qt::AlignHCenter);
    mainLayout->addWidget(picturesLicenseLbl, rowId++,0, 1,2);


    QPushButton* closeBtn = new QPushButton("close");
//    closeBtn->setMinimumWidth(300);
    connect(closeBtn, &QPushButton::clicked, this, &InfoDialog::close);
    connect(closeBtn, &QPushButton::clicked, this, &InfoDialog::deleteLater);
    vBox->addWidget(closeBtn);


//    for(int i=0; i < mainLayout->rowCount(); ++i)
//    {
//        mainLayout->setRowMinimumHeight(i, 100);
//    }
}

InfoDialog::~InfoDialog()
{

}

void InfoDialog::addPixmapToGridPane(const QString& pictureFileName,
                                         int row, int col,
                                         QGridLayout *layout)
{
    QPixmap pixmap(StaticFunctions::getPictureResourceFromFileName(pictureFileName));
    pixmap = pixmap.scaled(m_iconBtnSize);

    QLabel* lbl = new QLabel();
    lbl->setPixmap(pixmap);

    layout->addWidget(lbl, row, col);
}

void InfoDialog::setLicenseTextToButton(const QString &artist, int row, int col, QGridLayout *layout)
{
    QString txt = QString("designed by '%1' from www.flaticon.com.").arg(artist);
    QLabel* label = new QLabel(txt);
    layout->addWidget(label, row, col);
}

void InfoDialog::addWidgetToGridPane(QWidget *widget,
                                     int row, int col,
                                     QGridLayout *layout)
{
    layout->addWidget(widget, row, col);
}

QString InfoDialog::getGoshLicenseText()
{
    return QString("This Software was created using the Qt5 Framework lincesed under GNU LGPLv3.\n"
                   "For further notes read the Readme.txt in the installation folder.");
}

QString InfoDialog::getIconsLicenseText()
{
    return QString("All listed icons are from http:://www.flaticon.com\n"
                   "and licensed under the terms of 'Creative Commons BY 3.0'\n"
                   "(http://creativecommons.org/licenses/by/3.0/)");
}

QFrame* InfoDialog::genSeparator(int horizontal)
{
    QFrame* line = new QFrame();
    if(horizontal)
        line->setMaximumHeight(3);
    else
        line->setMaximumWidth(3);

    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

void InfoDialog::setMarginsOfLayout(QLayout* layout, int padding)
{
    layout->setSpacing(0);
    layout->setContentsMargins(padding, padding, padding, padding);
}
