#ifndef RESTAUTOUPDATER_H
#define RESTAUTOUPDATER_H

#include <QJsonParseError>
#include <QNetworkReply>

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
    //! \brief Получить имена файлов которые обновились
    QStringList getUpdatedFiles() { return m_updated_files; }

signals:
    //! \brief излучается всякий раз, когда загрузка обновлений прошла успешно
    void success();
    //! \brief излучается при возникновении ошибки
    void error(const QString& err);

protected:
    void          send_request(Type type = CONTENTS, const QString& reqStr = "");
    void          get_responce(QNetworkReply* reply);
    QJsonDocument read_local_info(const QString& name);
    void          write_local_info(const QJsonArray& arr);
    void          responce_handler(const QJsonDocument& doc);
    void          download_manager(const QJsonObject& obj);
    void          download_file(QNetworkReply* reply);
    void          download_all(const QJsonArray& arr);
    //! \brief Загрузка недостающих или отличных файлов из удаленного репозитория
    void download_missing(const QJsonArray& local_arr, const QJsonArray& remote_arr);
    void clear_whole_dir();

private:
    bool                   m_info_is_writable = true;
    QString                m_repo;
    QString                m_save_path;
    QNetworkAccessManager* m_manager;
    QNetworkAccessManager* m_file_manager;
    QJsonParseError        jsonErr;
    QStringList            m_current_files_path;
    QStringList            m_updated_files;
};

#endif // RESTAUTOUPDATER_H
