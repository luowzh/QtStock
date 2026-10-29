#include "macd.h"
#include <QColor>
#include <QStringList>
#include <cmath>

// ============================================================
// MACD 实现
// ============================================================

MACD::MACD(int fast, int slow, int signal)
    : m_fast(fast)                                            // 快线周期
    , m_slow(slow)                                            // 慢线周期
    , m_signal(signal)                                        // 信号线周期
{
    // MACD有三条线: DIF(白), DEA(黄), MACD柱(红绿)
    m_lines.append(IndicatorLine("DIF", QColor(255, 255, 255)));  // DIF线-白色
    m_lines.append(IndicatorLine("DEA", QColor(255, 255, 0)));    // DEA线-黄色
    m_lines.append(IndicatorLine("MACD", QColor(255, 0, 0)));     // MACD柱-红色
}

// EMA辅助计算（基于K线收盘价）
QVector<double> MACD::calcEMA(const QList<KLine>& data, int period) {
    int n = data.size();                                     // 数据长度
    QVector<double> ema(n, 0);                               // EMA结果数组
    if (n == 0) return ema;                                  // 空数据返回

    double multiplier = 2.0 / (period + 1);                 // 平滑系数
    double prev = data[0].close;                             // 首日EMA=收盘价

    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            prev = data[i].close;                            // 首日
        } else {
            prev = (data[i].close - prev) * multiplier + prev; // EMA递推
        }
        ema[i] = prev;                                       // 保存
    }
    return ema;                                              // 返回EMA数组
}

// EMA辅助计算（基于任意序列）
QVector<double> MACD::calcEMA(const QVector<double>& data, int period) {
    int n = data.size();                                     // 数据长度
    QVector<double> ema(n, 0);                               // EMA结果数组
    if (n == 0) return ema;                                  // 空数据返回

    double multiplier = 2.0 / (period + 1);                 // 平滑系数
    double prev = data[0];                                   // 首日

    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            prev = data[i];                                  // 首日
        } else {
            prev = (data[i] - prev) * multiplier + prev;     // EMA递推
        }
        ema[i] = prev;                                       // 保存
    }
    return ema;                                              // 返回EMA数组
}

// 计算MACD
void MACD::calculate(const QList<KLine>& data) {
    int n = data.size();                                     // 数据长度
    if (n == 0) return;                                      // 空数据不计算

    // 1. 计算EMA12和EMA26
    QVector<double> emaFast = calcEMA(data, m_fast);         // 快线EMA
    QVector<double> emaSlow = calcEMA(data, m_slow);         // 慢线EMA

    // 2. 计算DIF = EMA12 - EMA26
    QVector<double> dif(n, 0);                               // DIF数组
    for (int i = 0; i < n; ++i) {
        dif[i] = emaFast[i] - emaSlow[i];                    // DIF = 快线-慢线
    }

    // 3. 计算DEA = EMA(DIF, 9)
    QVector<double> dea = calcEMA(dif, m_signal);            // DEA = DIF的EMA

    // 4. 计算MACD柱 = (DIF - DEA) * 2
    m_lines[0].values = dif;                                 // DIF线
    m_lines[1].values = dea;                                 // DEA线
    m_lines[2].values.resize(n);                             // MACD柱
    for (int i = 0; i < n; ++i) {
        m_lines[2].values[i] = (dif[i] - dea[i]) * 2;        // MACD柱值
    }
}

// 参数描述
QString MACD::params() const {
    return QString("MACD(%1,%2,%3)").arg(m_fast).arg(m_slow).arg(m_signal); // 参数字符串
}

// ============================================================
// KDJ 实现
// ============================================================

KDJ::KDJ(int n, int m1, int m2)
    : m_n(n)                                                  // RSV周期
    , m_m1(m1)                                                // K平滑
    , m_m2(m2)                                                // D平滑
{
    // KDJ有三条线: K(白), D(黄), J(紫)
    m_lines.append(IndicatorLine("K", QColor(255, 255, 255)));  // K线-白色
    m_lines.append(IndicatorLine("D", QColor(255, 255, 0)));    // D线-黄色
    m_lines.append(IndicatorLine("J", QColor(128, 0, 128)));    // J线-紫色
}

