#ifndef RESTAUTOUPDATER_H
#define RESTAUTOUPDATER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileInfo>


//! \class Класс для автообновления с помощью GitHub Rest API
//! \brief
class RestAutoupdater : public QObject
{
    Q_OBJECT

    enum Type{
        CONTENTS = 0,
        DIR      = 1,
        FILE     = 2
    };

public:
    struct Keys{
        const QString git_rest_api = "https://api.github.com/repos/";
        const QString file_info    = "updates_info.json";
        const QString sha          = "sha";
        const QString url          = "url";
        const QString name         = "name";
        const QString type         = "type";
        const QString file_path    = "path";
        const QString download_url = "download_url";
    }KEYS;

public:
    RestAutoupdater(QObject *parent = nullptr);
    //! \brief Установка репозитория для проверки обновлений
    virtual void setRepo(const QString &repo);
    //! \brief Установка пути, куда будут скачиваться обновления
    //! \param[path] путь обновлений. Должен быть директорией.
    virtual bool setSavePath(const QString &path);
    //! \brief Проверяет необходимость обновления и скачивает если необходимо.
    virtual void loadUpdates();
    //! \brief Получить имена файлов которые обновились
    QStringList getUpdatedFiles(){return m_updated_files;}

signals:
    //! \brief излучается всякий раз, когда загрузка обновлений прошла успешно
    void success();
    //! \brief излучается при возникновении ошибки
    void error(const QString &err);

protected:
    virtual void send_request(Type type = CONTENTS, const QString &reqStr = "");
    virtual void get_responce(QNetworkReply* reply);
    virtual QJsonDocument read_local_info(const QString &name);
    virtual void responce_handler(const QJsonDocument &doc);
    virtual void download_manager(const QJsonObject &obj);
    virtual void download_file(QNetworkReply* reply);

private:
    QString m_repo;
    QString m_save_path;
    QNetworkAccessManager* m_manager;
    QNetworkAccessManager* m_file_manager;
    QJsonParseError jsonErr;
    QStringList m_current_file_name;
    QStringList m_updated_files;
};

#endif // RESTAUTOUPDATER_H
