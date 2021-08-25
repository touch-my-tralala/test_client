#ifndef BATFILECREATOR_H
#define BATFILECREATOR_H

#include <QString>

class BatFileCreator
{
public:
    BatFileCreator();

private:
    void create();

private:
    QString fname = "update.bat";
};

#endif // BATFILECREATOR_H
