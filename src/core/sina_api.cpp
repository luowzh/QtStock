#include "sina_api.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QByteArray>
#include <QStringList>
#include <QTextCodec>
#include <QRegularExpression>

// ============================================================
// 新浪行情接口实现
// 新浪实时行情接口格式:
//   var hq_str_sh600519="贵州茅台,1800.00,1799.00,1805.00,...";
// 新浪历史2K线接口:
//   https://money.finance.sina.com.cn/quotes_service/api/json_v2.php/CN_MarketData.getKLineData?symbol=sh600519&scale=240&ma=no&datalen=200
// ============================================================

SinaAPI::SinaAPI(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))                 // 创建网络管理器
{
}

SinaAPI::~SinaAPI() = default;

// 构造实时行情请求URL
QString SinaAPI::buildQuoteUrl(const QStringList& codes) {
    // 新浪行情接口: http://hq.sinajs.cn/list=sh600519,sz000001
    QStringList normalized;                                   // 规范化后的代码列表
    for (const QString& code : codes) {
        normalized << normalizeCode(code);                    // 添加市场前缀
    }
    return QString("http://hq.sinajs.cn/list=%1")
        .arg(normalized.join(","));                           // 拼接代码列表
}

// 构造K线请求URL
QString SinaAPI::buildKLineUrl(const QString& code,
                                const QString& period, int count) {
    // 新浪K线接口
    QString symbol = normalizeCode(code);                    // 规范化代码
    int scale = 240;                                          // 默认日线(240分钟)
    if (period == "5") scale = 5;                            // 5分钟线
    else if (period == "15") scale = 15;                     // 15分钟线
    else if (period == "30") scale = 30;                     // 30分钟线
    else if (period == "60") scale = 60;                     // 60分钟线
    else if (period == "week") scale = 1680;                 // 周线
    else if (period == "month") scale = 7200;                // 月线

    return QString("http://money.finance.sina.com.cn/quotes_service/api/json_v2.php/"
                   "CN_MarketData.getKLineData?symbol=%1&scale=%2&ma=no&datalen=%3")
        .arg(symbol).arg(scale).arg(count);                  // 拼接URL参数(http避免OpenSSL依赖)
}

// 请求实时行情
void SinaAPI::requestQuotes(const QStringList& codes) {
    if (codes.isEmpty()) return;                              // 空列表不请求
    QString url = buildQuoteUrl(codes);                       // 构造URL
    QNetworkRequest request(url);                             // 创建请求
    request.setHeader(QNetworkRequest::UserAgentHeader,      // 设置User-Agent
                      "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
    request.setRawHeader("Referer", "https://finance.sina.com.cn"); // 新浪要求Referer
    QNetworkReply* reply = m_nam->get(request);               // 发送GET请求
    connect(reply, &QNetworkReply::finished,                  // 连接完成信号
            this, [this, reply]() { onQuoteReply(reply); });
}

// 请求K线数据
void SinaAPI::requestKLines(const QString& code,
                             const QString& period, int count) {
    m_pendingCode = normalizeCode(code);                      // 保存当前请求代码
    QString url = buildKLineUrl(code, period, count);         // 构造URL
    QNetworkRequest request(url);                             // 创建请求
    request.setHeader(QNetworkRequest::UserAgentHeader,      // 设置User-Agent
                       "Mozilla/5.0");
    request.setRawHeader("Referer", "https://finance.sina.com.cn"); // 新浪要求Referer
    QNetworkReply* reply = m_nam->get(request);               // 发送GET请求
    connect(reply, &QNetworkReply::finished,                  // 连接完成信号
           this, [this, reply]() { onKLineReply(reply); });
}

// 请求分时数据
void SinaAPI::requestTimeSharing(const QString& code) {
    m_pendingCode = normalizeCode(code);                      // 保存当前请求代码
    QString symbol = normalizeCode(code);                     // 规范化代码
    // 新浪分时接口(http避免OpenSSL依赖)
    QString url = QString("http://money.finance.sina.com.cn/quotes_service/api/json_v2.php/"
                          "CN_MarketData.getKLineData?symbol=%1&scale=5&ma=no&datalen=48")
        .arg(symbol);                                         // 请求当日5分钟线
    QNetworkRequest request(url);                             // 创建请求
    request.setHeader(QNetworkRequest::UserAgentHeader,      // 设置User-Agent
                       "Mozilla/5.0");
    request.setRawHeader("Referer", "https://finance.sina.com.cn"); // 新浪要求Referer
    QNetworkReply* reply = m_nam->get(request);               // 发送GET请求
    connect(reply, &QNetworkReply::finished,                  // 连接完成信号
            this, [this, reply]() { onTimeSharingReply(reply); });
}

// 解析实时行情数据
QList<StockQuote> SinaAPI::parseQuoteData(const QByteArray& data) {
    QList<StockQuote> result;                                 // 结果列表
    // 新浪行情数据格式: var hq_str_sh600519="名称,今开,昨收,当前,最高,最低,...";
    QTextCodec* codec = QTextCodec::codecForName("GBK");     // 新浪返回GBK编码
    QString text = codec->toUnicode(data);                   // 转为QString

    QRegularExpression re("var hq_str_(\\w+)=\"([^\"]*)\"");  // 正则匹配每行
    auto it = re.globalMatch(text);                           // 全局匹配迭代器

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();            // 取一个匹配
        QString code = match.captured(1);                     // 股票代码
        QString content = match.captured(2);                  // 数据内容
        QStringList fields = content.split(",");              // 逗号分隔

        if (fields.size() < 32) continue;                  // 字段不足跳过

        StockQuote quote;                                     // 构造行情数据
        quote.code = code;                                    // 代码
        quote.name = fields[0];                               // 名称
        quote.open = fields[1].toDouble();                    // 今开
        quote.preClose = fields[2].toDouble();                // 昨收
        quote.current = fields[3].toDouble();                 // 当前价
        quote.high = fields[4].toDouble();                    // 最高
        quote.low = fields[5].toDouble();                     // 最低
        quote.volume = fields[8].toDouble();                  // 成交量
        quote.amount = fields[9].toDouble();                  // 成交额
        quote.bidPrice = fields[10].toDouble();               // 买一价
        quote.askPrice = fields[20].toDouble();               // 卖一价
        quote.bidVolume = fields[11].toDouble();              // 买一量
        quote.askVolume = fields[21].toDouble();              // 卖一量
        quote.timestamp = QDateTime::currentDateTime();       // 当前时间

        // 计算涨跌幅和涨跌额
        if (quote.preClose > 0) {
            quote.changeAmount = quote.current - quote.preClose;     // 涨跌额
            quote.changePercent = quote.changeAmount / quote.preClose * 100.0; // 涨跌幅
        }

        result.append(quote);                                 // 添加到结果
    }

    return result;                                            // 返回解析结果
}

