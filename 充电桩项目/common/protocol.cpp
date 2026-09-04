#include "protocol.h"
#include <QIODevice>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonParseError>

namespace Protocol {

// ---------------------------------------------------------------------------
// 封包：帧头(Magic + MsgId + Length) + JSON 负载
// ---------------------------------------------------------------------------
QByteArray buildPacket(quint16 msgId, const QJsonObject &payload)
{
    QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    QByteArray packet;
    packet.reserve(HEADER_SIZE + body.size());

    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);          // 统一大端字节序
    stream << MAGIC << quint16(msgId) << quint32(body.size());

    packet.append(body);
    return packet;
}

// ---------------------------------------------------------------------------
// 拆包器：累积数据并按帧头 Length 逐帧解析
// ---------------------------------------------------------------------------
void PacketParser::append(const QByteArray &data)
{
    m_buffer.append(data);
}

bool PacketParser::nextPacket(quint16 &msgId, QJsonObject &payload)
{
    // 缓冲区中至少要有完整帧头才能判断 Length
    if (m_buffer.size() < HEADER_SIZE) {
        return false;
    }

    // 解析帧头
    QDataStream stream(m_buffer);
    stream.setByteOrder(QDataStream::BigEndian);

    quint16 magic = 0;
    quint16 id = 0;
    quint32 length = 0;
    stream >> magic >> id >> length;

    // 校验幻数，防止脏数据/错位
    if (magic != MAGIC) {
        // 幻数不匹配：丢弃最前面一个字节，重新对齐（简单自愈）
        m_buffer.remove(0, 1);
        return false;
    }

    // 检查负载是否完整（半包处理：数据未到齐，继续等待）
    if (static_cast<quint64>(m_buffer.size()) < static_cast<quint64>(HEADER_SIZE) + length) {
        return false;
    }

    QByteArray body = m_buffer.mid(HEADER_SIZE, static_cast<int>(length));
    m_buffer.remove(0, HEADER_SIZE + static_cast<int>(length));

    // 解析 JSON 负载
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        // JSON 非法：跳过该帧，继续处理后续数据
        return nextPacket(msgId, payload);
    }

    msgId = id;
    payload = doc.object();
    return true;
}

void PacketParser::reset()
{
    m_buffer.clear();
}

} // namespace Protocol
