#include "restautoupdater.h"

RestAutoupdater::RestAutoupdater(QObject *parent) : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &RestAutoupdater::get_responce);

    m_file_manager = new QNetworkAccessManager(this);
    connect(m_file_manager, &QNetworkAccessManager::finished, this, &RestAutoupdater::download_file);
}

void RestAutoupdater::setRepo(const QString &repo)
{
    m_repo = Keys().git_rest_api + repo + "/contents/";
}

bool RestAutoupdater::setSavePath(const QString &path){
    QFileInfo info(path);
    if(!info.exists() && info.isDir()){
        m_save_path = path;
        return true;
    }

    return false;
}

void RestAutoupdater::loadUpdates(){
    send_request();
}

void RestAutoupdater::send_request(Type type, const QString &reqStr)
{
    QNetworkRequest request;

    switch(type){
    case(CONTENTS):
        request.setUrl(m_repo + reqStr);
        m_manager->get(request);
        break;

    case(DIR):
        request.setUrl(reqStr);
        m_manager->get(request);
        break;

    case(FILE):
        request.setUrl(reqStr);
        m_file_manager->get(request);
        break;
    }
}

void RestAutoupdater::get_responce(QNetworkReply *reply)
{
    auto doc = QJsonDocument::fromJson(reply->readAll(), &jsonErr);

    if(jsonErr.error == QJsonParseError::NoError)
        responce_handler(doc);
    else
        emit error(jsonErr.errorString());

    reply->deleteLater();
}

//! TODO: надо наверное сделать чтение блоками, да и вообще разобраться с тем как сюда большие данные приходят
void RestAutoupdater::download_file(QNetworkReply *reply){
    auto data = reply->readAll();

    // Возможно это и не надо делать. Но я подумал, что пока один файл отправляется тут могут обработаться другие файлы и возникнет говна-пирога.
    auto name = m_save_path + "/" + m_current_file_name.first();
    m_current_file_name.removeFirst();
    QFile file(name);
    if(!file.open(QIODevice::WriteOnly)){
        emit error("Невозможно открыть файл");
        return;
    }

    file.write(data);
    file.close();

    reply->deleteLater();
}

void RestAutoupdater::responce_handler(const QJsonDocument &doc){
    m_updated_files.clear();
    auto arr = doc.array();
    auto local_doc = read_local_info(Keys().file_info);
    auto local_arr = local_doc.array();

    if(local_doc.isEmpty() || arr.size() != local_arr.size()){
        for(const auto &i: qAsConst(arr)){
            auto obj = i.toObject();
            download_manager(obj);

            if(obj[Keys().type].toString() != "dir"){
                m_updated_files << obj[Keys().name].toString();
                m_current_file_name << obj[Keys().name].toString();
            }
        }
        emit success();
        return;
    }

    auto j = local_arr.begin();
    for(auto i = arr.begin(), e = arr.end(); i != e; i++){
        auto remote_obj = i->toObject();
        auto local_obj = j->toObject();

        if(remote_obj[Keys().sha] != local_obj[Keys().sha]){
            download_manager(remote_obj);

            if(remote_obj[Keys().type].toString() != "dir"){
                m_updated_files << remote_obj[Keys().name].toString();
                m_current_file_name << remote_obj[Keys().name].toString();
            }
        }
        j++;
    }
    emit success();
}

QJsonDocument RestAutoupdater::read_local_info(const QString &name)
{
    QFile file(m_save_path + "/" + name);
    if(!file.open(QIODevice::ReadOnly)){
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        return doc;
    }

    return QJsonDocument();
}

void RestAutoupdater::download_manager(const QJsonObject &obj)
{
    if(obj[Keys().type].toString() == "dir")
        send_request(DIR, obj[Keys().url].toString());
    else
        send_request(FILE, obj[Keys().download_url].toString());
}
