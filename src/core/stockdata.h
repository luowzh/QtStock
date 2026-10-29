#ifndef STOCKDATA_H
#define STOCKDATA_H

#include <QString>
#include <QDateTime>

// ============================================================
// 股票数据结构定义
// ============================================================

// 实时行情数据结构
struct StockQuote {
    QString code;           // 股票代码，如 "sh600519"
    QString name;           // 股票名称，如 "贵州茅台"
    double  open;           // 今开价
    double  preClose;       // 昨收价
    double  current;        // 当前价
    double  high;           // 最高价
    double  low;            // 最低价
    double  volume;         // 成交量（手）
    double  amount;         // 成交额（元）
    double  bidPrice;       // 买一价
    double  askPrice;       // 卖一价
    double  bidVolume;      // 买一量
    double  askVolume;      // 卖一量
    double  changePercent;  // 涨跌幅（%）
    double  changeAmount;   // 涨跌额
    double  turnoverRate;   // 换手率（%）
    QDateTime timestamp;    // 数据时间戳

    StockQuote() : open(0), preClose(0), current(0), high(0), low(0),
                   volume(0), amount(0), bidPrice(0), askPrice(0),
                   bidVolume(0), askVolume(0), changePercent(0),
                   changeAmount(0), turnoverRate(0) {}
};

// K线数据结构（单根K线）
struct KLine {
    QDateTime dateTime;     // 时间
    double  open;           // 开盘价
    double  close;          // 收盘价
    double  high;           // 最高价
    double  low;            // 最低价
    double  volume;         // 成交量（手）
    double  amount;         // 成交额（元）

    KLine() : open(0), close(0), high(0), low(0), volume(0), amount(0) {}
};

// 分时数据结构
struct TimeSharing {
    QDateTime dateTime;     // 时间
    double  price;          // 价格
    double  avgPrice;       // 均价
    double  volume;         // 成交量

    TimeSharing() : price(0), avgPrice(0), volume(0) {}
};

// 股票类型枚举
enum class StockType {
    SHA,    // 沪市A股
    SZA,    // 深市A股
    SHB,    // 沪市B股
    SZB,    // 深市B股
    Fund,   // 基金
    Bond,   // 债券
    Index,  // 指数
    Unknown
};

// 工具函数：根据代码判断市场前缀
inline QString normalizeCode(const QString& code) {
    QString c = code.toLower();                               // 转小写
    if (c.startsWith("sh") || c.startsWith("sz")) return c;   // 已有前缀直接返回
    if (c.startsWith("6")) return "sh" + c;                   // 6开头是沪市
    if (c.startsWith("5")) return "sh" + c;                   // 5开头是沪市基金
    if (c.startsWith("0") || c.startsWith("3")) return "sz" + c; // 0/3开头是深市
    return c;                                                  // 其他原样返回
}

// 工具函数：判断是否上涨
inline bool isRising(const StockQuote& q) {
    return q.current > q.preClose;                             // 当前价>昨收则为涨
}

// 工具函数：判断是否涨停
inline bool isLimitUp(const StockQuote& q) {
    if (q.preClose <= 0) return false;                        // 昨收为0无法计算
    return (q.current / q.preClose - 1.0) >= 0.099;           // 涨幅>=9.9%视为涨停
}

// 工具函数：判断是否跌停
inline bool isLimitDown(const StockQuote& q) {
    if (q.preClose <= 0) return false;                        // 昨收为0无法计算
    return (q.current / q.preClose - 1.0) <= -0.099;          // 跌幅>=9.9%视为跌停
}

#endif // STOCKDATA_H
