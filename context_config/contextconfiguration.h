#ifndef CONTEXTCONFIGURATION_H
#define CONTEXTCONFIGURATION_H

#include <QVariantMap>
#include <QSettings>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "json_keys/keys.h"


class BaseContextConfiguration{

public:
    //! \brief загрузка параметров из конфигурационного файла
    virtual bool loadConfiguration(QSettings* sett) = 0;
    //! \brief получение параметра по ключу
    virtual QVariant getConfigParam(const QString &key);
    //! \brief установка параметра по ключу
    virtual void setConfigParam(const QString& key, const QVariant &value);
    //! \brief звпись конфигурации в файл
    virtual bool writeConfiguration(){return {};}

protected:
    BaseContextConfiguration();
    virtual ~BaseContextConfiguration();

protected:
    QVariantMap params;
    QSettings* m_sett;
};

//! Загрузка пользовательских данных
class UserContextConfiguration : public BaseContextConfiguration{

public:
    UserContextConfiguration();
    ~UserContextConfiguration();
    bool loadConfiguration(QSettings* sett) override;
    bool writeConfiguration() override;
};

//! Загрузка общих данных
class CommonContextConfiguration : public BaseContextConfiguration{

public:
    CommonContextConfiguration();
    ~CommonContextConfiguration();
    bool loadConfiguration(QSettings* sett) override;
    bool writeConfiguration() override;
};


#endif // CONTEXTCONFIGURATION_H
