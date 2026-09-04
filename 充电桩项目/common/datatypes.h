#ifndef DATATYPES_H
#define DATATYPES_H

#include <QString>
#include <QMetaType>
#include <QList>

// ============================================================================
// 业务数据结构（server / client 共用，通过 Q_DECLARE_METATYPE 注册用于信号槽）
// ============================================================================
struct UserInfo {
    int     id = 0;
    QString phone;
    QString nickname;
    QString avatar;     // 头像 base64 编码，为空则用默认灰头像
    double  balance = 0.0;
    QString status;     // normal / frozen
    QString createTime;
};

struct StationInfo {
    int     id = 0;
    QString name;
    QString address;
    double  longitude = 0.0;
    double  latitude  = 0.0;
    double  pricePerKwh = 0.0;   // 元/度
    int     totalPiles = 0;      // 已由电桩统计填充
    int     idlePiles  = 0;      // 空闲电桩数
    int     onlinePiles= 0;      // 在线电桩数
};

struct PileInfo {
    int     id = 0;
    int     stationId = 0;
    QString code;
    QString type;       // fast / slow
    double  powerKw = 0.0;
    QString status;     // idle / charging / fault / offline
    int     chargeCount = 0;     // 累计充电次数
    int     chargeMinutes = 0;   // 累计充电时长(分钟)
};

struct OrderInfo {
    int     id = 0;
    int     userId = 0;
    int     pileId = 0;
    int     stationId = 0;
    QString status;     // charging / finished
    QString startTime;
    QString endTime;
    double  kwh = 0.0;
    double  fee = 0.0;
};

Q_DECLARE_METATYPE(UserInfo)
Q_DECLARE_METATYPE(StationInfo)
Q_DECLARE_METATYPE(PileInfo)
Q_DECLARE_METATYPE(OrderInfo)
Q_DECLARE_METATYPE(QList<UserInfo>)
Q_DECLARE_METATYPE(QList<StationInfo>)
Q_DECLARE_METATYPE(QList<PileInfo>)
Q_DECLARE_METATYPE(QList<OrderInfo>)

#endif // DATATYPES_H