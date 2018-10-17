#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "downloadmanager.h"

enum eSTATUS
{
    NONE = 0,
    CHECKING,
    DOWNLOADING,
    DECOMPRESSING,
    FINISHED,
};


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void BytesDownloaded(qint64, qint64, double, QString, QString);
    void DownloadFinished();

private:
    bool Initialization();
    void ReadSetting();

    void RequestVersion();
    void RequestDownload();

    bool CheckVersion();

    bool ExtractPackage();

    QString GetStatusString(eSTATUS status);
    QString ConvertBytesToString(double bytes);

private:
    Ui::MainWindow *ui;

    eSTATUS m_status;

    QString m_fileSettings;

    QString m_URL;
    QString m_DownloadURL;
    QString m_currentVersion;
    QString m_webCurrentVersion;

    DownloadManager* m_downloaderMgr;
};

#endif // MAINWINDOW_H
