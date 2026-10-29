#include "dataengine.h"
#include "sina_api.h"
#include <QTimer>

// ============================================================
// 数据引擎实现
// ============================================================

DataEngine::DataEngine(QObject* parent)
    : QObject(parent)
    , m_api(new SinaAPI(this))                                // 创建新浪API
    , m_timer(new QTimer(this))                               // 创建定时器
{
    // 连接API信号
    connect(m_api, &SinaAPI::quotesReceived,                  // 行情接收
            this, &DataEngine::onQuotesReceived);
    connect(m_api, &SinaAPI::klinesReceived,                  // K线接收
            this, &DataEngine::onKLinesReceived);
    connect(m_api, &SinaAPI::errorOccurred,                   // 错误传递
           this, &DataEngine::errorOccurred);

    // 连接定时器
    connect(m_timer, &QTimer::timeout,                        // 定时刷新
            this, &DataEngine::onTimeout);
}

DataEngine::~DataEngine() = default;

// 添加自选股
void DataEngine::addStock(const QString& code) {
    QString normalized = normalizeCode(code);                 // 规范化代码
    if (!m_stockList.contains(normalized)) {                  // 避免重复
        m_stockList.append(normalized);                       // 添加到列表
        emit stockListChanged();                              // 通知列表变化
    }
}

// 删除自选股
void DataEngine::removeStock(const QString& code) {
    QString normalized = normalizeCode(code);                 // 规范化代码
    if (m_stockList.removeAll(normalized) > 0) {              // 删除所有匹配项
        m_quoteCache.remove(normalized);                      // 清除缓存
        emit stockListChanged();                              // 通知列表变化
    }
}

// 获取自选股列表
QStringList DataEngine::getStockList() const {
    return m_stockList;                                       // 返回列表
}

// 批量设置自选股
void DataEngine::setStockList(const QStringList& codes) {
    m_stockList.clear();                                      // 清空列表
    for (const QString& code : codes) {
        QString normalized = normalizeCode(code);    // 规范化
        if (!m_stockList.contains(normalized)) {
            m_stockList.append(normalized);                   // 添加
        }
    }
    emit stockListChanged();                                  // 通知变化
}

// 获取缓存行情
StockQuote DataEngine::getQuote(const QString& code) const {
    return m_quoteCache.value(normalizeCode(code));           // 返回缓存数据
}

// 获取所有行情
QList<StockQuote> DataEngine::getAllQuotes() const {
    return m_quoteCache.values();                             // 返回所有缓存
}

// 启动自动刷新
void DataEngine::startAutoRefresh(int intervalMs) {
    m_timer->start(intervalMs);                               // 启动定时器
    refreshNow();                                             // 立即刷新一次
}

// 停止自动刷新
void DataEngine::stopAutoRefresh() {
    m_timer->stop();                                          // 停止定时器
}

// 立即刷新
void DataEngine::refreshNow() {
    if (m_stockList.isEmpty()) return;                        // 空列表不刷新
    m_api->requestQuotes(m_stockList);                        // 请求行情
}

// 请求K线
void DataEngine::requestKLines(const QString& code,
                                const QString& period, int count) {
    m_api->requestKLines(code, period, count);                // 转发给API
}

// 行情接收处理
void DataEngine::onQuotesReceived(const QList<StockQuote>& quotes) {
    for (const StockQuote& q : quotes) {
        m_quoteCache[q.code] = q;                             // 更新缓存
    }
    emit quotesUpdated(quotes);                               // 发送更新信号
}

// K线接收处理
void DataEngine::onKLinesReceived(const QString& code,
                                   const QList<KLine>& klines) {
    emit klinesReceived(code, klines);                        // 转发K线信号
}

// 定时刷新槽
void DataEngine::onTimeout() {
    refreshNow();                                             // 执行刷新
}
