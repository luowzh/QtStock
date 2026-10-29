#include "stock_list.h"
#include <QHeaderView>
#include <QColor>
#include <QTableWidgetItem>
#include <QHBoxLayout>
#include <QAbstractItemView>
#include <QStringList>

// ============================================================
// 自选股列表实现
// ============================================================

StockList::StockList(QWidget* parent)
    : QTableWidget(parent)
{
    initTable();                                              // 初始化表格
}

// 初始化表格
void StockList::initTable() {
    QStringList headers;                                      // 表头
    headers << "代码" << "名称" << "最新价" << "涨跌额"
            << "涨跌幅" << "成交量" << "今开" << "最高" << "最低";
    setColumnCount(headers.size());                           // 设置列数
    setHorizontalHeaderLabels(headers);                       // 设置表头

    // 表格外观设置
    horizontalHeader()->setStretchLastSection(true);          // 最后一列拉伸
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // 自适应列宽
    verticalHeader()->setVisible(false);                      // 隐藏行号
    setEditTriggers(QAbstractItemView::NoEditTriggers);       // 禁止编辑
    setSelectionBehavior(QAbstractItemView::SelectRows);      // 整行选中
    setAlternatingRowColors(true);                            // 交替行颜色
    setColumnWidth(0, 80);                                    // 代码列宽
    setColumnWidth(1, 100);                                   // 名称列宽
    setColumnWidth(2, 80);                                    // 最新价列宽
    setColumnWidth(3, 70);                                    // 涨跌额列宽
    setColumnWidth(4, 70);                                    // 涨跌幅列宽

    // 连接信号
    connect(this, &StockList::cellDoubleClicked,              // 双击信号
            this, &StockList::onCellDoubleClicked);
    connect(this, &StockList::cellClicked,                    // 单击信号
            this, &StockList::onCellClicked);
}

// 批量更新行情
void StockList::updateQuotes(const QList<StockQuote>& quotes) {
    setRowCount(quotes.size());                               // 设置行数

    for (int i = 0; i < quotes.size(); ++i) {
        updateRow(i, quotes[i]);                              // 更新每行
    }
}

// 只显示代码列表（无行情数据时）
void StockList::showCodes(const QStringList& codes) {
    setRowCount(codes.size());                                // 设置行数
    for (int i = 0; i < codes.size(); ++i) {
        setItem(i, 0, new QTableWidgetItem(codes[i]));      // 显示代码
        setItem(i, 1, new QTableWidgetItem("等待行情..."));  // 名称占位
        for (int col = 2; col < 9; ++col) {
            setItem(i, col, new QTableWidgetItem("-"));      // 其他列占位
        }
    }
}

// 更新一行数据
void StockList::updateRow(int row, const StockQuote& quote) {
    // 代码
    setItem(row, 0, new QTableWidgetItem(quote.code));       // 股票代码
    // 名称
    setItem(row, 1, new QTableWidgetItem(quote.name));        // 股票名称
    // 最新价
    setItem(row, 2, new QTableWidgetItem(QString::number(quote.current, 'f', 2))); // 当前价
    // 涨跌额
    setItem(row, 3, new QTableWidgetItem(QString::number(quote.changeAmount, 'f', 2))); // 涨跌额
    // 涨跌幅
    setItem(row, 4, new QTableWidgetItem(formatPercent(quote.changePercent))); // 涨跌幅
    // 成交量
    setItem(row, 5, new QTableWidgetItem(formatVolume(quote.volume))); // 成交量
    // 今开
    setItem(row, 6, new QTableWidgetItem(QString::number(quote.open, 'f', 2))); // 今开
    // 最高
    setItem(row, 7, new QTableWidgetItem(QString::number(quote.high, 'f', 2))); // 最高
    // 最低
    setItem(row, 8, new QTableWidgetItem(QString::number(quote.low, 'f', 2)));  // 最低

    // 设置涨跌颜色：涨红跌绿
    QColor color;
    if (quote.current > quote.preClose) {
        color = QColor(255, 0, 0);                            // 涨-红色
    } else if (quote.current < quote.preClose) {
        color = QColor(0, 200, 0);                            // 跌-绿色
    } else {
        color = QColor(128, 128, 128);                        // 平-灰色
    }

    // 给价格相关列设置颜色
    for (int col = 2; col <= 4; ++col) {
        if (item(row, col)) {
            item(row, col)->setForeground(color);             // 设置文字颜色
        }
    }
}

// 格式化涨跌幅
QString StockList::formatPercent(double val) {
    QString sign = val >= 0 ? "+" : "";                       // 正数加+号
    return QString("%1%2%").arg(sign).arg(val, 0, 'f', 2);   // 保留2位小数
}

// 格式化成交量
QString StockList::formatVolume(double val) {
    if (val >= 100000000) {                                   // 亿手
        return QString::number(val / 100000000, 'f', 2) + "亿";
    } else if (val >= 10000) {                                // 万手
        return QString::number(val / 10000, 'f', 2) + "万";
    }
    return QString::number(val, 'f', 0);                      // 手
}

// 双击单元格
void StockList::onCellDoubleClicked(int row, int col) {
    Q_UNUSED(col);                                            // 未使用参数
    if (auto* it = item(row, 0)) {
        emit stockDoubleClicked(it->text());                  // 发送双击信号
    }
}

// 单击单元格
void StockList::onCellClicked(int row, int col) {
    Q_UNUSED(col);                                            // 未使用参数
    if (auto* it = item(row, 0)) {
        emit stockSelected(it->text());                       // 发送选中信号
    }
}
