#ifndef BATFILECREATOR_H
#define BATFILECREATOR_H

#include <QString>

class BatFileCreator
{
public:
    BatFileCreator();
    //! \brief Установка пути, где будет сохранен файл и его имени
    void setName(const QString& path);
    void create();

private:
    QString m_fname = "update.bat";
};

#endif // BATFILECREATOR_H
