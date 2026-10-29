#include "kline_view.h"                                     // 包含对应头文件
#include <QPainter>                                         // QPainter::Antialiasing抗锯齿枚举
#include <QColor>                                           // QColor颜色类(设置K线红绿色)
#include <QBrush>                                           // QBrush画刷(设置chart背景为黑色)
#include <QDebug>                                           // qInfo()诊断日志需要QDebug完整定义

// ============================================================
// K线图视图实现
// 主图(上) + 副图(下) 双区域布局, 共享时间轴
// ============================================================

KLineView::KLineView(QWidget* parent)
    : QWidget(parent)                                       // 调用QWidget基类构造
    , m_mainView(new QtCharts::QChartView(this))            // 创建主图视图控件
    , m_mainChart(new QtCharts::QChart())                   // 创建主图QChart
    , m_klineSeries(new QtCharts::QCandlestickSeries())     // 创建K线蜡烛图系列
    , m_volumeSeries(new QtCharts::QLineSeries())           // 创建成交量折线系列
    , m_priceAxis(new QtCharts::QValueAxis())               // 创建价格数值轴
    , m_volumeAxis(new QtCharts::QValueAxis())              // 创建成交量数值轴
    , m_timeAxis(new QtCharts::QDateTimeAxis())             // 创建主图时间轴
    , m_subView(new QtCharts::QChartView(this))             // 创建副图视图控件
    , m_subChart(new QtCharts::QChart())                    // 创建副图QChart
    , m_subAxis(new QtCharts::QValueAxis())                 // 创建副图指标值轴
    , m_subTimeAxis(new QtCharts::QDateTimeAxis())          // 创建副图时间轴
    , m_mainIndicator(nullptr)                              // 初始无主图指标
    , m_subIndicator(nullptr)                               // 初始无副图指标
    , m_syncingAxis(false)                                  // 坐标轴同步标志初始为false
{
    qInfo() << "KLineView: constructor start";              // 诊断日志:构造开始

    // ---- K线蜡烛图系列样式 ----
    m_klineSeries->setIncreasingColor(QColor(255, 0, 0));   // 阳线(收盘>=开盘)设为红色
    m_klineSeries->setDecreasingColor(QColor(0, 200, 0));   // 阴线(收盘<开盘)设为绿色
    m_klineSeries->setName("K线");                          // 系列名称设为"K线"
    m_volumeSeries->setColor(QColor(200, 200, 0));            // 成交量线设为黄色(黑色背景下可见)

    // ---- 主图坐标轴样式 ----
    m_priceAxis->setTitleText("价格");                      // 价格轴标题
    m_priceAxis->setLabelFormat("%.2f");                    // 价格标签保留2位小数
    m_volumeAxis->setTitleText("成交量");                   // 成交量轴标题
    m_volumeAxis->setLabelFormat("%.0f");                   // 成交量标签显示整数
    m_timeAxis->setTitleText("时间");                       // 时间轴标题
    m_timeAxis->setFormat("MM-dd");                         // 时间格式:月-日
    m_timeAxis->setTickCount(8);                             // 时间轴刻度数设为8
    // 暗色主题:主图坐标轴标签浅灰色,网格线深灰色(黑色背景下可读)
    m_priceAxis->setLabelsColor(QColor(200, 200, 200));      // 价格轴标签浅灰色
    m_volumeAxis->setLabelsColor(QColor(200, 200, 200));      // 成交量轴标签浅灰色
    m_timeAxis->setLabelsColor(QColor(200, 200, 200));        // 时间轴标签浅灰色
    m_priceAxis->setGridLineColor(QColor(50, 50, 50));        // 价格轴网格线深灰色
    m_volumeAxis->setGridLineColor(QColor(50, 50, 50));       // 成交量轴网格线深灰色
    m_timeAxis->setGridLineColor(QColor(50, 50, 50));         // 时间轴网格线深灰色

    // ---- 副图坐标轴样式 ----
    m_subAxis->setLabelFormat("%.2f");                      // 副图指标值保留2位小数
    m_subTimeAxis->setFormat("MM-dd");                      // 副图时间格式:月-日
    m_subTimeAxis->setTickCount(8);                          // 副图时间轴刻度数设为8
    // 暗色主题:副图坐标轴标签浅灰色,网格线深灰色
    m_subAxis->setLabelsColor(QColor(200, 200, 200));        // 副图指标值轴标签浅灰色
    m_subTimeAxis->setLabelsColor(QColor(200, 200, 200));     // 副图时间轴标签浅灰色
    m_subAxis->setGridLineColor(QColor(50, 50, 50));          // 副图指标值轴网格线深灰色
    m_subTimeAxis->setGridLineColor(QColor(50, 50, 50));      // 副图时间轴网格线深灰色

    // ---- 主图chart组装 ----
    qInfo() << "KLineView: setup main chart";               // 诊断日志:组装主图
    m_mainChart->addSeries(m_klineSeries);                  // 主图添加K线蜡烛图系列
    m_mainChart->addSeries(m_volumeSeries);                 // 主图添加成交量折线系列
    m_mainChart->addAxis(m_priceAxis, Qt::AlignLeft);       // 主图添加价格轴(左侧)
    m_mainChart->addAxis(m_volumeAxis, Qt::AlignRight);     // 主图添加成交量轴(右侧)
    m_mainChart->addAxis(m_timeAxis, Qt::AlignBottom);      // 主图添加时间轴(底部)
    m_mainChart->legend()->hide();                          // 隐藏主图图例
    m_mainView->setChart(m_mainChart);                      // 主图视图绑定主图chart
    m_mainView->setRenderHint(QPainter::Antialiasing);      // 主图开启抗锯齿
    // 暗色主题:主图背景设为纯黑色
    m_mainChart->setBackgroundBrush(QBrush(QColor(0, 0, 0))); // 主图整体背景黑色
    m_mainChart->setPlotAreaBackgroundBrush(QBrush(QColor(0, 0, 0))); // 主图绘图区黑色
    m_mainChart->setPlotAreaBackgroundVisible(true);          // 启用绘图区背景显示
    m_mainChart->setTitleBrush(QBrush(QColor(200, 200, 200))); // 主图标题文字浅灰色(用QBrush设颜色)
    m_mainView->setBackgroundBrush(QBrush(QColor(0, 0, 0))); // 主图视图背景黑色

    // ---- 副图chart组装 ----
    qInfo() << "KLineView: setup sub chart";                // 诊断日志:组装副图
    m_subChart->addAxis(m_subAxis, Qt::AlignLeft);          // 副图添加指标值轴(左侧)
    m_subChart->addAxis(m_subTimeAxis, Qt::AlignBottom);    // 副图添加时间轴(底部)
    m_subChart->legend()->hide();                           // 隐藏副图图例
    m_subView->setChart(m_subChart);                        // 副图视图绑定副图chart
    m_subView->setRenderHint(QPainter::Antialiasing);       // 副图开启抗锯齿
    // 暗色主题:副图背景设为纯黑色
    m_subChart->setBackgroundBrush(QBrush(QColor(0, 0, 0))); // 副图整体背景黑色
    m_subChart->setPlotAreaBackgroundBrush(QBrush(QColor(0, 0, 0))); // 副图绘图区黑色
    m_subChart->setPlotAreaBackgroundVisible(true);           // 启用绘图区背景显示
    m_subChart->setTitleBrush(QBrush(QColor(200, 200, 200)));  // 副图标题文字浅灰色(用QBrush设颜色)
    m_subView->setBackgroundBrush(QBrush(QColor(0, 0, 0)));  // 副图视图背景黑色

    // ---- 上下布局:主图占70%,副图占30% ----
    auto* layout = new QVBoxLayout(this);                   // 创建垂直布局
    layout->setContentsMargins(0, 0, 0, 0);                 // 无边距(充分利用空间)
    layout->setSpacing(1);                                  // 主图副图间距1像素
    layout->addWidget(m_mainView, 7);                       // 主图占7份(约70%高度)
    layout->addWidget(m_subView, 3);                        // 副图占3份(约30%高度)
    m_subView->hide();                                      // 默认隐藏副图(无指标时不显示)

    // ---- 主图与副图时间轴双向同步 ----
    // 用户拖动/缩放主图时,副图X轴跟随;反之亦然
    connect(m_timeAxis, &QtCharts::QDateTimeAxis::rangeChanged, // 主图时间轴范围变化
            this, [this](const QDateTime& min, const QDateTime& max) { // lambda捕获this
        if (!m_syncingAxis) {                               // 非同步触发时才执行
            m_syncingAxis = true;                           // 设置同步标志防止递归
            m_subTimeAxis->setRange(min, max);              // 同步副图时间轴范围
            m_syncingAxis = false;                          // 清除同步标志
        }
    });
    connect(m_subTimeAxis, &QtCharts::QDateTimeAxis::rangeChanged, // 副图时间轴范围变化
            this, [this](const QDateTime& min, const QDateTime& max) { // lambda捕获this
        if (!m_syncingAxis) {                               // 非同步触发时才执行
            m_syncingAxis = true;                           // 设置同步标志防止递归
            m_timeAxis->setRange(min, max);                 // 同步主图时间轴范围
            m_syncingAxis = false;                          // 清除同步标志
        }
    });

    qInfo() << "KLineView: constructor done";               // 诊断日志:构造完成
}

