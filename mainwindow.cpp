#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "downloadmanager.h"

#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QMessageBox>
#include <QDesktopWidget>

#include <QFile>

#include "zip/zip.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //setFixedSize(width(), height());

    ui->setupUi(this);

    m_status = NONE;

    Initialization();

    RequestVersion();

    if(CheckVersion())
    {
        // launcher is already up-date.
        m_status = FINISHED;
        ui->label->setText(GetStatusString(m_status));
        ui->progressBar->setValue(100);
    }
    else
    {
        // need to update.
        m_status = DOWNLOADING;
        ui->label->setText(GetStatusString(m_status));

        RequestDownload();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::Initialization()
{
    //m_fileSettings = QCoreApplication::applicationDirPath() +"/Config.ini";

    // TODO : change this directory way.
    m_fileSettings = "/home/litwin/MDS/PDF2CASH_Launcher/Config.ini";

    // Read setting file.
    ReadSetting();

    m_status = CHECKING;

    ui->label->setText(GetStatusString(m_status));

    ui->progressBar->setValue(0);

    return true;
}

QString MainWindow::GetStatusString(eSTATUS status)
{
    switch (status)
    {
    case NONE: return "";
    case CHECKING: return "Verificando";
    case DOWNLOADING: return "Fazendo download";
    case DECOMPRESSING: return "Descompactando";
    case FINISHED: return "Finalizado!";
    }
}

void MainWindow::ReadSetting()
{
    QFileInfo file(m_fileSettings);
    bool exists = file.exists();
    if(!exists)
    {
        QMessageBox m( QMessageBox::Critical, "Erro", "Não foi encontrada o arquivo Config.ini !", QMessageBox::NoButton, this );

        QSize mSize = m.sizeHint();
        QRect screenRect = QDesktopWidget().screen()->rect();
        m.move( QPoint( screenRect.width()/2 - mSize.width()/2, screenRect.height()/2 - mSize.height()/2 ) );
        m.exec();

        exit(1);
    }

    QSettings settings(m_fileSettings, QSettings::IniFormat);
    //settings.sync();
    settings.beginGroup("Config");
    m_currentVersion = settings.value("CurrentVersion", "").toString();
    m_URL = settings.value("URL", "").toString();
    m_DownloadURL = settings.value("DownloadURL", "").toString();
    settings.endGroup();
}

void MainWindow::RequestVersion()
{
    QEventLoop eventLoop;

    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    QUrl url(m_URL);
    QNetworkRequest req( url );

    QNetworkReply *reply = mgr.get(req);
    eventLoop.exec();

    if (reply->error() == QNetworkReply::NoError) {

        QString strReply = (QString)reply->readAll();

        QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());

        QJsonObject jsonObj = jsonResponse.object();
        QJsonValue res = jsonObj.value("version");
        int version = res.toString().toInt();
        m_webCurrentVersion = QString::number(version);
    }
    else
    {
        QMessageBox m( QMessageBox::Critical, "Erro", "Não foi possível carregar o json data !", QMessageBox::NoButton, this );

        QSize mSize = m.sizeHint();
        QRect screenRect = QDesktopWidget().screen()->rect();
        m.move( QPoint( screenRect.width()/2 - mSize.width()/2, screenRect.height()/2 - mSize.height()/2 ) );
        m.exec();

        exit(1);
    }

    delete reply;
}

bool MainWindow::CheckVersion()
{
    return (m_currentVersion == m_webCurrentVersion) ? true : false;
}

void MainWindow::DownloadFinished()
{
    ui->label->setText("Download realizado com sucesso.");
}

void MainWindow::RequestDownload()
{
    m_downloaderMgr = new DownloadManager();
    QUrl url(m_DownloadURL);
    m_downloaderMgr->append(url);

    ui->progressBar->setRange(0, 100);

    connect(m_downloaderMgr, &DownloadManager::BytesDownloaded, this, &MainWindow::BytesDownloaded);
    QObject::connect(m_downloaderMgr, SIGNAL(finished()), this, SLOT(DownloadFinished()));
}

QString MainWindow::ConvertBytesToString(double bytes)
{
    QString unit;
    if (bytes < 1024)
    {
        unit = "bytes/sec";
    }
    else if (bytes < 1024*1024)
    {
        bytes /= 1024;
        unit = "kB/s";
    }
    else
    {
        bytes /= 1024*1024;
        unit = "MB/s";
    }

    std::string str;
    QString message;
    str = unit.toStdString();
    message.sprintf("%.2lf %s", bytes, str.c_str());

    return message;
}

void MainWindow::BytesDownloaded(qint64 bytesReceived, qint64 bytesTotal, double speed, QString unit, QString fileName)
{
    int percent = bytesReceived * 100 / bytesTotal;
    ui->progressBar->setValue(percent);

    std::string str;
    QString message;
    str = fileName.toStdString();
    message.sprintf("Baixando arquivo: %s - [%s / %s] | ", str.c_str(),
                    ConvertBytesToString(bytesReceived).toStdString().c_str(),
                    ConvertBytesToString(bytesTotal).toStdString().c_str());


    QString speedDownload;
    str = unit.toStdString();
    speedDownload.sprintf("Velocidade: %.2lf %s", speed, str.c_str());

    message += speedDownload;

    ui->label->setText(message);
}

int on_extract_entry(const char *filename, void *arg)
{
    static int i = 0;
    int n = *(int *)arg;
    printf("Extracted: %s (%d of %d)\n", filename, ++i, n);

    return 0;
}

bool MainWindow::ExtractPackage()
{
    int arg = 2;
    QByteArray array =  QCoreApplication::applicationDirPath().toLocal8Bit();
    char* buffer = array.data();
    int status = zip_extract("/home/litwin/DELETE/cemu_1.12.0.zip", buffer, on_extract_entry, &arg);
    if(status == 0)
    {

    }
    else
    {

    }
}

