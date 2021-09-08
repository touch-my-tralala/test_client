#ifndef RESTAUTOUPDATER_H
#define RESTAUTOUPDATER_H

#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkReply>

//! TODO: 1) Удаление дирректорий если в них нет содержимого
//! 2) Откуда взялись элементы "." и ".." в списке от entryList

//! \class Класс для автообновления с помощью GitHub Rest API
//! \brief
class RestAutoupdater : public QObject
{
    Q_OBJECT

    enum Type
    {
        CONTENTS = 0,
        DIR      = 1,
        FILE     = 2
    };

public:
    struct Keys
    {
        const QString git_rest_api = "https://api.github.com/repos/";
        const QString file_info    = "updates_info.json";
        const QString sha          = "sha";
        const QString url          = "url";
        const QString name         = "name";
        const QString type         = "type";
        const QString path         = "path";
        const QString download_url = "download_url";
    } KEYS;

public:
    RestAutoupdater(QObject* parent = nullptr);
    //! \brief Установка репозитория для проверки обновлений
    void setRepo(const QString& repo);
    //! \brief Установка пути, куда будут скачиваться обновления
    //! \param[path] путь обновлений. Должен быть директорией.
    bool setSavePath(const QString& path);
    //! \brief Проверяет необходимость обновления и скачивает если необходимо.
    void loadUpdates();

signals:
    //! \brief излучается всякий раз, когда загрузка обновлений прошла успешно
    void success();
    //! \brief излучается при возникновении ошибки
    void error(const QString& err);

protected:
    void                       send_request(Type type = CONTENTS, const QString& reqStr = "");
    void                       get_responce(QNetworkReply* reply);
    QMap<QString, QJsonObject> read_local_info();
    void                       write_local_info();
    void                       download_manager();
    void                       download_file(QNetworkReply* reply);
    //! \brief Загрузка недостающих или отличных файлов из удаленного репозитория
    void download_missing(const QJsonArray& local_arr, const QJsonArray& remote_arr);
    //! \brief Удаление неиспользуемых файлов
    void remove_excess();
    //! \brief Обход всей дирректории и сбор в лист всех файлов.
    void collecting_file_info(const QJsonArray& arr);

private:
    QString                m_repo;
    QString                m_save_path;
    QNetworkAccessManager* m_manager;
    QNetworkAccessManager* m_file_manager;
    QJsonParseError        jsonErr;
    QStringList            m_download_files_path;
    QList<QJsonObject>     m_dir_queue;
    QList<QJsonObject>     m_file_queue;
};

#endif // RESTAUTOUPDATER_H
