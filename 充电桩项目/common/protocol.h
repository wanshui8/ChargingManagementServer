#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QByteArray>
#include <QJsonObject>

// ============================================================================
// 通信协议定义
// ----------------------------------------------------------------------------
// 帧格式（大端字节序）：
//   +----------+----------+----------+---------------------+
//   |  Magic   |  MsgId   |  Length  |       Payload       |
//   |  2 字节  |  2 字节  |  4 字节  |   Length 字节       |
//   +----------+----------+----------+---------------------+
//   Magic  : 固定 0x4E51（'NQ'），用于校验帧起始，防止脏数据
//   MsgId  : 消息类型标识，见下方 MsgId 命名空间
//   Length : Payload 的字节长度（网络字节序），用于解决 TCP 粘包/半包
//   Payload: UTF-8 编码的 JSON 文本
// ============================================================================

namespace Protocol {

// 帧头常量
constexpr quint16 MAGIC = 0x4E51;      // 帧幻数 'NQ'
constexpr int     HEADER_SIZE = 8;     // Magic(2) + MsgId(2) + Length(4)

// ---------------------------------------------------------------------------
// 消息类型（MsgId）
//   1xxx : 用户端 -> 服务器端（请求）
//   2xxx : 服务器端 -> 用户端（响应 / 推送）
// ---------------------------------------------------------------------------
namespace MsgId {
    // ---- 用户端 -> 服务器端 ----
    constexpr quint16 REQ_LOGIN          = 1001; // 手机号免密登录/注册
    constexpr quint16 REQ_STATION_LIST   = 1002; // 请求附近充电站列表
    constexpr quint16 REQ_PILE_LIST      = 1003; // 请求某充电站的电桩列表
    constexpr quint16 REQ_CHECK_PENDING  = 1004; // 检查是否存在未完成订单
    constexpr quint16 REQ_START_CHARGE   = 1005; // 启动充电
    constexpr quint16 REQ_STOP_CHARGE    = 1006; // 停止充电并结算
    constexpr quint16 REQ_RECHARGE       = 1007; // 钱包充值
    constexpr quint16 REQ_UPDATE_PROFILE = 1008; // 修改昵称/头像
    constexpr quint16 REQ_MY_ORDERS      = 1009; // 查询我的订单
    constexpr quint16 REQ_HEARTBEAT      = 1010; // 心跳

    // ---- 服务器端 -> 用户端 ----
    constexpr quint16 RES_LOGIN          = 2001; // 登录响应(含用户信息)
    constexpr quint16 RES_STATION_LIST   = 2002; // 充电站列表响应
    constexpr quint16 RES_PILE_LIST      = 2003; // 电桩列表响应
    constexpr quint16 RES_CHECK_PENDING  = 2004; // 未完成订单检查响应
    constexpr quint16 RES_START_CHARGE   = 2005; // 启动充电响应(含订单)
    constexpr quint16 RES_CHARGE_DATA    = 2006; // 充电实时数据推送
    constexpr quint16 RES_STOP_CHARGE    = 2007; // 停止充电响应(含结算订单)
    constexpr quint16 RES_RECHARGE       = 2008; // 充值响应
    constexpr quint16 RES_UPDATE_PROFILE = 2009; // 修改资料响应
    constexpr quint16 RES_MY_ORDERS      = 2010; // 我的订单响应
    constexpr quint16 RES_HEARTBEAT      = 2011; // 心跳响应
}

// ---------------------------------------------------------------------------
// 业务错误码（响应 Payload 中的 code 字段）
// ---------------------------------------------------------------------------
namespace ErrorCode {
    constexpr int OK                = 0;  // 成功
    constexpr int INTERNAL_ERROR    = 1;  // 服务器内部错误
    constexpr int INVALID_MESSAGE   = 2;  // 非法消息/参数
    constexpr int USER_NOT_FOUND    = 3;  // 用户不存在
    constexpr int USER_FROZEN       = 4;  // 用户已冻结
    constexpr int PILE_NOT_FOUND    = 5;  // 电桩不存在
    constexpr int PILE_NOT_IDLE     = 6;  // 电桩非空闲
    constexpr int ORDER_NOT_FOUND   = 7;  // 订单不存在
    constexpr int HAS_PENDING_ORDER = 8;  // 存在未完成订单
    constexpr int BALANCE_NOT_ENOUGH= 9;  // 余额不足
    constexpr int AUTH_FAILED       = 10; // 管理员鉴权失败
}

// ---------------------------------------------------------------------------
// 业务状态字符串（在 JSON 中以字符串传递，语义化、可读）
// ---------------------------------------------------------------------------
namespace PileStatus {
    constexpr const char* IDLE      = "idle";      // 空闲
    constexpr const char* CHARGING  = "charging";  // 充电中
    constexpr const char* FAULT     = "fault";     // 故障
    constexpr const char* OFFLINE   = "offline";   // 离线
    constexpr const char* RESERVED  = "reserved";  // 已预约
}

namespace UserStatus {
    constexpr const char* NORMAL    = "normal";    // 正常
    constexpr const char* FROZEN    = "frozen";    // 冻结
}

namespace OrderStatus {
    constexpr const char* CHARGING  = "charging";  // 充电中
    constexpr const char* FINISHED  = "finished";  // 已结算
    constexpr const char* PENDING   = "pending";   // 待结算/未完成
}

// ---------------------------------------------------------------------------
// 封包：将消息类型 + JSON 负载打包为带帧头的字节流
// ---------------------------------------------------------------------------
QByteArray buildPacket(quint16 msgId, const QJsonObject &payload);

// ---------------------------------------------------------------------------
// 拆包器：维护接收缓冲区，累积 TCP 数据并逐帧拆解，解决粘包/半包问题
// ---------------------------------------------------------------------------
class PacketParser {
public:
    PacketParser() = default;

    // 追加接收到的字节数据（可多次调用）
    void append(const QByteArray &data);

    // 尝试从缓冲区取出一个完整帧；成功返回 true 并填充 msgId/payload
    // 缓冲区不足（半包）或数据非法时返回 false
    bool nextPacket(quint16 &msgId, QJsonObject &payload);

    // 清空缓冲区（连接断开或数据异常时调用）
    void reset();

private:
    QByteArray m_buffer;
};

} // namespace Protocol

#endif // PROTOCOL_H