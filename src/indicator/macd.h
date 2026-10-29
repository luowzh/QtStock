#ifndef MACD_H
#define MACD_H

#include "indicator_base.h"

// ============================================================
// MACD - 指数平滑异同移动平均线
// 公式:
//   EMA12 = EMA(Close, 12)
//   EMA26 = EMA(Close, 26)
//   DIF = EMA12 - EMA26
//   DEA = EMA(DIF, 9)
//   MACD = (DIF - DEA) * 2
// 类型: 副图指标
// ============================================================

class MACD : public IndicatorBase {
public:
    explicit MACD(int fast = 12, int slow = 26, int signal = 9); // 构造函数

    void calculate(const QList<KLine>& data) override;       // 计算MACD
    QString name() const override { return "MACD"; }         // 指标名称
    IndicatorType type() const override { return IndicatorType::SubChart; } // 副图指标
    QString params() const override;                         // 参数描述

private:
    int m_fast;                                               // 快线周期(12)
    int m_slow;                                               // 慢线周期(26)
    int m_signal;                                             // 信号线周期(9)

    // EMA计算辅助函数
    static QVector<double> calcEMA(const QList<KLine>& data, int period); // 计算EMA
    static QVector<double> calcEMA(const QVector<double>& data, int period); // 对任意序列计算EMA
};

// ============================================================
// KDJ - 随机指标
// 公式:
//   RSV = (Close - LLV(n)) / (HHV(n) - LLV(n)) * 100
//   K = 2/3 * K_prev + 1/3 * RSV
//   D = 2/3 * D_prev + 1/3 * K
//   J = 3 * K - 2 * D
// 类型: 副图指标
// ============================================================

class KDJ : public IndicatorBase {
public:
    explicit KDJ(int n = 9, int m1 = 3, int m2 = 3);        // 构造函数

    void calculate(const QList<KLine>& data) override;       // 计算KDJ
    QString name() const override { return "KDJ"; }          // 指标名称
    IndicatorType type() const override { return IndicatorType::SubChart; } // 副图指标
    QString params() const override;                         // 参数描述

private:
    int m_n;                                                  // RSV周期(9)
    int m_m1;                                                 // K平滑系数(3)
    int m_m2;                                                 // D平滑系数(3)
};

// ============================================================
// RSI - 相对强弱指标
// 公式:
//   RSI(n) = AU / (AU + AD) * 100
//   AU = n日内涨幅平均值
//   AD = n日内跌幅平均值
// 类型: 副图指标
// ============================================================

class RSI : public IndicatorBase {
public:
    explicit RSI(const QList<int>& periods = {6, 12, 24});  // 构造函数

    void calculate(const QList<KLine>& data) override;       // 计算RSI
    QString name() const override { return "RSI"; }          // 指标名称
    IndicatorType type() const override { return IndicatorType::SubChart; } // 副图指标
    QString params() const override;                         // 参数描述

private:
    QList<int> m_periods;                                     // RSI周期列表
};

#endif // MACD_H
