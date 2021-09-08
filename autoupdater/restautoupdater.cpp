#include "restautoupdater.h"

#include <QDir>
#include <QJsonArray>
#include <QNetworkAccessManager>

#include "autoupdater/batfilecreator.h"

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
    send_request(CONTENTS);
}

void RestAutoupdater::send_request(Type type, const QString& reqStr)
{
    QNetworkRequest request;

    switch (type)
    {
        case (CONTENTS):
            m_file_queue.clear();
            m_download_files_path.clear();
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
    {
        auto remote_arr = doc.array();
        collecting_file_info(remote_arr);
    }
    else
        emit error(jsonErr.errorString());

    reply->deleteLater();
}

void RestAutoupdater::collecting_file_info(const QJsonArray& arr)
{
    for (const auto& i : qAsConst(arr))
    {
        auto obj = i.toObject();
        if (obj[Keys().type].toString() == "dir")
            m_dir_queue.append(obj);
        else
            m_file_queue.append(obj);
    }

    if (!m_dir_queue.isEmpty())
    {
        auto obj = m_dir_queue.takeFirst();
        QDir().mkdir(m_save_path + "/" + obj[Keys().path].toString());
        send_request(DIR, obj[Keys().url].toString());
    }
    else
    {
        download_manager();
    }
}

void RestAutoupdater::download_manager()
{
    auto local_info = read_local_info();

    for (const auto& i : qAsConst(m_file_queue))
    {
        auto obj  = i;
        auto name = obj[Keys().name].toString();

        if (local_info.contains(name))
        {
            auto local_obj = local_info[name];
            if (local_obj[Keys().sha] == obj[Keys().sha] && local_obj[Keys().path] == obj[Keys().path])
                continue;

            m_download_files_path << obj[Keys().path].toString();
            send_request(FILE, obj[Keys().download_url].toString());
        }
        else
        {
            m_download_files_path << obj[Keys().path].toString();
            send_request(FILE, obj[Keys().download_url].toString());
        }
    }

    // удаление лишних элементов, которые больше не используются.
    remove_excess();
    // запись нового локального инфармационного файла
    write_local_info();
}

QMap<QString, QJsonObject> RestAutoupdater::read_local_info()
{
    QFile file(m_save_path + "/" + Keys().file_info);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "update_info.json not open";
        return {};
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QMap<QString, QJsonObject> out;
    auto                       arr = doc.array();

    for (const auto& i : qAsConst(arr))
    {
        auto obj = i.toObject();
        out.insert(obj[Keys().name].toString(), obj);
    }

    return out;
}

void RestAutoupdater::write_local_info()
{
    QFile file(m_save_path + "/" + Keys().file_info);
    if (!file.open(QIODevice::WriteOnly))
    {
        emit error("cant write file_info.json");
        return;
    }

    QJsonArray arr;
    for (const auto& i : qAsConst(m_file_queue))
        arr << i;

    file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    file.close();
}

void RestAutoupdater::download_file(QNetworkReply* reply)
{
    auto data = reply->readAll();

    // Возможно это и не надо делать. Но я подумал, что пока один файл отправляется тут могут обработаться другие файлы и возникнет говна-пирога.
    auto  name = m_save_path + "/" + m_download_files_path.takeFirst();
    QFile file(name);
    if (!file.open(QIODevice::WriteOnly))
    {
        emit error("Невозможно открыть файл");
        return;
    }

    file.write(data);
    file.close();

    if (m_download_files_path.isEmpty())
    {
        BatFileCreator bat;
        bat.create();
        emit success();
    }

    reply->deleteLater();
}

void RestAutoupdater::remove_excess()
{
    QStringList files;
    for (const auto& i : qAsConst(m_file_queue))
        files << i[Keys().name].toString();

    files << Keys().file_info;

    std::function<void(QString)> remove_dir_contets = [files, &remove_dir_contets](const QString& dirName) {
        QDir dir(dirName);

        if (!dir.exists())
            return;

        dir.setFilter(QDir::Files);
        for (const auto& i : dir.entryList())
            if (!files.contains(i))
                dir.remove(i);

        dir.setFilter(QDir::Dirs);
        auto subdirs = dir.entryList();
        for (const auto& i : qAsConst(subdirs))
        {
            if (i == "." || i == "..")
                continue;
            remove_dir_contets(dirName + "/" + i);
        }
    };

    QDir dir(m_save_path);
    dir.setFilter(QDir::Dirs);
    auto dirs = dir.entryList();
    dirs << m_save_path;

    for (const auto& i : qAsConst(dirs))
    {
        // хрен знае откуда в списке беруться папки с таким именем
        if (i == "." || i == "..")
            continue;

        if (i != m_save_path)
            remove_dir_contets(m_save_path + "/" + i);
        else
            remove_dir_contets(m_save_path);
    }
}
