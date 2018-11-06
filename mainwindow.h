#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QTableWidgetItem>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void init();

public slots:
    void load();
    void open();
    void save();
    void saveAs();
    void close();
    void setFps(int i);
    void setCharName(QString name);
    void setDuration(qint64 qint);
    void newPosition(qint64 qint);
    void playPhoneme(QTableWidgetItem* twItem);
    void stopPlayPhoneme();

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:

    void savePosition();
    void loadPosition();

    Ui::MainWindow *ui;
    QMediaPlayer* player;
    QString mFileName;
    QString mCharName;
    QStringList mInfoList;
    QStringList sl;
    QTableWidgetItem* mTabWidgetItem = nullptr;

    int mFps;
    qint64 mPosition;
    qint64 mDuration;
    qint64 mFramesInAudio;
};

#endif // MAINWINDOW_H

