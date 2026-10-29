#include "mainwindow.h"
#include "stock_list.h"
#include "kline_view.h"
#include "../core/dataengine.h"
#include "../core/csv_loader.h"
#include "../indicator/indicator_base.h"
#include "../indicator/ma.h"
#include "../indicator/macd.h"
#include "../indicator/boll.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QHeaderView>
#include <QStatusBar>
#include <QToolBar>
#include <QDateTime>
#include <QCoreApplication>

// ============================================================
// 主窗口实现
// ============================================================

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_engine(new DataEngine(this))                          // 创建数据引擎
    , m_stockList(new StockList(this))                        // 创建自选股列表
    , m_klineView(new KLineView(this))                        // 创建K线视图
    , m_chartTab(new QTabWidget(this))                        // 创建图表标签页
    , m_codeInput(new QLineEdit(this))                        // 创建代码输入框
    , m_addBtn(new QPushButton("添加", this))                 // 创建添加按钮
    , m_removeBtn(new QPushButton("删除", this))              // 创建删除按钮
    , m_statusLabel(new QLabel("就绪", this))                 // 创建状态标签
{
    initUI();                                                 // 初始化界面
    initMenuBar();                                            // 初始化菜单栏
    initToolBar();                                            // 初始化工具栏
    initStatusBar();                                          // 初始化状态栏

    // 连接数据引擎信号（在loadDefaultStocks之前连接，避免信号丢失）
    connect(m_engine, &DataEngine::quotesUpdated,             // 行情更新
            this, &MainWindow::onQuotesUpdated);
    connect(m_engine, &DataEngine::klinesReceived,            // K线接收
            this, &MainWindow::onKLinesReceived);
    connect(m_engine, &DataEngine::errorOccurred,             // 错误提示
            this, [this](const QString& msg) {
        m_statusLabel->setText("行情错误: " + msg);            // 状态栏显示错误
    });

    // 连接自选股列表信号
    connect(m_stockList, &StockList::stockSelected,           // 选中股票
            this, &MainWindow::onStockSelected);
    connect(m_stockList, &StockList::stockDoubleClicked,      // 双击股票
            this, &MainWindow::onStockDoubleClicked);

    loadDefaultStocks();                                      // 加载自选股（CSV）

    // 启动自动刷新（3秒间隔）
    m_engine->startAutoRefresh(3000);                         // 3秒刷新一次
}

MainWindow::~MainWindow() = default;

// 初始化界面
void MainWindow::initUI() {
    setWindowTitle("QtStock");                                // 窗口标题
    resize(1200, 700);                                        // 窗口大小

    // K线图标签页
    m_chartTab->addTab(m_klineView, "K线图");                  // 添加K线标签页

    // 左侧面板：自选股列表 + 输入框
    auto* leftLayout = new QVBoxLayout;                       // 左侧垂直布局
    auto* inputLayout = new QHBoxLayout;                      // 输入区域水平布局
    m_codeInput->setPlaceholderText("输入股票代码");           // 提示文字
    inputLayout->addWidget(m_codeInput);                      // 添加输入框
    inputLayout->addWidget(m_addBtn);                         // 添加按钮
    inputLayout->addWidget(m_removeBtn);                      // 删除按钮
    leftLayout->addLayout(inputLayout);                       // 添加输入区域
    leftLayout->addWidget(m_stockList);                       // 添加列表

    auto* leftWidget = new QWidget;                           // 左侧容器
    leftWidget->setLayout(leftLayout);                        // 设置布局
    leftWidget->setFixedWidth(500);                           // 固定宽度

    // 使用分割器布局
    auto* splitter = new QSplitter(Qt::Horizontal);           // 水平分割器
    splitter->addWidget(leftWidget);                          // 左侧
    splitter->addWidget(m_chartTab);                          // 右侧图表
    splitter->setStretchFactor(0, 0);                         // 左侧不拉伸
    splitter->setStretchFactor(1, 1);                         // 右侧拉伸

    setCentralWidget(splitter);                               // 设置中心部件

    // 连接按钮信号
    connect(m_addBtn, &QPushButton::clicked,                  // 添加按钮
            this, &MainWindow::onAddStock);
    connect(m_removeBtn, &QPushButton::clicked,               // 删除按钮
            this, &MainWindow::onRemoveStock);
}

