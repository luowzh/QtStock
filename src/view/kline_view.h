#ifndef KLINE_VIEW_H                                        // 防止重复包含头文件
#define KLINE_VIEW_H                                        // 定义头文件宏

#include <QWidget>                                          // QWidget基类(KLineView继承它)
#include <QVBoxLayout>                                      // 垂直布局(主图副图上下排列)
#include <QtCharts/QChartView>                              // Qt图表视图(显示QChart)
#include <QtCharts/QChart>                                  // Qt图表(管理series和axis)
#include <QtCharts/QCandlestickSeries>                      // 蜡烛图系列(绘制K线)
#include <QtCharts/QCandlestickSet>                         // 蜡烛图数据集(单根K线)
#include <QtCharts/QLineSeries>                             // 折线系列(成交量/指标线)
#include <QtCharts/QValueAxis>                              // 数值轴(价格/成交量/指标值)
#include <QtCharts/QDateTimeAxis>                           // 日期时间轴(横轴)
#include <QList>                                            // QList容器(存储K线数据)
#include "../indicator/indicator_base.h"                    // 指标基类(MA/MACD等)
#include "../core/stockdata.h"                              // 股票数据结构(KLine等)

// ============================================================
// K线图视图组件
// 布局: 主图(上) + 副图(下), 上下排列, 共享时间轴
// 主图: K线蜡烛图 + 成交量线 + 主图指标(MA/EMA/BOLL)
// 副图: 副图指标(MACD/KDJ/RSI/WR/OBV/ATR), 无指标时隐藏
// ============================================================

class KLineView : public QWidget {                          // KLineView继承QWidget(非QChartView)
    Q_OBJECT                                                // Qt元对象宏(支持信号槽)

public:
    explicit KLineView(QWidget* parent = nullptr);          // 构造函数

    // 设置K线数据
    void setKLines(const QList<KLine>& klines);             // 设置K线数据并绘制主图+副图
    void clear();                                           // 清空所有图表内容

    // 设置标题
    void setTitle(const QString& title);                    // 设置主图标题

    // 技术指标管理
    void setMainIndicator(IndicatorBase* indicator);        // 设置主图指标(MA/EMA/BOLL叠加在K线上)
    void setSubIndicator(IndicatorBase* indicator);         // 设置副图指标(MACD/KDJ/RSI显示在下方独立区域)
    void clearMainIndicator();                              // 清除主图指标
    void clearSubIndicator();                               // 清除副图指标(隐藏副图区域)

private:
    void drawKLines(const QList<KLine>& klines);            // 绘制K线蜡烛图到主图
    void drawVolume(const QList<KLine>& klines);            // 绘制成交量线到主图
    void drawMainIndicator(const QList<KLine>& klines);     // 绘制主图指标(MA/BOLL叠加在主图)
    void drawSubIndicator(const QList<KLine>& klines);      // 绘制副图指标(MACD/KDJ/RSI画在副图)
    void setupAxes(const QList<KLine>& klines);             // 设置主图和副图坐标轴范围

    QList<KLine> m_klineData;                               // 缓存当前K线数据(供重绘使用)

    // ---- 主图区域(K线+成交量+主图指标) ----
    QtCharts::QChartView* m_mainView;                       // 主图视图控件(上方)
    QtCharts::QChart* m_mainChart;                          // 主图QChart(管理主图所有series和axis)
    QtCharts::QCandlestickSeries* m_klineSeries;            // K线蜡烛图系列
    QtCharts::QLineSeries* m_volumeSeries;                  // 成交量折线系列
    QtCharts::QValueAxis* m_priceAxis;                      // 价格轴(主图左侧)
    QtCharts::QValueAxis* m_volumeAxis;                     // 成交量轴(主图右侧)
    QtCharts::QDateTimeAxis* m_timeAxis;                    // 主图时间轴(底部,与副图同步)

    // ---- 副图区域(MACD/KDJ/RSI等) ----
    QtCharts::QChartView* m_subView;                        // 副图视图控件(下方,无指标时隐藏)
    QtCharts::QChart* m_subChart;                           // 副图QChart(管理副图所有series和axis)
    QtCharts::QValueAxis* m_subAxis;                        // 副图指标值轴(左侧)
    QtCharts::QDateTimeAxis* m_subTimeAxis;                 // 副图时间轴(底部,与主图同步)

    // ---- 指标相关 ----
    IndicatorBase* m_mainIndicator;                         // 主图指标指针(MA/EMA/BOLL)
    IndicatorBase* m_subIndicator;                          // 副图指标指针(MACD/KDJ/RSI等)
    QList<QtCharts::QLineSeries*> m_mainIndicatorLines;     // 主图指标线系列列表(供清除使用)
    QList<QtCharts::QLineSeries*> m_subIndicatorLines;      // 副图指标线系列列表(供清除使用)
    bool m_syncingAxis;                                     // 坐标轴同步标志(防止rangeChanged递归)
};

#endif // KLINE_VIEW_H                                      // 结束头文件
