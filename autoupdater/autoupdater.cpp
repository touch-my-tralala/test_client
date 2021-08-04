#include "autoupdater.h"

AutoUpdater::AutoUpdater()
{
    m_fileStream.setVersion(QDataStream::Qt_5_12);
}

AutoUpdater::~AutoUpdater(){
    if(!m_read_block)
        delete [] m_read_block;
}

bool AutoUpdater::setUpdateFilePath(const QString& path)
{
    QFileInfo fileInfo(path);
    if (fileInfo.exists() && fileInfo.isDir())
    {
        m_update_file_path = path;
        return true;
    }
    return false;
}

bool AutoUpdater::addUpdateFile(const QPair<QString, QString>& file)
{
    auto      file_name    = file.first;
    auto      file_version = file.second;
    QFileInfo fInfo(m_update_file_path + "/" + file_name);

    if (!fInfo.exists() || !fInfo.isFile())
        return false;

    m_update_files.insert(file_name, file_version);
    return true;
}

bool AutoUpdater::checkNeedUpdate(const QPair<QString, QString> &file){
    if(m_update_files.contains(file.first))
        return (m_update_files[file.first] == file.second) ? false : true;

    return false;
}

bool AutoUpdater::checkAndSendFile(QTcpSocket& sock, const QPair<QString, QVariant>& file, const QByteArray& header)
{
    auto file_name    = file.first;
    auto file_version = static_cast<QString>(file.second.toString());
    if (m_update_files.contains(file_name) && m_update_files[file_name] != file_version)
    {
        if( !send_file_info(sock, file_name, header) )
            return false;
        if( !send_file(sock, file_name, header) )
            return false;

        return true;
    }

    return false;
}

// Сначала приходят данные [размер][user header][имя файла].
// после этого открывается соответствующий файл для записи
// затем приходят данные [размер][user header][файл]
int AutoUpdater::recvFile(QDataStream &readStream, const quint32 &size){
    if(!m_wrFile.isOpen()){
        m_cur_file_size = 0;
        if(readStream.device()->bytesAvailable() < size)
            return WaitData;

        //FIXME: потестить QString. Если в конструктор передать массив, который не полностью заполнен.
        // он создаст строку до символа конца строки? или попытается создать из всего массива.
        char* read_block = new char[size];
        readStream.readRawData(read_block, size);
        m_cur_file_name = QString(read_block);
        m_wrFile.setFileName(m_update_file_path + "/" + m_cur_file_name);
        delete [] read_block;

        if(!m_wrFile.open(QIODevice::WriteOnly))
            return FileOpenError;

        m_fileStream.setDevice(&m_wrFile);

        if(m_read_block != nullptr)
            delete [] m_read_block;

        m_read_block = new char[BLOCK_DATA];

        return EndNameRead;
    }


    auto bytes_read = readStream.readRawData(m_read_block, BLOCK_DATA);

    if(bytes_read == -1){
        delete [] m_read_block;
        return ReadStreamError;
    }

    auto write_bytes = m_fileStream.writeRawData(m_read_block, bytes_read);

    if(write_bytes == -1){
        delete [] m_read_block;
        return WriteStreamError;
    }

    m_cur_file_size += write_bytes;
    if(m_cur_file_size == size){
        m_wrFile.close();
        delete [] m_read_block;
        return EndFileWriting;
    }

    return WaitData;
}

bool AutoUpdater::send_file_info(QTcpSocket &sock, const QString &fileName, const QByteArray &header)
{
    if (sock.state() != QTcpSocket::ConnectedState)
        return false;

    QDataStream sendStream(&sock);
    sendStream.setVersion(QDataStream::Qt_5_12);

    auto block = fileName.toUtf8();
    // информация о размере строки добавляется автоматически перед строкой 4 байта. Хз как выключить
    sendStream << header << block;

    return true;
}

bool AutoUpdater::send_file(QTcpSocket& sock, const QString& fileName, const QByteArray& header)
{
    if (sock.state() != QTcpSocket::ConnectedState)
        return false;

    QFile file(m_update_file_path + "/" + fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        bool        first_pack = true;
        QByteArray  block;
        QDataStream fileStream(&file);
        char*       read_block = new char[BLOCK_DATA];

        QDataStream sendStream(&block, QIODevice::ReadWrite);
        sendStream.setVersion(QDataStream::Qt_5_12);

        while (fileStream.readRawData(read_block, BLOCK_DATA))
        {
            if (first_pack)
            {
                sendStream << header << quint32(file.size()) << read_block;
                first_pack = false;
            }
            else
            {
                sendStream << read_block;
            }

            sock.write(block);
            block.clear(); // FIXME: надо проверить, мб после seek(0) можно не обнулять буфер а писать поверх
            sendStream.device()->seek(0);
        }
        delete[] read_block;
        return true;
    }
    file.close();
    return false;
}
