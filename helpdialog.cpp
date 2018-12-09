#include "helpdialog.h"


HelpDialog::HelpDialog(QSize size, QPoint pos, QWidget *parent)
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
//    int colId = 0;

    addLabelToGridPane(QString("Shortcuts"), rowId++, 0, mainLayout, 1, 2);

    mainLayout->addWidget(genSeparator(), rowId++, 0, 1, 2);

    addLabelToGridPane(QString("Ctrl + F4"), rowId,   0, mainLayout);
    addLabelToGridPane(QString("close current tab"), rowId++, 1, mainLayout);

    addLabelToGridPane(QString("Ctrl + W"), rowId,   0, mainLayout);
    addLabelToGridPane(QString("open selected folders in new tab"), rowId++, 1, mainLayout);

    addLabelToGridPane(QString("Ctrl + A"), rowId,   0, mainLayout);
    addLabelToGridPane(QString("select entire content"), rowId++, 1, mainLayout);

    addLabelToGridPane(QString("Ctrl + F"), rowId,   0, mainLayout);
    addLabelToGridPane(QString("initiate search mode"), rowId++, 1, mainLayout);

    addLabelToGridPane(QString("Ctrl + M"), rowId,   0, mainLayout);
    addLabelToGridPane(QString("create new File"), rowId++, 1, mainLayout);

    addLabelToGridPane(QString("Ctrl + N"), rowId,   0, mainLayout);
    addLabelToGridPane(QString("create new Folder"), rowId++, 1, mainLayout);

    addLabelToGridPane(QString("Ctrl + R"), rowId,   0, mainLayout);
    addLabelToGridPane(QString("rename file/folder"), rowId++, 1, mainLayout);

    addLabelToGridPane(QString("Ctrl + T"), rowId,   0, mainLayout);
    addLabelToGridPane(QString("open terminal"), rowId++, 1, mainLayout);

    addLabelToGridPane(QString("Ctrl + H"), rowId,   0, mainLayout);
    addLabelToGridPane(QString("show/hide hidden files"), rowId++, 1, mainLayout);

    mainLayout->addWidget(genSeparator(), rowId++, 0, 1, 2);

    addLabelToGridPane(QString("Ctrl + C"), rowId,   0, mainLayout);
    addLabelToGridPane(QString("copy selected content"), rowId++, 1, mainLayout);

    addLabelToGridPane(QString("Ctrl + X"), rowId,   0, mainLayout);
    addLabelToGridPane(QString("cut selected content"), rowId++, 1, mainLayout);

    addLabelToGridPane(QString("Ctrl + V"), rowId,   0, mainLayout);
    addLabelToGridPane(QString("paste selected content"), rowId++, 1, mainLayout);


    QPushButton* closeBtn = new QPushButton("close");
    connect(closeBtn, &QPushButton::clicked, this, &HelpDialog::close);
    connect(closeBtn, &QPushButton::clicked, this, &HelpDialog::deleteLater);
    vBox->addWidget(closeBtn);
}

HelpDialog::~HelpDialog()
{

}

void HelpDialog::addWidgetToGridPane(QWidget *widget,
                                     int row, int col,
                                     QGridLayout *layout)
{
    layout->addWidget(widget, row, col);
}

void HelpDialog::addLabelToGridPane(const QString &txt,
                                  int row, int col,
                                  QGridLayout *lay,
                                  int rowSpan, int colSpan)
{
    QLabel* label = new QLabel(this);
    label->setText( txt );
    label->setAlignment(Qt::AlignHCenter);
    lay->addWidget(label, row, col, rowSpan, colSpan);
}

QFrame* HelpDialog::genSeparator(int horizontal)
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

void HelpDialog::setMarginsOfLayout(QLayout* layout, int padding)
{
    layout->setSpacing(0);
    layout->setContentsMargins(padding, padding, padding, padding);
}

