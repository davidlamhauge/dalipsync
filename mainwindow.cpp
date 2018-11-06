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
    mCharName = "DIAL";

    mFps = 25;
    mDuration = 0;
    mFramesInAudio = 0;
    ui->tableWidget->setRowHeight(0, 70);
    ui->tableWidget->setRowHeight(1, 30);

    player = new QMediaPlayer;
    connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::setDuration);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionLoad_Audio, &QAction::triggered, this, &MainWindow::load);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::open);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save);
    connect(ui->actionSave_as, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->tableWidget, &QTableWidget::itemEntered, this, &MainWindow::playPhoneme);
    connect(ui->tableWidget, &QTableWidget::itemPressed, this, &MainWindow::playPhoneme);
    connect(ui->leAddName, &QLineEdit::textChanged, this, &MainWindow::setCharName);
    loadPosition();
}

void MainWindow::load()
{
    loadPosition();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Audio File"),
                                    "", tr("Audio files (*.ogg *.wav *.mp3)"));
    if (fileName.isEmpty()) { return; }
    mFileName = fileName;
    player->setMedia(QUrl::fromLocalFile(fileName));
    setDuration(player->duration());
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Lipsyns File"),
                                    "", tr("Lipsync files (*.lip2d)"));
    if (fileName.isEmpty()) { return; }
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) { return; }
    qDebug() << fileName;
    QTextStream in(&file);
    qDebug() << "check 1";
    QString tmp = in.readLine();
    QStringList sl = tmp.split(" ");
    mCharName = sl.at(0);
    ui->leAddName->setText(mCharName);
    mFps = sl.at(1).toInt();
    ui->sBoxFps->setValue(mFps);
    if (sl.size() > 2)
        mFramesInAudio = sl.at(2).toInt();
    if (sl.size() > 3)
        mFileName = sl.at(3);
    qDebug() << "check 2";
    setDuration((mFramesInAudio * 1000) / mFps);
    tmp.clear();
    sl.clear();
    qDebug() << "check 3";
    while (!in.atEnd()) {
        tmp = in.readLine();
        sl = tmp.split(" ");
        mTabWidgetItem = new QTableWidgetItem(sl.at(1));
        ui->tableWidget->setItem(1, sl.at(0).toInt() - 1, mTabWidgetItem);
        sl.clear();
    }
    file.close();
    player->setMedia(QUrl::fromLocalFile(fileName));
    qDebug() << player->errorString() << " error";
}

void MainWindow::save()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Lipsyns File"),
                                    "", tr("Lipsync files (*.lip2d)"));
    if (fileName.isEmpty()) { return; }
    if (!fileName.endsWith(".lip2d"))
        fileName += ".lip2d";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) { return; }
    QTextStream out(&file);
    // FIRST line: CHARACTER_NAME FPS SIZE_IN_FRAMES FILENAME
    out << mCharName << " " << mFps << " " << ui->tableWidget->columnCount()
        << " " << mFileName << '\n';
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
    QApplication::quit();
}

void MainWindow::setFps(int i)
{
    mFps = i;
}

void MainWindow::setCharName(QString name)
{
    mCharName = name;
    setWindowTitle("dalipsync " + name);
}

void MainWindow::setDuration(qint64 qint)
{
    mDuration = qint;
    qDebug() << mDuration << " ms";
    mFramesInAudio = qCeil((mDuration * mFps) / 1000);
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
    mPosition = (twItem->column() + 1) * mFps;
    player->setPosition(mPosition);
    player->play();
    QTimer::singleShot((1000 * 3) / mFps, this, SLOT(stopPlayPhoneme()));
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
            case Qt::Key_Delete: // Delete
                mTabWidgetItem = new QTableWidgetItem("");
                ui->tableWidget->setItem(1, ui->tableWidget->currentColumn(), mTabWidgetItem);
                break;
            case Qt::Key_Space:
                qDebug() << "Space pressed";
                if (player->state() == QMediaPlayer::StoppedState)
                    player->play();
                else if (player->state() == QMediaPlayer::PlayingState)
                    player->pause();
                else if (player->state() == QMediaPlayer::PausedState)
                    player->play();
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
    QRect myrect = settings.value("position").toRect();
    if (myrect.width() < 100)
    {
        myrect.setCoords(100, 50, 1000, 400 );
    }
    setGeometry(myrect);
    settings.endGroup();
    qDebug() << myrect.width();
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    qDebug() << "Keypress: " << e->text();
}

