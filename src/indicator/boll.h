#ifndef BOLL_H
#define BOLL_H

#include "indicator_base.h"

// ============================================================
// BOLL - 布林带
// 公式:
//   MID = MA(Close, n)
//   UPPER = MID + k * STD(Close, n)
//   LOWER = MID - k * STD(Close, n)
// 类型: 主图指标
// ============================================================

class BOLL : public IndicatorBase {
public:
    explicit BOLL(int n = 20, double k = 2.0);              // 构造函数

    void calculate(const QList<KLine>& data) override;       // 计算BOLL
    QString name() const override { return "BOLL"; }         // 指标名称
    IndicatorType type() const override { return IndicatorType::MainChart; } // 主图指标
    QString params() const override;                         // 参数描述

private:
    int m_n;                                                  // 周期(20)
    double m_k;                                               // 标准差倍数(2.0)
};

// ============================================================
// WR - 威廉指标
// 公式: WR(n) = (HHV(n) - Close) / (HHV(n) - LLV(n)) * 100
// 类型: 副图指标
// ============================================================

class WR : public IndicatorBase {
public:
    explicit WR(int n = 14);                                 // 构造函数

    void calculate(const QList<KLine>& data) override;       // 计算WR
    QString name() const override { return "WR"; }           // 指标名称
    IndicatorType type() const override { return IndicatorType::SubChart; } // 副图指标
    QString params() const override;                         // 参数描述

private:
    int m_n;                                                  // 周期(14)
};

// ============================================================
// OBV - 能量潮指标
// 公式:
//   若 Close > Close_prev: OBV = OBV_prev + Volume
//   若 Close < Close_prev: OBV = OBV_prev - Volume
//   若 Close = Close_prev: OBV = OBV_prev
// 类型: 副图指标
// ============================================================

class OBV : public IndicatorBase {
public:
    explicit OBV();                                          // 构造函数

    void calculate(const QList<KLine>& data) override;       // 计算OBV
    QString name() const override { return "OBV"; }          // 指标名称
    IndicatorType type() const override { return IndicatorType::SubChart; } // 副图指标
    QString params() const override { return "OBV"; }        // 参数描述
};

// ============================================================
// ATR - 真实波幅
// 公式:
//   TR = max(High-Low, |High-Close_prev|, |Low-Close_prev|)
//   ATR = MA(TR, n)
// 类型: 副图指标
// ============================================================

class ATR : public IndicatorBase {
public:
    explicit ATR(int n = 14);                                // 构造函数

    void calculate(const QList<KLine>& data) override;       // 计算ATR
    QString name() const override { return "ATR"; }          // 指标名称
    IndicatorType type() const override { return IndicatorType::SubChart; } // 副图指标
    QString params() const override;                         // 参数描述

private:
    int m_n;                                                  // 周期(14)
};

#endif // BOLL_H
