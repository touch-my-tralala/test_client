#ifndef JSON_KEYS_H
#define JSON_KEYS_H
#include "QString"

class JSON_KEYS{

public:
    static const struct Config{
        const QString server_settings = "SERVER_SETTINGS";
        const QString port            = "port";
        const QString max_user        = "max_user";
    }CONFIG;

    static const struct Common{
        const QString resource_list     = "RESOURCE_LIST";
        const QString user_list         = "USER_LIST";
        const QString resnum            = "resnum";
        const QString resuser           = "resuser";
        const QString busy_time         = "busy_time";
        const QString status            = "status";
        const QString resource          = "resource";
        const QString resource_responce = "resource_responce";
        const QString user_name         = "username";
        const QString time              = "time";
    }COMMON;

    static const struct Action{
        const QString action = "action";
        const QString leave  = "leave";
        const QString take   = "take";
    }ACTION;

    static const struct State{
        const QString free = "free";
    }STATE;

    static const struct ReqType{
        const QString type              = "type";
        const QString authorization     = "authorization";
        const QString grab_res          = "grab_res";
        const QString request_responce  = "request_responce";
        const QString resource_responce = "resource_responce";
        const QString broadcast         = "broadcast";
        const QString connect_fail      = "connect_fail";
        const QString res_request       = "request";
    }TYPE;

};

#endif // JSON_KEYS_H
