#ifndef STOCK_LIST_H
#define STOCK_LIST_H

#include <QWidget>
#include <QTableWidget>
#include <QStringList>
#include "../core/stockdata.h"

// ============================================================
// 自选股列表组件
// 表格显示自选股实时行情，涨红跌绿
// ============================================================

class StockList : public QTableWidget {
    Q_OBJECT

public:
    explicit StockList(QWidget* parent = nullptr);

    // 更新行情数据
    void updateQuotes(const QList<StockQuote>& quotes);       // 批量更新行情

    // 只显示股票代码（无行情数据时用）
    void showCodes(const QStringList& codes);                 // 只显示代码列表

signals:
    void stockDoubleClicked(const QString& code);             // 双击股票代码信号
    void stockSelected(const QString& code);                  // 选中股票信号

private slots:
    void onCellDoubleClicked(int row, int col);               // 双击单元格
    void onCellClicked(int row, int col);                     // 单击单元格

private:
    void initTable();                                         // 初始化表格
    void updateRow(int row, const StockQuote& quote);         // 更新一行数据
    QString formatPercent(double val);                        // 格式化涨跌幅
    QString formatVolume(double val);                         // 格式化成交量
};

#endif // STOCK_LIST_H