// 初始化菜单栏
void MainWindow::initMenuBar() {
    auto* fileMenu = menuBar()->addMenu("文件(&F)");          // 文件菜单
    fileMenu->addAction("退出(&X)", this, &QWidget::close);   // 退出

    auto* viewMenu = menuBar()->addMenu("视图(&V)");          // 视图菜单
    viewMenu->addAction("刷新K线", this, &MainWindow::onRefreshKLine); // 刷新K线

    // 技术指标菜单
    initIndicatorMenu();                                      // 初始化指标菜单

    auto* helpMenu = menuBar()->addMenu("帮助(&H)");          // 帮助菜单
    helpMenu->addAction("关于", [this]() {                    // 关于对话框
        QMessageBox::about(this, "关于",
            "QtStock v1.0\n股票行情软件\n\n"
            "功能: 行情显示/K线图/技术分析\n"
            "数据源: 新浪财经\n"
            "技术栈: Qt 5.14.2 + C++17");
    });
}

// 初始化工具栏
void MainWindow::initToolBar() {
    auto* toolbar = addToolBar("主工具栏");                    // 添加工具栏
    toolbar->addAction("刷新K线", this, &MainWindow::onRefreshKLine); // 刷新按钮
}

// 初始化状态栏
void MainWindow::initStatusBar() {
    statusBar()->addWidget(m_statusLabel);                    // 添加状态标签
}

// 加载自选股（从CSV文件读取）
void MainWindow::loadDefaultStocks() {
    // CSV文件路径（与可执行文件同目录下的 data/stocks.csv）
    QString csvPath = QCoreApplication::applicationDirPath() + "/data/stocks.csv"; // CSV路径

    // 尝试从CSV文件加载
    QStringList stocks = CsvLoader::loadStocks(csvPath);      // 从CSV加载

    if (stocks.isEmpty()) {                                    // CSV不存在或为空
        m_statusLabel->setText("未找到 data/stocks.csv，无自选股。请创建CSV文件添加股票代码。");
        return;                                                // 不加载任何股票
    }

    m_engine->setStockList(stocks);                            // 设置自选股
    m_stockList->showCodes(stocks);                            // 立即在表格中显示代码
    m_statusLabel->setText(QString("从CSV加载 %1 只自选股").arg(stocks.size())); // 更新状态
}

// 行情更新槽
void MainWindow::onQuotesUpdated(const QList<StockQuote>& quotes) {
    if (quotes.isEmpty()) return;                             // 空数据不清空表格
    m_stockList->updateQuotes(quotes);                        // 更新列表显示
    m_statusLabel->setText(QString("行情更新: %1 只股票 %2")
        .arg(quotes.size())
        .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))); // 更新状态栏
}

// K线接收槽
void MainWindow::onKLinesReceived(const QString& code,
                                   const QList<KLine>& klines) {
    if (klines.isEmpty()) {                                    // 空数据给用户提示
        m_klineView->clear();                                  // 清空K线图
        m_klineView->setTitle(code + " - 无K线数据");          // 提示标题
        m_statusLabel->setText(QString("%1 K线数据为空,请检查网络连接").arg(code)); // 状态栏提示
        return;                                                // 不更新视图
    }
    m_klineView->setKLines(klines);                           // 设置K线数据
    m_klineView->setTitle(code + " 日K线");                   // 设置标题
    m_statusLabel->setText(QString("K线加载: %1 共%2根")
        .arg(code).arg(klines.size()));                       // 更新状态栏
}

// 股票选中槽(单击即请求K线,符合行情软件习惯)
void MainWindow::onStockSelected(const QString& code) {
    m_currentCode = code;                                     // 保存当前代码
    m_engine->requestKLines(code, "day", 200);                // 单击即请求日K线
    m_statusLabel->setText(QString("正在加载 %1 K线...").arg(code)); // 更新状态
}

// 双击槽：请求K线
void MainWindow::onStockDoubleClicked(const QString& code) {
    m_currentCode = code;                                     // 保存当前代码
    m_engine->requestKLines(code, "day", 200);                // 请求日K线
    m_statusLabel->setText(QString("正在加载 %1 K线...").arg(code)); // 更新状态
}

