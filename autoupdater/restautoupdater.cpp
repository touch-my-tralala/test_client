#include "restautoupdater.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>

RestAutoupdater::RestAutoupdater(QObject* parent)
    : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &RestAutoupdater::get_responce);

    m_file_manager = new QNetworkAccessManager(this);
    connect(m_file_manager, &QNetworkAccessManager::finished, this, &RestAutoupdater::download_file);
}

void RestAutoupdater::setRepo(const QString& repo)
{
    m_repo = Keys().git_rest_api + repo + "/contents/";
}

bool RestAutoupdater::setSavePath(const QString& path)
{
    QFileInfo info(path);
    if (info.exists() && info.isDir())
    {
        m_save_path = path;
        return true;
    }

    return false;
}

void RestAutoupdater::loadUpdates()
{
    m_info_is_writable = true;
    send_request(CONTENTS);
}

void RestAutoupdater::send_request(Type type, const QString& reqStr)
{
    QNetworkRequest request;

    switch (type)
    {
        case (CONTENTS):
            request.setUrl(m_repo + reqStr);
            m_manager->get(request);
            break;

        case (DIR):
            request.setUrl(reqStr);
            m_manager->get(request);
            break;

        case (FILE):
            request.setUrl(reqStr);
            m_file_manager->get(request);
            break;
    }
}

void RestAutoupdater::get_responce(QNetworkReply* reply)
{
    auto doc = QJsonDocument::fromJson(reply->readAll(), &jsonErr);

    if (jsonErr.error == QJsonParseError::NoError)
        responce_handler(doc);
    else
        emit error(jsonErr.errorString());

    reply->deleteLater();
}

//! TODO: надо наверное сделать чтение блоками, да и вообще разобраться с тем как сюда большие данные приходят
void RestAutoupdater::download_file(QNetworkReply* reply)
{
    auto data = reply->readAll();

    // Возможно это и не надо делать. Но я подумал, что пока один файл отправляется тут могут обработаться другие файлы и возникнет говна-пирога.
    auto name = m_save_path + "/" + m_current_files_path.first();
    m_current_files_path.removeFirst();
    QFile file(name);
    if (!file.open(QIODevice::WriteOnly))
    {
        emit error("Невозможно открыть файл");
        return;
    }

    file.write(data);
    file.close();

    reply->deleteLater();
}

void RestAutoupdater::responce_handler(const QJsonDocument& doc)
{
    m_updated_files.clear();
    auto remote_arr = doc.array();
    auto local_doc  = read_local_info(Keys().file_info);
    auto local_arr  = local_doc.array();

    if (local_doc.isEmpty() || local_arr != remote_arr)
        download_all(remote_arr);
    //else
    //    download_missing(local_arr, remote_arr);

    // Если есть файлы для обновления FIXME: по идее когда эта фигна будет ходить по папкам, для каждой
    // папки будет излучаться сигнал. А надо чтобы он излучался 1 раз в самом конце
    if (m_updated_files.size() > 0)
        emit success();
}

QJsonDocument RestAutoupdater::read_local_info(const QString& name)
{
    QFile file(m_save_path + "/" + name);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "update_info.json not open";
        return QJsonDocument();
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    return doc;
}

void RestAutoupdater::write_local_info(const QJsonArray& arr)
{
    QFile file(m_save_path + "/" + Keys().file_info);
    if (!file.open(QIODevice::WriteOnly))
    {
        emit error("cant write file_info.json");
        return;
    }

    file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    file.close();
}

void RestAutoupdater::download_manager(const QJsonObject& obj)
{
    if (obj[Keys().type].toString() == "dir")
    {
        QDir().mkdir(m_save_path + "/" + obj[Keys().path].toString());
        send_request(DIR, obj[Keys().url].toString());
    }
    else
        send_request(FILE, obj[Keys().download_url].toString());
}

void RestAutoupdater::download_missing(const QJsonArray& local_arr, const QJsonArray& remote_arr)
{
    auto j = local_arr.begin();
    for (auto i = remote_arr.begin(), e = remote_arr.end(); i != e; i++)
    {
        auto remote_obj = i->toObject();
        auto local_obj  = j->toObject();

        if (remote_obj[Keys().sha] != local_obj[Keys().sha])
        {
            download_manager(remote_obj);

            if (remote_obj[Keys().type].toString() != "dir")
            {
                m_updated_files << remote_obj[Keys().name].toString();
                m_current_files_path << remote_obj[Keys().path].toString();
            }
        }
        j++;
    }

    write_local_info(remote_arr);
}

void RestAutoupdater::download_all(const QJsonArray& arr)
{
    if (m_info_is_writable)
    {
        clear_whole_dir();
        write_local_info(arr);
    }

    m_info_is_writable = false;

    for (const auto& i : qAsConst(arr))
    {
        auto obj = i.toObject();
        download_manager(obj);

        if (obj[Keys().type].toString() != "dir")
        {
            m_updated_files << obj[Keys().name].toString();
            m_current_files_path << obj[Keys().path].toString();
        }
    }
}

// FIXME:: почему-то не удаляет все из папки
void RestAutoupdater::clear_whole_dir()
{
    QDir dir(m_save_path);
    dir.setFilter(QDir::Files | QDir::Dirs);
    for (const auto& i : dir.entryList())
    {
        if (i != Keys().file_info)
            dir.remove(i);
    }
}