// 设置K线数据并绘制
void KLineView::setKLines(const QList<KLine>& klines) {
    m_klineData = klines;                                   // 缓存K线数据供重绘使用
    clear();                                                // 清空所有series内容
    if (klines.isEmpty()) return;                           // 空数据直接返回不绘制
    drawKLines(klines);                                     // 绘制K线蜡烛图到主图
    drawVolume(klines);                                     // 绘制成交量线到主图
    drawMainIndicator(klines);                              // 绘制主图指标(MA/BOLL叠加在主图)
    drawSubIndicator(klines);                               // 绘制副图指标(MACD/KDJ/RSI画在副图)
    // 根据是否有副图指标,显示或隐藏副图区域
    if (m_subIndicator && !m_subIndicatorLines.isEmpty()) { // 有副图指标且有数据线
        m_subView->show();                                  // 显示副图区域
    } else {                                                // 无副图指标
        m_subView->hide();                                  // 隐藏副图区域
    }
    setupAxes(klines);                                      // 设置主图和副图坐标轴范围
}

// 清空所有图表内容
void KLineView::clear() {
    m_klineSeries->clear();                                 // 清空K线蜡烛图所有数据
    m_volumeSeries->clear();                                // 清空成交量线所有数据
    // 清除主图指标线系列
    for (auto* s : m_mainIndicatorLines) {                  // 遍历主图指标线列表
        m_mainChart->removeSeries(s);                       // 从主图chart移除该series
        delete s;                                           // 释放series内存
    }
    m_mainIndicatorLines.clear();                           // 清空主图指标线列表
    // 清除副图指标线系列
    for (auto* s : m_subIndicatorLines) {                   // 遍历副图指标线列表
        m_subChart->removeSeries(s);                        // 从副图chart移除该series
        delete s;                                           // 释放series内存
    }
    m_subIndicatorLines.clear();                            // 清空副图指标线列表
}