// 解析K线数据
QList<KLine> SinaAPI::parseKLineData(const QByteArray& data) {
    QList<KLine> result;                                      // 结果列表
    QString text = QString::fromUtf8(data);                   // JSON文本

    // 新浪K线JSON格式: [{"day":"2024-01-01","open":"100","high":"105","low":"99","close":"103","volume":"10000"},...]
    QRegularExpression re("\\{[^}]*\\}");                     // 匹配每个JSON对象
    auto it = re.globalMatch(text);                           // 全局匹配

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();            // 取一个匹配
        QString obj = match.captured(0);                      // JSON对象文本

        KLine kline;                                          // 构造K线数据
        // 提取各字段
        QRegularExpression fieldRe("\"(\\w+)\":\"([^\"]*)\"");
        auto fieldIt = fieldRe.globalMatch(obj);              // 字段匹配迭代器
        while (fieldIt.hasNext()) {
            QRegularExpressionMatch fm = fieldIt.next();       // 取一个字段
            QString key = fm.captured(1);                     // 字段名
            QString value = fm.captured(2);                   // 字段值

            if (key == "day" || key == "date") {
                kline.dateTime = QDateTime::fromString(value, "yyyy-MM-dd"); // 解析日期
            } else if (key == "open") {
                kline.open = value.toDouble();                // 开盘价
            } else if (key == "high") {
                kline.high = value.toDouble();                // 最高价
            } else if (key == "low") {
                kline.low = value.toDouble();                 // 最低价
            } else if (key == "close") {
                kline.close = value.toDouble();               // 收盘价
            } else if (key == "volume") {
                kline.volume = value.toDouble();              // 成交量
            }
        }

        if (kline.close > 0) {                                // 有效数据才添加
            result.append(kline);
        }
    }

    return result;                                            // 返回解析结果
}

// 解析分时数据
QList<TimeSharing> SinaAPI::parseTimeSharingData(const QByteArray& data) {
    QList<TimeSharing> result;                                // 结果列表
    QString text = QString::fromUtf8(data);                   // JSON文本

    QRegularExpression re("\\{[^}]*\\}");                     // 匹配每个JSON对象
    auto it = re.globalMatch(text);                           // 全局匹配

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();            // 取一个匹配
        QString obj = match.captured(0);                      // JSON对象文本

        TimeSharing ts;                                       // 构造分时数据
        QRegularExpression fieldRe("\"(\\w+)\":\"([^\"]*)\"");
        auto fieldIt = fieldRe.globalMatch(obj);
        while (fieldIt.hasNext()) {
            QRegularExpressionMatch fm = fieldIt.next();
            QString key = fm.captured(1);
            QString value = fm.captured(2);

            if (key == "day" || key == "date") {
                ts.dateTime = QDateTime::fromString(value, "yyyy-MM-dd HH:mm:ss");
                if (!ts.dateTime.isValid()) {
                    ts.dateTime = QDateTime::fromString(value, "yyyy-MM-dd");
                }
            } else if (key == "close") {
                ts.price = value.toDouble();                  // 价格
            } else if (key == "volume") {
                ts.volume = value.toDouble();                 // 成交量
            }
        }

        if (ts.price > 0) {
            result.append(ts);
        }
    }

    return result;
}

// 行情回复处理
void SinaAPI::onQuoteReply(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {           // 检查错误
        emit errorOccurred(reply->errorString());             // 发送错误信号
        reply->deleteLater();                                 // 释放reply
        return;
    }

    QByteArray data = reply->readAll();                       // 读取数据
    QList<StockQuote> quotes = parseQuoteData(data);          // 解析行情
    if (!quotes.isEmpty()) {
        emit quotesReceived(quotes);                          // 发送行情信号
    }
    reply->deleteLater();                                     // 释放reply
}

// K线回复处理
void SinaAPI::onKLineReply(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {           // 检查错误
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();                       // 读取数据
    QList<KLine> klines = parseKLineData(data);               // 解析K线
    emit klinesReceived(m_pendingCode, klines);               // 发送K线信号
    reply->deleteLater();
}

// 分时回复处理
void SinaAPI::onTimeSharingReply(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {           // 检查错误
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();                       // 读取数据
    QList<TimeSharing> tsData = parseTimeSharingData(data);   // 解析分时
    emit timeSharingReceived(m_pendingCode, tsData);          // 发送分时信号
    reply->deleteLater();
}
