#ifndef KEYS_H
#define KEYS_H
#include "QString"

class KEYS{

public:
    static const struct Config{
        const QString settings        = "SETTINGS";
        const QString port            = "port";
        const QString address         = "address";
        const QString tray_en         = "tray_enable"; // FIXME: почему-то вылетает приложение, если попытаться изначально поставить пункт меню tray_en в необходимое значение.
        const QString user_settings   = "USER_SETTINGS";
        const QString name            = "name";
    }CONFIG;

    static const struct Json{
        // common
        const QString type              = "type";
        const QString user_name         = "username";
        const QString resources         = "resources";
        // res info
        const QString res_name          = "res_name";
        const QString time              = "time";
        const QString status            = "status";
        // req type
        const QString connect_fail      = "connect_fail";
        const QString authorization     = "authorization";
        const QString grab_res          = "grab_res";
        const QString request_responce  = "request_responce";
        const QString broadcast         = "broadcast";
        const QString res_request       = "res_request";
        // Action
        const QString action            = "action";
        const QString take              = "take";
        const QString drop              = "drop";
    }JSON;

    static const struct Common{
        const QString no_user = "-";
    }COMMON;


};

#endif // JSON_KEYS_H
