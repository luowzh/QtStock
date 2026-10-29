#include "ma.h"
#include <QColor>
#include <QStringList>

// ============================================================
// MA 简单移动平均线实现
// ============================================================

MA::MA(const QList<int>& periods)
    : m_periods(periods)                                      // 保存周期列表
{
    // 预定义颜色
    static const QColor colors[] = {                         // 均线颜色数组
        QColor(255, 255, 0),    // MA5  - 黄色
        QColor(255, 165, 0),    // MA10 - 橙色
        QColor(128, 128, 255),  // MA20 - 蓝色
        QColor(128, 0, 128)     // MA60 - 紫色
    };

    for (int i = 0; i < m_periods.size(); ++i) {
        QString lineName = QString("MA%1").arg(m_periods[i]); // 线名称
        QColor color = colors[i % 4];                        // 循环使用颜色
        m_lines.append(IndicatorLine(lineName, color));      // 添加线
    }
}

// 计算MA
void MA::calculate(const QList<KLine>& data) {
    int n = data.size();                                     // 数据长度
    if (n == 0) return;                                      // 空数据不计算

    // 为每条均线计算
    for (int li = 0; li < m_periods.size(); ++li) {
        int period = m_periods[li];                          // 当前周期
        m_lines[li].values.resize(n);                        // 调整结果数组大小
        m_lines[li].values.fill(0);                          // 初始化为0

        double sum = 0;                                      // 累加和
        for (int i = 0; i < n; ++i) {
            sum += data[i].close;                            // 累加收盘价
            if (i >= period) {
                sum -= data[i - period].close;              // 减去超出窗口的数据
            }
            if (i >= period - 1) {
                m_lines[li].values[i] = sum / period;       // 计算平均值
            }
        }
    }
}

// 参数描述
QString MA::params() const {
    QStringList sl;                                           // 参数列表
    for (int p : m_periods) sl << QString::number(p);        // 转为字符串
    return "MA(" + sl.join(",") + ")";                       // 拼接参数
}

// ============================================================
// EMA 指数移动平均线实现
// ============================================================

EMA::EMA(const QList<int>& periods)
    : m_periods(periods)                                      // 保存周期列表
{
    static const QColor colors[] = {                         // EMA颜色数组
        QColor(255, 255, 0),    // EMA5  - 黄色
        QColor(255, 165, 0),    // EMA10 - 橙色
        QColor(128, 128, 255)   // EMA20 - 蓝色
    };

    for (int i = 0; i < m_periods.size(); ++i) {
        QString lineName = QString("EMA%1").arg(m_periods[i]); // 线名称
        QColor color = colors[i % 3];                        // 循环使用颜色
        m_lines.append(IndicatorLine(lineName, color));      // 添加线
    }
}

// 计算EMA
void EMA::calculate(const QList<KLine>& data) {
    int n = data.size();                                     // 数据长度
    if (n == 0) return;                                      // 空数据不计算

    for (int li = 0; li < m_periods.size(); ++li) {
        int period = m_periods[li];                          // 当前周期
        m_lines[li].values.resize(n);                        // 调整大小
        m_lines[li].values.fill(0);                          // 初始化

        double multiplier = 2.0 / (period + 1);             // 平滑系数
        double prevEMA = 0;                                  // 前一日EMA

        for (int i = 0; i < n; ++i) {
            double close = data[i].close;                    // 当日收盘价
            if (i == 0) {
                prevEMA = close;                             // 首日EMA=收盘价
            } else {
                prevEMA = (close - prevEMA) * multiplier + prevEMA; // EMA递推公式
            }
            m_lines[li].values[i] = prevEMA;                 // 保存结果
        }
    }
}

// 参数描述
QString EMA::params() const {
    QStringList sl;                                           // 参数列表
    for (int p : m_periods) sl << QString::number(p);        // 转为字符串
    return "EMA(" + sl.join(",") + ")";                      // 拼接参数
}
