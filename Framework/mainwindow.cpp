#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <windows.h>
#include <QJsonParseError>
#include <QJsonObject>
#include <QPluginLoader>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    gridLayout = new QGridLayout();
    // Set the left, top, right and bottom margins
    gridLayout->setContentsMargins(5, 5, 5, 5);
    ui->centralwidget->setLayout(gridLayout);

    gridLayout->addWidget(ui->label, 0, 0, 5, 1);
    ui->label->setText("Welcome to the Bus Interface Test Tool!");
    ui->label->setAlignment(Qt::AlignCenter);
    ui->label->setFrameStyle(QFrame::Box);

    InitGlobalParams();
    LoadPlugins();
    // Set the icon and text of the toolbar button to display together
    ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    // Click to call the plugin
    connect(ui->toolBar, &QToolBar::actionTriggered,
            this, &MainWindow::actionTriggered);

    // Disable maximize button
    this->setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    // Disable window resizing
    this->setFixedSize(this->width(), this->height());

    // Set the status bar display
    topMost = new QPushButton(this);
//    // program path
//    qInfo().noquote() << "QCoreApplication::applicationDirPath() = " << QCoreApplication::applicationDirPath();
//    // current working directory
//    qInfo().noquote() << "QDir::currentPath() = " << QDir::currentPath();
//    // user directory path
//    qInfo().noquote() << "QStandardPaths::writableLocation(QStandardPaths::HomeLocation) = " << QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
//    // desktop path
//    qInfo().noquote() << "QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) = " << QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
//    // Program data storage path
//    qInfo().noquote() << "QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) = " << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
//    // temporary file path
//    qInfo().noquote() << "QStandardPaths::writableLocation(QStandardPaths::TempLocation) = " << QStandardPaths::writableLocation(QStandardPaths::TempLocation);
//    qInfo().noquote() << "QDir::tempPath() = " << QDir::tempPath();
    topMost->setIcon(QIcon(QPixmap("res/png/locked.jpeg")));
    topMost->setStyleSheet("background:transparent;");
    topMost->setIconSize(QSize(20, 20));
    topMost->setToolTip("Set this window to always be displayed at the top.");
    topMost->setFixedSize(20, 20);
    // left top right bottom
    topMost->setContentsMargins(0, 0, 0, 0);
    connect(topMost, SIGNAL(clicked()), this, SLOT(TopMost_Clicked()));
    ui->statusbar->addWidget(topMost);

    // Show temporary message, 2 seconds
    ui->statusbar->showMessage(tr("    Welcome to the Bus Interface Test Tool       Designed by: Jinkun Zhang"), 0);
    // Create a label, style it and display information, add it to the status bar as a permanent widget
    QLabel *label_copyright = new QLabel(this);
    QPushButton *about = new QPushButton(this);
    // Sets the button border to not be displayed unless the button is pressed
    about->setFlat(true);
    about->setStyleSheet("border-image:url(res/png/help.jpeg); \
                         background-position:cener;background-repeat:no-repeat;");
    about->setFixedSize(20, 20);
    connect(about, SIGNAL(clicked()), this, SLOT(About_Clicked()));
    label_copyright->setText("<a href=\"https://github.com/TestingLife-Zhangjinkun/BusTest\">"
                             "Open source project : BusTest</a>");
    label_copyright->setOpenExternalLinks(true);
    ui->statusbar->addPermanentWidget(about);
    ui->statusbar->addPermanentWidget(label_copyright);
    ui->statusbar->setContentsMargins(0, 0, 0, 0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::TopMost_Clicked()
{
    if(!isTopMost)
    {
        // Window top
        ::SetWindowPos(HWND(this->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        topMost->setIcon(QIcon(QPixmap("res/png/unlocked.jpeg")));
        topMost->setToolTip("Cancels the topmost display of this window.");
    }
    else
    {
        // Cancel Window Top
        ::SetWindowPos(HWND(this->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        topMost->setIcon(QIcon(QPixmap("res/png/locked.jpeg")));
        topMost->setToolTip("Set this window to always be displayed at the top.");
    }
    isTopMost = !isTopMost;
    this->show();
}

void MainWindow::About_Clicked()
{
    qInfo().noquote() << "about dialog, processed later."; // 20220809
}

void MainWindow::actionTriggered(QAction *action)
{
    // Traversing the list of test plugins
    foreach(BusTestInterface* ti, tiList)
    {
        if(QString::compare(ti->PluginName(), action->text(), Qt::CaseInsensitive) == 0)
        {
            // If the current test plugin is not a null pointer, hide it;
            // otherwise, it will be cascaded after calling the addWidget() function.
            if(curTi != nullptr)
                curTi->PluginWidget()->hide();
            else
                // hide welcome window
                ui->label->hide();
            gridLayout->setContentsMargins(5, 5, 5, 0);
            gridLayout->addWidget(ti->PluginWidget(), 0, 0, 5, 1);
            ti->PluginWidget()->show();
            // Record the current test plugin
            curTi = ti;
            break;
        }
    }
}

// Parse plugin configuration file
void MainWindow::InitGlobalParams()
{
    QString fileName = tr("%1/config/AppInit.json").arg(QDir::currentPath());
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
    {
        qDebug().noquote() << tr("Failed to open configuration file %1.\nError message: %2")
                        .arg(fileName, file.errorString());
        return ;
    }

    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(file.readAll(), &jsonParserError);
    file.close();

    if(!jsonDocument.isNull() && jsonParserError.error == QJsonParseError::NoError)
    {
        if(jsonDocument.isObject())
        {
            QJsonObject jsonObject = jsonDocument.object();
            if(jsonObject.contains("UDPUnicastFormNum") && jsonObject.value("UDPUnicastFormNum").isDouble())
            {
                udpUnicastFormNum = jsonObject.value("UDPUnicastFormNum").toInt(1);
//                qDebug().noquote() << "udpUnicastFormNum = " << udpUnicastFormNum;
            }
        }
    }
}

quint8 MainWindow::LoadPlugins()
{
    quint8 count = 0;

    // Change to the directory where the plugin is located
    QDir pluginsDir(qApp->applicationDirPath());
    if (pluginsDir.dirName().toLower() == "debug"
            || pluginsDir.dirName().toLower() == "release")
    {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }
    pluginsDir.cd("plugins");
    if(!pluginsDir.cd("plugin"))
        return count;

    foreach (QString fileName, pluginsDir.entryList(QDir::Files))
    {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject* plugin = pluginLoader.instance();
        if(plugin)
        {
            auto testInterface = qobject_cast<BusTestInterface*>(plugin);
            if(testInterface)
            {
                // The framework passes configuration parameters to the plugin
                if(testInterface->PluginName().toLower() == "uart")
                {
//                    testInterface->SetTabNum(uartTabNum);
                }
                else if(testInterface->PluginName().toLower() == "udp")
                    testInterface->SetTabNum(udpUnicastFormNum);
                ++count;
                tiList.append(testInterface);
                // Generate ToolBar after loading the plugin
                AddToolBar(testInterface);
            }
        }
    }
    qInfo().noquote() << tr("Number of loaded plugins：%1").arg(count);
    return count;
}

void MainWindow::AddToolBar(BusTestInterface *ti)
{
    // Toolbar button icons and text
    QAction* action = new QAction(ti->PluginIcon(), ti->PluginName());
    action->setToolTip(ti->PluginTooltip());
    ui->toolBar->addAction(action);
}

