#ifndef INDICATOR_BASE_H
#define INDICATOR_BASE_H

#include <QString>
#include <QList>
#include <QVector>
#include <QColor>
#include "../core/stockdata.h"

// ============================================================
// 技术指标基类
// 所有技术指标（MA/MACD/KDJ等）均继承此基类
// ============================================================

// 指标绘制类型
enum class IndicatorType {
    MainChart,    // 主图指标（叠加在K线上，如MA/BOLL）
    SubChart      // 副图指标（单独区域显示，如MACD/KDJ/RSI）
};

// 指标线数据（一条线：如MA5是一条线，MACD的DIF是另一条线）
struct IndicatorLine {
    QString name;               // 线名称，如 "MA5"、"DIF"
    QVector<double> values;     // 计算结果值数组（与K线数据等长）
    QColor color;               // 线颜色

    IndicatorLine() = default;
    IndicatorLine(const QString& n, const QColor& c) : name(n), color(c) {}
};

// 技术指标基类
class IndicatorBase {
public:
    virtual ~IndicatorBase() = default;

    // 计算指标值
    virtual void calculate(const QList<KLine>& data) = 0;    // 纯虚函数：计算指标

    // 获取指标名称
    virtual QString name() const = 0;                        // 纯虚函数：指标名称

    // 获取指标类型（主图/副图）
    virtual IndicatorType type() const = 0;                  // 纯虚函数：指标类型

    // 获取指标线列表（一个指标可能有多条线，如MACD有DIF/DEA/MACD三条）
    const QList<IndicatorLine>& lines() const { return m_lines; } // 返回所有线

    // 获取指标参数描述
    virtual QString params() const = 0;                      // 纯虚函数：参数描述

protected:
    QList<IndicatorLine> m_lines;                            // 指标线列表
};

#endif // INDICATOR_BASE_H
