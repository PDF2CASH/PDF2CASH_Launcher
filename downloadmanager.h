#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QtNetwork>
#include <QtCore>

class DownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = nullptr);

    void append(const QUrl &url);
    static QString saveFileName(const QUrl &url);

signals:
    void finished();
    void BytesDownloaded(qint64, qint64, double, QString, QString);

private slots:
    void startNextDownload();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();
    void downloadReadyRead();

private:
    bool isHttpRedirect() const;
    void reportRedirect();

private:
    QNetworkAccessManager manager;

    QFile output;
    QQueue<QUrl> downloadQueue;
    QNetworkReply* currentDownload = nullptr;
    QTime downloadTime;

    int downloadedCount = 0;
    int totalCount = 0;
};

#endif // DOWNLOADMANAGER_H