// 计算KDJ
void KDJ::calculate(const QList<KLine>& data) {
    int n = data.size();                                     // 数据长度
    if (n == 0) return;                                      // 空数据不计算

    m_lines[0].values.resize(n);                             // K值数组
    m_lines[1].values.resize(n);                             // D值数组
    m_lines[2].values.resize(n);                             // J值数组

    double prevK = 50;                                       // 前一日K（初始50）
    double prevD = 50;                                       // 前一日D（初始50）

    for (int i = 0; i < n; ++i) {
        // 计算n日内的最低价和最高价
        double llv = data[i].low;                            // n日最低
        double hhv = data[i].high;                           // n日最高
        for (int j = qMax(0, i - m_n + 1); j <= i; ++j) {
            if (data[j].low < llv) llv = data[j].low;        // 更新最低
            if (data[j].high > hhv) hhv = data[j].high;      // 更新最高
        }

        // 计算RSV
        double rsv = 50;                                     // 默认RSV=50
        if (hhv > llv) {                                     // 避免除零
            rsv = (data[i].close - llv) / (hhv - llv) * 100; // RSV=(收盘-最低)/(最高-最低)*100
        }

        // 计算K、D、J
        double k = (m_m1 - 1) * prevK / m_m1 + rsv / m_m1;   // K = 2/3*prevK + 1/3*RSV
        double d = (m_m2 - 1) * prevD / m_m2 + k / m_m2;     // D = 2/3*prevD + 1/3*K
        double j = 3 * k - 2 * d;                            // J = 3K - 2D

        m_lines[0].values[i] = k;                            // 保存K
        m_lines[1].values[i] = d;                            // 保存D
        m_lines[2].values[i] = j;                            // 保存J

        prevK = k;                                           // 更新prevK
        prevD = d;                                           // 更新prevD
    }
}

// 参数描述
QString KDJ::params() const {
    return QString("KDJ(%1,%2,%3)").arg(m_n).arg(m_m1).arg(m_m2); // 参数字符串
}

// ============================================================
// RSI 实现
// ============================================================

RSI::RSI(const QList<int>& periods)
    : m_periods(periods)                                      // RSI周期列表
{
    static const QColor colors[] = {                         // RSI颜色数组
        QColor(255, 255, 255),  // RSI6  - 白色
        QColor(255, 255, 0),    // RSI12 - 黄色
        QColor(128, 0, 128)     // RSI24 - 紫色
    };

    for (int i = 0; i < m_periods.size(); ++i) {
        QString lineName = QString("RSI%1").arg(m_periods[i]); // 线名称
        QColor color = colors[i % 3];                        // 循环颜色
        m_lines.append(IndicatorLine(lineName, color));      // 添加线
    }
}

// 计算RSI
void RSI::calculate(const QList<KLine>& data) {
    int n = data.size();                                     // 数据长度
    if (n == 0) return;                                      // 空数据不计算

    for (int li = 0; li < m_periods.size(); ++li) {
        int period = m_periods[li];                          // 当前周期
        m_lines[li].values.resize(n);                        // 调整大小
        m_lines[li].values.fill(50);                         // 初始化为50

        double avgUp = 0;                                    // 平均涨幅
        double avgDown = 0;                                  // 平均跌幅

        for (int i = 1; i < n; ++i) {
            double diff = data[i].close - data[i-1].close;   // 涨跌幅
            double up = diff > 0 ? diff : 0;                 // 涨幅
            double down = diff < 0 ? -diff : 0;              // 跌幅

            if (i <= period) {
                // 初始累积
                avgUp += up;                                 // 累加涨幅
                avgDown += down;                             // 累加跌幅
                if (i == period) {
                    avgUp /= period;                         // 计算平均
                    avgDown /= period;                       // 计算平均
                }
            } else {
                // Wilder平滑法
                avgUp = (avgUp * (period - 1) + up) / period; // 平滑涨幅
                avgDown = (avgDown * (period - 1) + down) / period; // 平滑跌幅
            }

            // 计算RSI
            if (i >= period) {
                if (avgUp + avgDown > 0) {                   // 避免除零
                    m_lines[li].values[i] = avgUp / (avgUp + avgDown) * 100; // RSI = AUG/(*100
                }
            }
        }
    }
}

// 参数描述
QString RSI::params() const {
    QStringList sl;                                           // 参数列表
    for (int p : m_periods) sl << QString::number(p);        // 转字符串
    return "RSI(" + sl.join(",") + ")";                      // 拼接参数
}
