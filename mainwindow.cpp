#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QSettings>
#include <QtMath>
#include <QHoverEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
    ui->tableWidget->setRowHeight(0, 50);
    ui->tableWidget->setRowHeight(1, 20);

    player = new QMediaPlayer;
    connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::setDuration);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionLoad_Audio, &QAction::triggered, this, &MainWindow::load);
    connect(ui->actionSave_as, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->tableWidget, &QTableWidget::itemEntered, this, &MainWindow::playPhoneme);
    loadPosition();
 //   connect(ui->tableWidget, &QTableWidget::entered, this, &MainWindow::mouseMoveEvent);
}

void MainWindow::load()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Audio File"),
                                    "", tr("Audio files (*.ogg *.wav *.mp3)"));
    if (fileName.isEmpty()) { return; }
    mFileName = fileName.right(fileName.length() - fileName.lastIndexOf('/') - 1);
    qDebug() << fileName << " " << mFileName;
    player->setMedia(QUrl::fromLocalFile(fileName));
}

void MainWindow::open()
{

}

void MainWindow::save()
{

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
        ui->tableWidget->setColumnWidth(i, 20);
        mTabWidgetItem = new QTableWidgetItem("");
        mTabWidgetItem->setBackgroundColor(Qt::yellow);
        ui->tableWidget->setItem(0, i, mTabWidgetItem);
    }
}

void MainWindow::playPhoneme(QTableWidgetItem *twItem)
{
    int start = (twItem->column() + 1) * 25;
    player->setPosition(start);
    player->play();
    QTimer::singleShot((1000 * 3) / 25, this, SLOT(stopPlayPhoneme()));
}

void MainWindow::stopPlayPhoneme()
{
    player->stop();
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
    QRect myrect = settings.value("position", 0).toRect();
    setGeometry(myrect);
    settings.endGroup();
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    qDebug() << e->text();
}

