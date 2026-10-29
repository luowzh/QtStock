#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include "../core/stockdata.h"

class DataEngine;
class StockList;
class KLineView;
class QLabel;
class QTabWidget;
class QLineEdit;
class QPushButton;

// ============================================================
// 主窗口
// 布局: 左侧自选股列表 + 右侧K线图/分时图 + 底部状态栏
// ============================================================

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onQuotesUpdated(const QList<StockQuote>& quotes);    // 行情更新槽
    void onKLinesReceived(const QString& code,               // K线接收槽
                          const QList<KLine>& klines);
    void onStockSelected(const QString& code);               // 股票选中槽
    void onStockDoubleClicked(const QString& code);          // 双击槽
    void onAddStock();                                        // 添加自选股
    void onRemoveStock();                                     // 删除自选股
    void onRefreshKLine();                                    // 刷新K线
    void onSelectMainIndicator(const QString& name);          // 选择主图指标
    void onSelectSubIndicator(const QString& name);           // 选择副图指标

private:
    void initUI();                                            // 初始化界面
    void initMenuBar();                                       // 初始化菜单栏
    void initToolBar();                                       // 初始化工具栏
    void initStatusBar();                                     // 初始化状态栏
    void loadDefaultStocks();                                 // 加载默认自选股
    void initIndicatorMenu();                                 // 初始化指标菜单

    DataEngine* m_engine;                                     // 数据引擎
    StockList* m_stockList;                                   // 自选股列表
    KLineView* m_klineView;                                   // K线图视图
    QTabWidget* m_chartTab;                                   // 图表标签页(K线/分时)
    QLineEdit* m_codeInput;                                   // 代码输入框
    QPushButton* m_addBtn;                                    // 添加按钮
    QPushButton* m_removeBtn;                                 // 删除按钮
    QLabel* m_statusLabel;                                    // 状态栏标签
    QString m_currentCode;                                    // 当前选中代码
};

#endif // MAINWINDOW_H
