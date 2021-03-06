#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QSettings>
#include <QtMath>
#include <QHoverEvent>
#include <QMessageBox>

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
    mInfoList.append("Dial 25 200 x");
    sl.clear();

    ui->tableWidget->setRowHeight(0, 70);
    ui->tableWidget->setRowHeight(1, 30);

    player = new QMediaPlayer;

    connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::setDuration);
    connect(ui->sliderVolume, &QSlider::valueChanged, this, &MainWindow::setvolume);
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Audio File"),
                                    "", tr("Audio files (*.ogg *.wav *.mp3)"));
    if (fileName.isEmpty()) { return; }
    mFileName = fileName;
    mInfoList.clear();
    mInfoList.append("DIAL 25 200 " + mFileName);
    player->setMedia(QUrl::fromLocalFile(fileName));
    savePosition();
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Lipsync File"),
                                    "", tr("Lipsync files (*.lip2d)"));
    if (fileName.isEmpty()) { return; }
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) { return; }
    QTextStream in(&file);
    QString tmp = in.readLine();
    sl = tmp.split(" ");
    if (sl.size() < 4)
    {
        int ret = QMessageBox::information(this,
                                           tr("Not enough data!"),
                                           tr("File lacks audio file information"),
                                           QMessageBox::Ok);
        Q_UNUSED(ret);
        file.close();
        return;
    }
    mFileName = sl.at(3);
    QFile f(mFileName);
    if (!f.exists())
    {
        int ret = QMessageBox::information(this,
                                           tr("File not found!"),
                                           tr("File path lacks information"),
                                           QMessageBox::Ok);
        Q_UNUSED(ret);
        file.close();
        return;
    }
    mInfoList.clear();
    mInfoList.append(tmp);
    QString s = mInfoList.at(0);
    sl = s.split(" ");
    mCharName = sl.at(0);
    ui->leAddName->setText(mCharName);
    mFps = sl.at(1).toInt();
    ui->sBoxFps->setValue(mFps);
    mFramesInAudio = sl.at(2).toInt();
    tmp.clear();
    sl.clear();
    while (!in.atEnd()) {
        tmp = in.readLine();
        mInfoList.append(tmp);
        sl = tmp.split(" ");
        mTabWidgetItem = new QTableWidgetItem(sl.at(1));
        ui->tableWidget->setItem(1, sl.at(0).toInt() - 1, mTabWidgetItem);
        sl.clear();
    }
    file.close();
    player->setMedia(QUrl::fromLocalFile(mFileName));
    savePosition();
}

void MainWindow::save()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Lipsync File"),
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
    if (mDuration == qint) { return; }
    mDuration = qint;
    int remains = mDuration % mFps;
    mFramesInAudio = qCeil((mDuration + remains) / mFps);
    ui->tableWidget->setColumnCount(mFramesInAudio + 1);
    for (int i = 0; i <= mFramesInAudio; i++)
    {
        ui->tableWidget->setColumnWidth(i, 25);
        mTabWidgetItem = new QTableWidgetItem("");
        mTabWidgetItem->setBackgroundColor(Qt::yellow);
        ui->tableWidget->setItem(0, i, mTabWidgetItem);
        mTabWidgetItem = new QTableWidgetItem("");
        ui->tableWidget->setItem(1, i, mTabWidgetItem);
    }
    for (int i = 1; i < mInfoList.length(); i++)
    {
        sl = mInfoList.at(i).split(" ");
        mTabWidgetItem = new QTableWidgetItem(sl.at(1));
        ui->tableWidget->setItem(1, sl.at(0).toInt(), mTabWidgetItem);
    }
}

void MainWindow::setvolume(int i)
{
    player->setVolume(i);
}

void MainWindow::playPhoneme(QTableWidgetItem *twItem)
{
    mCurrFrame = twItem->column();
    mPosition = (twItem->column() + 1) * mFps;
    player->setPosition(mPosition);
    player->play();
    QTimer::singleShot(150, this, SLOT(stopPlayPhoneme()));
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
            int currCol = ui->tableWidget->currentColumn();
            QKeyEvent * ke = static_cast<QKeyEvent *>(event);
            switch (ke->key()) {
            case Qt::Key_Delete: // Delete
                mTabWidgetItem = new QTableWidgetItem("");
                ui->tableWidget->setItem(1, currCol, mTabWidgetItem);
                break;
            case Qt::Key_Space:
                if (player->state() == QMediaPlayer::StoppedState)
                {
                    player->setPosition(mFps * mCurrFrame);
                    player->play();
                }
                else if (player->state() == QMediaPlayer::PlayingState)
                    player->pause();
                else if (player->state() == QMediaPlayer::PausedState)
                {
                    player->setPosition(mFps * mCurrFrame);
                    player->play();
                }
                break;
            default:
                QString s = ke->text();
                mTabWidgetItem = new QTableWidgetItem(s.toUpper());
                ui->tableWidget->setItem(1, currCol, mTabWidgetItem);
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
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    qDebug() << "Keypress: " << e->text();
}

