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

//#include "zip/zip.h"
//#include "zip/miniz.h"

#include "miniz/miniz.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //setFixedSize(width(), height());

    ui->setupUi(this);

    m_status = NONE;

    //ExtractPackage();
    //return;

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

    if(ExtractPackage())
    {
        // Update setting file.
        QSettings settings(m_fileSettings, QSettings::IniFormat);
        settings.beginGroup("Config");
        m_currentVersion = settings.value("CurrentVersion", "").toString();
        settings.setValue("CurrentVersion", m_webCurrentVersion);
        settings.endGroup();
    }
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

bool MainWindow::CheckPackage(const QString& sZipFilePath)
{
    mz_zip_archive* mz = new mz_zip_archive;
    mz_zip_zero_struct(mz);

    mz_bool ok = mz_zip_reader_init_file(mz, sZipFilePath.toUtf8().data(), 0);
    if (!ok) return false;

    int num = mz_zip_reader_get_num_files(mz);

    mz_zip_reader_end(mz);
    return (num > 0);
}

bool MainWindow::ExtractPackage()
{
    m_status = DECOMPRESSING;
    ui->label->setText(GetStatusString(m_status));

    QString zipFilePath = "/home/litwin/DELETE/cemu_1.12.0.zip";
    //QString destPath = QCoreApplication::applicationDirPath();

    if (!QFile::exists(zipFilePath))
    {
        return false;
    }

    if(!CheckPackage(zipFilePath))
    {
        return false;
    }

    //QString sBaseDir = QFileInfo(destPath).absolutePath();
    QString sBaseDir = QCoreApplication::applicationDirPath();
    QDir baseDir(sBaseDir);
    if (!baseDir.exists())
    {
        bool ok = baseDir.mkpath(".");
        Q_ASSERT(ok);
    }

    baseDir.makeAbsolute();

    mz_zip_archive* mz = new mz_zip_archive;
    mz_zip_zero_struct(mz);

    mz_bool ok = mz_zip_reader_init_file(mz, zipFilePath.toUtf8().data(), 0);
    if (!ok)
    {
        return false;
    }

    int num = mz_zip_reader_get_num_files(mz);

    ui->progressBar->setRange(0, 100);

    mz_zip_archive_file_stat* stat = new mz_zip_archive_file_stat;

    for (int i = 0; i < num; ++i)
    {
        ok &= mz_zip_reader_file_stat(mz, i, stat);

        if (stat->m_is_directory)
        {
            QString sFolderPath = QString::fromUtf8(stat->m_filename);
            qDebug() << QString("Make Dir: ").append(sFolderPath);

            bool mkDirOK = baseDir.mkpath(sFolderPath);
            Q_ASSERT(mkDirOK);
            if (!mkDirOK)
            {
                qDebug() << "Make Dir failed";
            }
        }
    }

    int percent = 0;
    ui->progressBar->setValue(percent);

    for (int i = 0; i < num; ++i)
    {
        percent = i * 100 / num;
        ui->progressBar->setValue(percent);

        ok &= mz_zip_reader_file_stat(mz, i, stat);

        if (!stat->m_is_directory)
        {
            QString sFullPath = baseDir.filePath(QString::fromUtf8(stat->m_filename));
            qDebug() << QString("Unzip file: ").append(sFullPath);
            bool b = QFileInfo(sFullPath).absoluteDir().mkpath(".");
            Q_ASSERT(b);

            bool extractOK = mz_zip_reader_extract_to_file(mz, i, sFullPath.toUtf8(), 0);
            if (!extractOK)
            {
                ok = false;
                qDebug() << "  File extraction failed.";
            }
        }
    }

    ui->progressBar->setValue(100);

    mz_zip_reader_end(mz);

    if (!ok)
    {
        qDebug() << "Unzip error!";
    }

    m_status = FINISHED;
    ui->label->setText(GetStatusString(m_status));

    return true;
}