// 设置主图标题
void KLineView::setTitle(const QString& title) {
    m_mainChart->setTitle(title);                           // 设置主图chart的标题
}

// 设置主图指标(MA/EMA/BOLL叠加在K线上)
void KLineView::setMainIndicator(IndicatorBase* indicator) {
    m_mainIndicator = indicator;                            // 保存主图指标指针
    if (!m_klineData.isEmpty()) {                           // 有K线数据时重绘
        setKLines(m_klineData);                             // 用缓存数据重新绘制
    }
}

// 设置副图指标(MACD/KDJ/RSI显示在下方独立区域)
void KLineView::setSubIndicator(IndicatorBase* indicator) {
    m_subIndicator = indicator;                             // 保存副图指标指针
    if (!m_klineData.isEmpty()) {                           // 有K线数据时重绘
        setKLines(m_klineData);                             // 用缓存数据重新绘制
    }
}

// 清除主图指标
void KLineView::clearMainIndicator() {
    m_mainIndicator = nullptr;                              // 主图指标指针置空
    if (!m_klineData.isEmpty()) setKLines(m_klineData);     // 有数据则重绘(不画主图指标)
}

// 清除副图指标
void KLineView::clearSubIndicator() {
    m_subIndicator = nullptr;                               // 副图指标指针置空
    if (!m_klineData.isEmpty()) {                           // 有K线数据时重绘
        setKLines(m_klineData);                             // 重绘(setKLines会隐藏副图)
    } else {                                                // 无K线数据
        m_subView->hide();                                  // 直接隐藏副图区域
    }
}

// 绘制K线蜡烛图到主图
void KLineView::drawKLines(const QList<KLine>& klines) {
    for (int i = 0; i < klines.size(); ++i) {               // 遍历每根K线
        const KLine& k = klines[i];                         // 取第i根K线引用
        auto* set = new QtCharts::QCandlestickSet(          // 创建蜡烛数据集
            k.dateTime.toMSecsSinceEpoch());                // 时间戳作为X值
        set->setOpen(k.open);                               // 设置开盘价
        set->setHigh(k.high);                               // 设置最高价
        set->setLow(k.low);                                 // 设置最低价
        set->setClose(k.close);                             // 设置收盘价
        m_klineSeries->append(set);                         // 添加到K线蜡烛图系列
    }
}

// 绘制成交量线到主图
void KLineView::drawVolume(const QList<KLine>& klines) {
    for (int i = 0; i < klines.size(); ++i) {               // 遍历每根K线
        const KLine& k = klines[i];                         // 取第i根K线引用
        m_volumeSeries->append(                             // 向成交量系列添加点
            k.dateTime.toMSecsSinceEpoch(), k.volume);      // X=时间戳, Y=成交量
    }
}

