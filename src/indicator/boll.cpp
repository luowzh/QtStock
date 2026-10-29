#include "boll.h"
#include <QColor>
#include <cmath>

// ============================================================
// BOLL 布林带实现
// ============================================================

BOLL::BOLL(int n, double k)
    : m_n(n)                                                  // 周期
    , m_k(k)                                                  // 标准差倍数
{
    // BOLL有三条线: UPPER(上轨), MID(中轨), LOWER(下轨)
    m_lines.append(IndicatorLine("UPPER", QColor(255, 0, 0)));   // 上轨-红色
    m_lines.append(IndicatorLine("MID", QColor(255, 255, 0)));   // 中轨-黄色
    m_lines.append(IndicatorLine("LOWER", QColor(0, 200, 0)));   // 下轨-绿色
}

// 计算BOLL
void BOLL::calculate(const QList<KLine>& data) {
    int n = data.size();                                     // 数据长度
    if (n == 0) return;                                      // 空数据不计算

    for (int li = 0; li < 3; ++li) {
        m_lines[li].values.resize(n);                        // 调整大小
        m_lines[li].values.fill(0);                          // 初始化
    }

    for (int i = 0; i < n; ++i) {
        if (i < m_n - 1) continue;                           // 数据不足跳过

        // 计算n日收盘价均值（中轨）
        double sum = 0;                                      // 累加和
        for (int j = i - m_n + 1; j <= i; ++j) {
            sum += data[j].close;                            // 累加收盘价
        }
        double mid = sum / m_n;                              // 中轨 = MA(n)

        // 计算n日收盘价标准差
        double varSum = 0;                                   // 方差累加
        for (int j = i - m_n + 1; j <= i; ++j) {
            double diff = data[j].close - mid;               // 偏差
            varSum += diff * diff;                           // 平方累加
        }
        double stdDev = sqrt(varSum / m_n);                  // 标准差

        // 计算上轨和下轨
        double upper = mid + m_k * stdDev;                   // 上轨 = MID + k*STD
        double lower = mid - m_k * stdDev;                   // 下轨 = MID - k*STD

        m_lines[0].values[i] = upper;                        // 保存上轨
        m_lines[1].values[i] = mid;                          // 保存中轨
        m_lines[2].values[i] = lower;                        // 保存下轨
    }
}

// 参数描述
QString BOLL::params() const {
    return QString("BOLL(%1,%2)").arg(m_n).arg(m_k);         // 参数字符串
}

// ============================================================
// WR 威廉指标实现
// ============================================================

WR::WR(int n)
    : m_n(n)                                                  // 周期
{
    m_lines.append(IndicatorLine("WR", QColor(255, 255, 0)));    // WR线-黄色
}

// 计算WR
void WR::calculate(const QList<KLine>& data) {
    int n = data.size();                                     // 数据长度
    if (n == 0) return;                                      // 空数据不计算

    m_lines[0].values.resize(n);                             // 调整大小
    m_lines[0].values.fill(50);                              // 初始化

    for (int i = 0; i < n; ++i) {
        if (i < m_n - 1) {                                   // 数据不足
            m_lines[0].values[i] = 50;                       // 默认50
            continue;
        }

        // 计算n日内最高价和最低价
        double hhv = data[i].high;                           // n日最高
        double llv = data[i].low;                            // n日最低
        for (int j = i - m_n + 1; j <= i; ++j) {
            if (data[j].high > hhv) hhv = data[j].high;      // 更新最高
            if (data[j].low < llv) llv = data[j].low;        // 更新最低
        }

        // 计算WR
        if (hhv > llv) {                                     // 避免除零
            m_lines[0].values[i] = (hhv - data[i].close) / (hhv - llv) * 100; // WR=(HHV-Close)/(HHV-LLV)*100
        }
    }
}

// 参数描述
QString WR::params() const {
    return QString("WR(%1)").arg(m_n);                       // 参数字符串
}

// ============================================================
// OBV 能量潮实现
// ============================================================

OBV::OBV() {
    m_lines.append(IndicatorLine("OBV", QColor(255, 255, 0)));   // OBV线-黄色
}

// 计算OBV
void OBV::calculate(const QList<KLine>& data) {
    int n = data.size();                                     // 数据长度
    if (n == 0) return;                                      // 空数据不计算

    m_lines[0].values.resize(n);                             // 调整大小
    m_lines[0].values.fill(0);                               // 初始化

    double prevOBV = 0;                                      // 前一日OBV

    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            prevOBV = 0;                                     // 首日OBV=0
        } else {
            if (data[i].close > data[i-1].close) {           // 上涨
                prevOBV += data[i].volume;                   // OBV += 成交量
            } else if (data[i].close < data[i-1].close) {    // 下跌
                prevOBV -= data[i].volume;                   // OBV -= 成交量
            }
            // 平盘OBV不变
        }
        m_lines[0].values[i] = prevOBV;                      // 保存OBV
    }
}

// ============================================================
// ATR 真实波幅实现
// ============================================================

ATR::ATR(int n)
    : m_n(n)                                                  // 周期
{
    m_lines.append(IndicatorLine("ATR", QColor(255, 255, 0)));   // ATR线-黄色
}

// 计算ATR
void ATR::calculate(const QList<KLine>& data) {
    int n = data.size();                                     // 数据长度
    if (n == 0) return;                                      // 空数据不计算

    m_lines[0].values.resize(n);                             // 调整大小
    m_lines[0].values.fill(0);                               // 初始化

    QVector<double> trValues(n, 0);                          // TR数组

    for (int i = 0; i < n; ++i) {
        double tr;                                           // 真实波幅
        if (i == 0) {
            tr = data[i].high - data[i].low;                 // 首日TR=最高-最低
        } else {
            double hl = data[i].high - data[i].low;          // 当日振幅
            double hc = abs(data[i].high - data[i-1].close); // 当日高-昨收
            double lc = abs(data[i].low - data[i-1].close);  // 当日低-昨收
            tr = qMax(hl, qMax(hc, lc));                     // TR = max(HL, HC, LC)
        }
        trValues[i] = tr;                                    // 保存TR
    }

    // 计算ATR = MA(TR, n)
    double sum = 0;                                          // 累加和
    for (int i = 0; i < n; ++i) {
        sum += trValues[i];                                  // 累加TR
        if (i >= m_n) {
            sum -= trValues[i - m_n];                        // 减去超出窗口的
        }
        if (i >= m_n - 1) {
            m_lines[0].values[i] = sum / m_n;                // ATR = MA(TR, n)
        }
    }
}

// 参数描述
QString ATR::params() const {
    return QString("ATR(%1)").arg(m_n);                      // 参数字符串
}
