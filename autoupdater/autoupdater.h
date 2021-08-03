#ifndef AUTOUPDATER_H
#define AUTOUPDATER_H

#include <QTcpSocket>
#include <QFile>
#include <QDataStream>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>

#define BLOCK_DATA 32768 //< количество байт для отправки


class AutoUpdater
{
public:
    enum ApdaterInfo{
        WaitData         = 0,
        EndNameRead      = 1,
        EndFileWriting   = 2,
        FileOpenError    = 3,
        ReadStreamError  = 4,
        WriteStreamError = 5
    };

    AutoUpdater();
    ~AutoUpdater();

    //! \brief Добавление файлов для обновления.
    //! \param[file] <имя файла, версия>.
    //! \return true если файл успешно добавлен.
    bool addUpdateFile(const QPair<QString, QString> &file);
    //! \brief Удаление из списка всех ранее добавленных файлов
    void removeUpdateFiles(){ m_update_files.clear(); }
    //! \brief Установка пути к директории где лежат файлы обновлений.
    //! \param[absolut] true - абсолютный путь, false - относительный запуска приложения.
    //! \return true если путь существует.
    bool setUpdateFilePath(const QString &path);
    //! \brief Проверка необходимости обновлений файла.
    //! \return true если версии не совпадают, false если совпадают или файла нет в списке.
    bool checkNeedUpdate(const QPair<QString, QString>& file);
    //! \brief Прием нового файла обновлений.
    //! \param[readStream] поток для чтения данных.
    //! \param[size] размер файла
    //! \return true файл полностью принят, false файл принят не полностью.
    int recvFile(QDataStream &readStream, const quint32 &size);
    //! \brief Проверка обновления и отправка нового файла при необходимости. Имеет заголовок 4 байта - размер файла, кастомный заголовок(при необходимости).
    //! \param[file] <имя файла, версия>.
    //! \param[header] кастомный заголовок, который необходимо добавить.
    //! \return true файл успешно отправлен, false нет зарегистрированного файла с таким именем или обновление не требуется.
    bool checkAndSendFile(QTcpSocket &sock, const QPair<QString, QVariant> &file, const QByteArray& header);
    //! \brief Возвращает мапу зарегестрированных файлов
    //! \return <имя файла, версия>
    QMap<QString, QString> getFileMap(){ return m_update_files; }
    //! \brief Возвращает путь, где сейчас ищуться файлы обновлений
    QString getUpdatesPath(){ return m_update_file_path; }

private:
    //! \brief Отправка файла пользователю
    bool send_file(QTcpSocket &sock, const QString &fileName, const QByteArray& header);
    //! \brief Отправка имени файла пользователю
    bool send_file_info(QTcpSocket &sock, const QString &fileName, const QByteArray& header);

private:
    char *m_read_block = nullptr;
    QDataStream m_fileStream;
    QFile m_wrFile;
    quint64 m_cur_file_size;
    QString m_update_file_path = "";
    QString m_cur_file_name;
    QByteArray m_buff;
    // FIXME: переделать на контрольную сумму файла или что-то что уникально для файла и не заполнятся руками
    QMap<QString, QString> m_update_files; //< name, version>
};

#endif // AUTOUPDATER_H