// 添加自选股
void MainWindow::onAddStock() {
    QString code = m_codeInput->text().trimmed();             // 获取输入代码
    if (code.isEmpty()) return;                               // 空输入不处理
    m_engine->addStock(code);                                 // 添加到引擎
    m_stockList->showCodes(m_engine->getStockList());         // 立即更新表格显示
    m_engine->refreshNow();                                   // 立即刷新行情
    m_codeInput->clear();                                     // 清空输入框
    m_statusLabel->setText(QString("已添加: %1").arg(code));  // 更新状态
}

// 删除自选股
void MainWindow::onRemoveStock() {
    if (m_currentCode.isEmpty()) {                            // 未选中提示
        QMessageBox::warning(this, "提示", "请先选中要删除的股票");
        return;
    }
    m_engine->removeStock(m_currentCode);                     // 从引擎删除
    m_stockList->showCodes(m_engine->getStockList());         // 立即更新表格
    m_statusLabel->setText(QString("已删除: %1").arg(m_currentCode)); // 更新状态
    m_currentCode.clear();                                    // 清空当前代码
}

// 刷新K线
void MainWindow::onRefreshKLine() {
    if (m_currentCode.isEmpty()) {                            // 未选中提示
        QMessageBox::warning(this, "提示", "请先双击选择一只股票");
        return;
    }
    m_engine->requestKLines(m_currentCode, "day", 200);       // 请求K线
    m_statusLabel->setText(QString("正在刷新 %1 K线...").arg(m_currentCode)); // 更新状态
}

// 初始化指标菜单
void MainWindow::initIndicatorMenu() {
    auto* indicatorMenu = menuBar()->addMenu("指标(&I)");     // 指标菜单

    // 主图指标子菜单
    auto* mainMenu = indicatorMenu->addMenu("主图指标");       // 主图指标
    for (const QString& name : {"MA", "EMA", "BOLL", "无"}) {
        mainMenu->addAction(name, [this, name]() {            // 添加菜单项
            onSelectMainIndicator(name);                       // 选择主图指标
        });
    }

    // 副图指标子菜单
    auto* subMenu = indicatorMenu->addMenu("副图指标");        // 副图指标
    for (const QString& name : {"MACD", "KDJ", "RSI", "WR", "OBV", "ATR", "无"}) {
        subMenu->addAction(name, [this, name]() {             // 添加菜单项
            onSelectSubIndicator(name);                        // 选择副图指标
        });
    }
}

// 选择主图指标
void MainWindow::onSelectMainIndicator(const QString& name) {
    if (name == "无") {                                        // 清除指标
        m_klineView->clearMainIndicator();                     // 清除
        m_statusLabel->setText("已清除主图指标");               // 状态
        return;
    }

    IndicatorBase* indicator = nullptr;                       // 指标指针
    if (name == "MA") {                                        // 均线
        indicator = new MA();                                  // 创建MA
    } else if (name == "EMA") {                                // 指数均线
        indicator = new EMA();                                 // 创建EMA
    } else if (name == "BOLL") {                               // 布林带
        indicator = new BOLL();                                // 创建BOLL
    }

    if (indicator) {                                           // 有效指标
        m_klineView->setMainIndicator(indicator);              // 设置主图指标
        m_statusLabel->setText(QString("主图指标: %1").arg(indicator->params())); // 状态
    }
}

// 选择副图指标
void MainWindow::onSelectSubIndicator(const QString& name) {
    if (name == "无") {                                        // 清除指标
        m_klineView->clearSubIndicator();                      // 清除
        m_statusLabel->setText("已清除副图指标");               // 状态
        return;
    }

    IndicatorBase* indicator = nullptr;                       // 指标指针
    if (name == "MACD") {                                      // MACD
        indicator = new MACD();                                // 创建MACD
    } else if (name == "KDJ") {                                // KDJ
        indicator = new KDJ();                                 // 创建KDJ
    } else if (name == "RSI") {                                // RSI
        indicator = new RSI();                                 // 创建RSI
    } else if (name == "WR") {                                 // 威廉指标
        indicator = new WR();                                  // 创建WR
    } else if (name == "OBV") {                                // 能量潮
        indicator = new OBV();                                 // 创建OBV
    } else if (name == "ATR") {                                // 真实波幅
        indicator = new ATR();                                 // 创建ATR
    }

    if (indicator) {                                           // 有效指标
        m_klineView->setSubIndicator(indicator);               // 设置副图指标
        m_statusLabel->setText(QString("副图指标: %1").arg(indicator->params())); // 状态
    }
}
