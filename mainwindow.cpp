#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QSettings>
#include <QtMath>
#include <QHoverEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    mFileName.clear();
    mDuration = 0;
    mFramesInAudio = 0;
    ui->tableWidget->setRowHeight(0, 70);
    ui->tableWidget->setRowHeight(1, 30);

    player = new QMediaPlayer;
    connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::setDuration);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionLoad_Audio, &QAction::triggered, this, &MainWindow::load);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save);
    connect(ui->actionSave_as, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->tableWidget, &QTableWidget::itemEntered, this, &MainWindow::playPhoneme);
    connect(ui->tableWidget, &QTableWidget::itemPressed, this, &MainWindow::playPhoneme);
    loadPosition();
}

void MainWindow::load()
{
    loadPosition();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Audio File"),
                                    "", tr("Audio files (*.ogg *.wav *.mp3)"));
    if (fileName.isEmpty()) { return; }
    mFileName = fileName.right(fileName.length() - fileName.lastIndexOf('/') - 1);
    player->setMedia(QUrl::fromLocalFile(fileName));
}

void MainWindow::open()
{

}

void MainWindow::save()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Lipsyns File"),
                                    "", tr("Lipsync files (*.lip2d)"));
    if (fileName.isEmpty()) { return; }
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) { return; }
    QTextStream out(&file);
    // FIRST line: CHARACTER_NAME FPS SIZE_IN_FRAMES FILENAME
    out << "DIAL 25 " << ui->tableWidget->columnCount() << " " << mFileName << '\n';
    for (int i = 0; i < ui->tableWidget->columnCount(); i++)
    {
        if (!ui->tableWidget->item(1, i)->text().isEmpty())
            out << i + 1 << " " << ui->tableWidget->item(1, i)->text() << '\n';
    }
    file.close();
    savePosition();
}

void MainWindow::saveAs()
{
    player->play();
}

void MainWindow::close()
{
    close();
}

void MainWindow::setDuration(qint64 qint)
{
    mDuration = qint;
    qDebug() << mDuration << " ms";
    mFramesInAudio = qCeil((mDuration * 25) / 1000);
    qDebug() << mFramesInAudio << " frames";
    ui->tableWidget->setColumnCount(mFramesInAudio + 1);
    for (int i = 0; i <= mFramesInAudio; i++)
    {
        ui->tableWidget->setColumnWidth(i, 30);
        mTabWidgetItem = new QTableWidgetItem("");
        mTabWidgetItem->setBackgroundColor(Qt::yellow);
        ui->tableWidget->setItem(0, i, mTabWidgetItem);
        mTabWidgetItem = new QTableWidgetItem("");
        ui->tableWidget->setItem(1, i, mTabWidgetItem);
    }
}

void MainWindow::playPhoneme(QTableWidgetItem *twItem)
{
    mPosition = (twItem->column() + 1) * 25;
    player->setPosition(mPosition);
    player->play();
    QTimer::singleShot((1000 * 3) / 25, this, SLOT(stopPlayPhoneme()));
}

void MainWindow::stopPlayPhoneme()
{
    player->stop();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        if (watched == ui->tableWidget)
        {
            QKeyEvent * ke = static_cast<QKeyEvent *>(event);
            switch (ke->key()) {
            case 8: // Backspace
                mTabWidgetItem = new QTableWidgetItem("");
                ui->tableWidget->setItem(1, ui->tableWidget->currentColumn(), mTabWidgetItem);
                break;
            default:
                QString s = ke->text();
                mTabWidgetItem = new QTableWidgetItem(s.toUpper());
                ui->tableWidget->setItem(1, ui->tableWidget->currentColumn(), mTabWidgetItem);
                break;
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

void MainWindow::savePosition()
{
    QSettings settings("dalanima", "dalipsync");
    settings.beginGroup("MainWindow");
    settings.setValue("position", this->geometry());
    settings.endGroup();
}

void MainWindow::loadPosition()
{
    QSettings settings("dalanima", "dalipsync");
    settings.beginGroup("MainWindow");
    QRect myrect = settings.value("position", QRect(0, 0, 600, 300)).toRect();
    setGeometry(myrect);
    settings.endGroup();
    qDebug() << myrect.width();
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    qDebug() << "Keypress: " << e->text();
}

