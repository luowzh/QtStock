#ifndef MA_H
#define MA_H

#include "indicator_base.h"

// ============================================================
// MA - 简单移动平均线
// 公式: MA(n) = (C1 + C2 + ... + Cn) / n
// 类型: 主图指标
// ============================================================

class MA : public IndicatorBase {
public:
    // 构造函数，指定均线周期列表（默认MA5/MA10/MA20/MA60）
    explicit MA(const QList<int>& periods = {5, 10, 20, 60});

    void calculate(const QList<KLine>& data) override;       // 计算均线
    QString name() const override { return "MA"; }           // 指标名称
    IndicatorType type() const override { return IndicatorType::MainChart; } // 主图指标
    QString params() const override;                         // 参数描述

private:
    QList<int> m_periods;                                     // 均线周期列表
};

// ============================================================
// EMA - 指数移动平均线
// 公式: EMA(n) = (Close * 2 + EMA_prev * (n-1)) / (n+1)
// 类型: 主图指标
// ============================================================

class EMA : public IndicatorBase {
public:
    explicit EMA(const QList<int>& periods = {5, 10, 20});

    void calculate(const QList<KLine>& data) override;       // 计算EMA
    QString name() const override { return "EMA"; }          // 指标名称
    IndicatorType type() const override { return IndicatorType::MainChart; } // 主图指标
    QString params() const override;                         // 参数描述

private:
    QList<int> m_periods;                                     // EMA周期列表
};

#endif // MA_H
