#include "openwithdialog.h"

OpenWithDialog::OpenWithDialog(QFileInfo fileToOpen, QSize size, QPoint pos, QWidget *parent)
    : QDialog(parent),
      m_fileToOpen(fileToOpen)
{
    setMouseTracking(true);


    StaticFunctions::setIconToWidget(this);

    this->setModal(true);
    this->setWindowFlags(Qt::FramelessWindowHint);

    this->setMaximumWidth (size.width());
    this->setMaximumHeight(size.height());
    this->setMinimumWidth (size.width());
    this->setMinimumHeight(size.height());

    this->move(pos.x(), pos.y());

    QString mimeType = FileHandler::getMimeOf(fileToOpen);
    m_programHandlers = FileHandler::getProgramsThatCanHandleMimeType(mimeType.toStdString());

    OpenWithGraphicsView* graphicsView = new OpenWithGraphicsView(fileToOpen, m_programHandlers);

    connect(graphicsView, &OpenWithGraphicsView::clicked, this, &OpenWithDialog::selected);
    QVBoxLayout* vBox = new QVBoxLayout();
    vBox->addWidget(graphicsView);

//    QPushButton* okBtn = new QPushButton("ok");
//    connect(okBtn, &QPushButton::clicked, this, &OpenWithDialog::onOkClicked);
//    connect(okBtn, &QPushButton::clicked, this, &OpenWithDialog::close);
//    connect(okBtn, &QPushButton::clicked, this, &OpenWithDialog::deleteLater);

    QPushButton* cancelBtn = new QPushButton("cancel");
    connect(cancelBtn, &QPushButton::clicked, this, &OpenWithDialog::close);
    connect(cancelBtn, &QPushButton::clicked, this, &OpenWithDialog::deleteLater);

    QHBoxLayout* btnLyt = new QHBoxLayout();
//    btnLyt->addWidget(okBtn);
    btnLyt->addWidget(cancelBtn);

    vBox->addLayout(btnLyt);

    this->setLayout(vBox);

    setMarginsOfLayout(btnLyt);
    setMarginsOfLayout(vBox);


    this->exec();
}

OpenWithDialog::~OpenWithDialog()
{
}

void OpenWithDialog::onOkClicked()
{
    if(m_selectedProgramId < m_programHandlers.size())
    {
        QString executeionString = QString::fromStdString(m_programHandlers[m_selectedProgramId].execution);
        qDebug() << "selected: exec: " << executeionString;
        StaticFunctions::openFileWithAppcliation(m_fileToOpen, executeionString);
    }
    this->close();
    this->deleteLater();
}

void OpenWithDialog::selected(int id)
{
    m_selectedProgramId = id;
    onOkClicked();

}

void OpenWithDialog::setMarginsOfLayout(QLayout* layout, int padding)
{
    layout->setSpacing(0);
    layout->setContentsMargins(padding, padding, padding, padding);
}
