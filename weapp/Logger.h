#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QDir>
#include <QSharedPointer>

class QFile;
class QTextStream;

class Logger : public QObject
{
    Q_OBJECT
public:
    static Logger *instance();

    void log(const QString &message);
    void flush() const;

private:
    explicit Logger(QObject *parent = nullptr);

    //remove copy constructor and assignment operator for Singleton
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    void createLogFile(bool checkingFilesCount);

    QSharedPointer<QFile> m_file;
    QSharedPointer<QTextStream> m_out;
    QDir m_logDir;
    QString m_fileName;
    int m_fileSize;
};
#endif // LOGGER_H
