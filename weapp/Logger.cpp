#include "Logger.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStandardPaths>
#include <QFileInfo>

#define MAX_FILE_COUNT 20
#define FILE_SIZE 1000000 //byte

Logger *Logger::instance()
{
    static Logger *instance = nullptr;
    if(!instance){
        instance = new Logger();
    }
    return instance;
}

void Logger::log(const QString &message)
{
    QString output = "%1: %2\r\n";
    QDateTime now = QDateTime::currentDateTime();
    QString datetime = now.toString("yyyy-MM-dd HH:mm:ss");
    QString msg = output.arg(datetime).arg(message);
    if(m_fileSize >= FILE_SIZE){
        m_fileName = "log_" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HHmmss") + ".log";
        createLogFile(true);
        m_fileSize =0;
    }
    m_fileSize += msg.size();
    *m_out << msg;

    this->flush();  //at least for now
}

void Logger::flush() const
{
    m_out->flush();
}

Logger::Logger(QObject *parent) : QObject(parent)
  , m_file(nullptr)
  , m_fileSize(0)
{
    QString locationDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    m_logDir = QDir(locationDirPath + "/WEAPP/Log");
    m_fileName = "log_" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HHmmss") + ".log";

    if(!m_logDir.exists()){
        m_logDir.mkpath(".");
        createLogFile(false);
        return;
    }
    QStringList fileList = m_logDir.entryList(QStringList() << "*.log", QDir::Files, QDir::Name);
    bool checkingFilesCount = false;

    if(!fileList.isEmpty()){
        QFileInfo fileInfo(m_logDir.path() +"/" + fileList.last());
        m_fileSize = fileInfo.size();
        if(m_fileSize >= FILE_SIZE){
            m_fileSize =0;
            checkingFilesCount = true;
        }else{
            m_fileName = fileList.last();
        }
    }
    createLogFile(checkingFilesCount);
}

void Logger::createLogFile(bool checkingFilesCount)
{
    if(m_file){
        m_file.get()->close();
    }
    if(checkingFilesCount){
        QStringList fileList = m_logDir.entryList(QStringList() << "*.log",QDir::Files, QDir::Name);
        if(fileList.size() >= MAX_FILE_COUNT){
            m_logDir.remove(fileList.first());
        }
    }
    m_file.reset(new QFile(m_logDir.path() + "/" + m_fileName));
    m_file.get()->open(QIODevice::WriteOnly | QIODevice::Append);
    m_out.reset(new QTextStream(m_file.get()));
}
