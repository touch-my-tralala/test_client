#include "context_config/contextconfiguration.h"

BaseContextConfiguration::BaseContextConfiguration(){
}

BaseContextConfiguration::~BaseContextConfiguration(){
    m_sett = nullptr;
}

QVariant BaseContextConfiguration::getConfigParam(const QString &key){
    if(params.contains(key))
        return params[key];

    return QVariant();
}

void BaseContextConfiguration::setConfigParam(const QString &key, const QVariant &value){
    params.insert(key, value);
}


UserContextConfiguration::UserContextConfiguration() = default;

UserContextConfiguration::~UserContextConfiguration() = default;

bool UserContextConfiguration::loadConfiguration(QSettings* sett){
    if(sett != nullptr && !sett->isWritable())
        return false;

    m_sett = sett;

    m_sett->beginGroup(KEYS::Config().user_settings);
    auto keys = sett->childKeys();
    for(const auto &i: qAsConst(keys)){
        setConfigParam(i, sett->value(i));
    }
    sett->endGroup();

    return true;
}

bool UserContextConfiguration::writeConfiguration(){
    if(m_sett != nullptr && !m_sett->isWritable())
        return false;

    m_sett->beginGroup(KEYS::Config().user_settings);
    for(auto i = params.begin(), e = params.end(); i != e; i++){
        setConfigParam(i.key(), i.value());
    }
    m_sett->endGroup();

    return true;
}


CommonContextConfiguration::CommonContextConfiguration() = default;

CommonContextConfiguration::~CommonContextConfiguration() = default;

bool CommonContextConfiguration::loadConfiguration(QSettings* sett){
    if(sett != nullptr && !sett->isWritable())
        return false;

    m_sett = sett;

    m_sett->beginGroup(KEYS::Config().settings);
    auto keys = m_sett->childKeys();
    for(const auto &i: qAsConst(keys)){
        setConfigParam(i, m_sett->value(i));
    }
    m_sett->endGroup();

    return true;
}

bool CommonContextConfiguration::writeConfiguration(){
    if(m_sett != nullptr && !m_sett->isWritable())
        return false;

    m_sett->beginGroup(KEYS::Config().settings);
    for(auto i = params.begin(), e = params.end(); i != e; i++){
        setConfigParam(i.key(), i.value());
    }
    m_sett->endGroup();

    return true;
}


UpdaterContextConfiguration::UpdaterContextConfiguration() = default;

UpdaterContextConfiguration::~UpdaterContextConfiguration() = default;

bool UpdaterContextConfiguration::loadConfiguration(QSettings* sett){
    if(sett != nullptr && !sett->isWritable())
        return false;

    m_sett = sett;

    m_sett->beginGroup(KEYS::Config().updates);
    auto keys = m_sett->childKeys();
    for(const auto &i: qAsConst(keys)){
        setConfigParam(i, m_sett->value(i));
    }
    m_sett->endGroup();

    return true;
}

bool UpdaterContextConfiguration::writeConfiguration(){
    if(m_sett != nullptr && !m_sett->isWritable())
        return false;

    m_sett->beginGroup(KEYS::Config().updates);
    for(auto i = params.begin(), e = params.end(); i != e; i++){
        setConfigParam(i.key(), i.value());
    }
    m_sett->endGroup();

    return true;
}