// 绘制主图指标(MA/EMA/BOLL叠加在主图K线上)
void KLineView::drawMainIndicator(const QList<KLine>& klines) {
    if (!m_mainIndicator) return;                           // 无主图指标直接返回
    m_mainIndicator->calculate(klines);                     // 计算主图指标值

    for (const auto& line : m_mainIndicator->lines()) {     // 遍历指标的每条线
        auto* series = new QtCharts::QLineSeries();         // 创建折线系列
        series->setColor(line.color);                       // 设置线的颜色
        series->setName(line.name);                         // 设置线的名称

        for (int i = 0; i < klines.size(); ++i) {           // 遍历每根K线
            if (i < line.values.size() && line.values[i] != 0) { // 有值且非0
                series->append(                             // 向折线添加点
                    klines[i].dateTime.toMSecsSinceEpoch(), // X=时间戳
                    line.values[i]);                        // Y=指标值
            }
        }
        m_mainChart->addSeries(series);                     // 将指标线添加到主图chart
        series->attachAxis(m_timeAxis);                     // 绑定主图时间轴
        series->attachAxis(m_priceAxis);                    // 绑定主图价格轴(与K线共用)
        m_mainIndicatorLines.append(series);                // 保存到列表供清除使用
    }
}

// 绘制副图指标(MACD/KDJ/RSI画在副图独立区域)
void KLineView::drawSubIndicator(const QList<KLine>& klines) {
    if (!m_subIndicator) return;                            // 无副图指标直接返回
    m_subIndicator->calculate(klines);                      // 计算副图指标值

    for (const auto& line : m_subIndicator->lines()) {      // 遍历指标的每条线
        auto* series = new QtCharts::QLineSeries();         // 创建折线系列
        series->setColor(line.color);                       // 设置线的颜色
        series->setName(line.name);                         // 设置线的名称

        for (int i = 0; i < klines.size(); ++i) {           // 遍历每根K线
            if (i < line.values.size()) {                   // 有值就添加
                series->append(                             // 向折线添加点
                    klines[i].dateTime.toMSecsSinceEpoch(), // X=时间戳
                    line.values[i]);                        // Y=指标值
            }
        }
        m_subChart->addSeries(series);                      // 将指标线添加到副图chart
        series->attachAxis(m_subTimeAxis);                  // 绑定副图时间轴
        series->attachAxis(m_subAxis);                      // 绑定副图指标值轴
        m_subIndicatorLines.append(series);                 // 保存到列表供清除使用
    }

    // 计算副图指标值范围,设置副图Y轴
    double minVal = 1e9, maxVal = -1e9;                     // 初始化最小最大值
    for (const auto& line : m_subIndicator->lines()) {      // 遍历所有线
        for (double v : line.values) {                      // 遍历所有值
            if (v < minVal) minVal = v;                     // 更新最小值
            if (v > maxVal) maxVal = v;                     // 更新最大值
        }
    }
    if (minVal < maxVal) {                                  // 有效范围
        m_subAxis->setRange(minVal, maxVal);                // 设置副图指标值轴范围
    }
}

// 设置主图和副图坐标轴范围
void KLineView::setupAxes(const QList<KLine>& klines) {
    double minPrice = klines[0].low;                        // 最低价初始化为第一根K线最低
    double maxPrice = klines[0].high;                       // 最高价初始化为第一根K线最高
    double maxVolume = 0;                                   // 最大成交量初始化为0

    for (const KLine& k : klines) {                         // 遍历所有K线
        if (k.low < minPrice) minPrice = k.low;             // 更新最低价
        if (k.high > maxPrice) maxPrice = k.high;           // 更新最高价
        if (k.volume > maxVolume) maxVolume = k.volume;     // 更新最大成交量
    }

    // 主图坐标轴范围
    m_priceAxis->setRange(minPrice * 0.99, maxPrice * 1.01); // 价格轴留1%边距
    m_volumeAxis->setRange(0, maxVolume * 1.1);             // 成交量轴从0到最大量1.1倍
    m_timeAxis->setRange(klines.first().dateTime,           // 主图时间轴起始
                         klines.last().dateTime);           // 主图时间轴结束

    // 副图时间轴与主图同步(范围相同)
    m_subTimeAxis->setRange(klines.first().dateTime,        // 副图时间轴起始
                            klines.last().dateTime);        // 副图时间轴结束

    // 主图series绑定坐标轴(首次需要attach)
    m_klineSeries->attachAxis(m_timeAxis);                  // K线绑定主图时间轴
    m_klineSeries->attachAxis(m_priceAxis);                 // K线绑定主图价格轴
    m_volumeSeries->attachAxis(m_timeAxis);                 // 成交量绑定主图时间轴
    m_volumeSeries->attachAxis(m_volumeAxis);               // 成交量绑定主图成交量轴
}
