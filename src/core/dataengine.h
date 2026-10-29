#ifndef DATAENGINE_H
#define DATAENGINE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QTimer>
#include <QMap>
#include "stockdata.h"

class SinaAPI;

// ============================================================
// 数据引擎
// 管理自选股列表、定时刷新行情、缓存数据
// ============================================================

class DataEngine : public QObject {
    Q_OBJECT

public:
    explicit DataEngine(QObject* parent = nullptr);
    ~DataEngine();

    // 自选股管理
    void addStock(const QString& code);                       // 添加自选股
    void removeStock(const QString& code);                    // 删除自选股
    QStringList getStockList() const;                         // 获取自选股列表
    void setStockList(const QStringList& codes);              // 批量设置自选股

    // 行情查询
    StockQuote getQuote(const QString& code) const;           // 获取缓存行情
    QList<StockQuote> getAllQuotes() const;                   // 获取所有行情

    // 刷新控制
    void startAutoRefresh(int intervalMs = 3000);             // 启动自动刷新（默认3秒）
    void stopAutoRefresh();                                   // 停止自动刷新
    void refreshNow();                                        // 立即刷新一次

    // K线请求
    void requestKLines(const QString& code,                   // 请求K线
                        const QString& period = "day",
                        int count = 200);

signals:
    void quotesUpdated(const QList<StockQuote>& quotes);      // 行情更新信号
    void klinesReceived(const QString& code,                 // K线数据信号
                        const QList<KLine>& klines);
    void stockListChanged();                                  // 自选股列表变化
    void errorOccurred(const QString& msg);                   // 错误信号9.0信号

private slots:
    void onQuotesReceived(const QList<StockQuote>& quotes);   // 行情接收处理
    void onKLinesReceived(const QString& code,               // K线接收处理
                          const QList<KLine>& klines);
    void onTimeout();                                         // 定时刷新槽

private:
    SinaAPI* m_api;                                           // 新浪API接口
    QTimer* m_timer;                                          // 定时刷新定时器
    QStringList m_stockList;                                  // 自选股代码列表
    QMap<QString, StockQuote> m_quoteCache;                   // 行情缓存
};

#endif // DATAENGINE_H
