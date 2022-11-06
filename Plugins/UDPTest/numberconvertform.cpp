#include "numberconvertform.h"
#include "ui_numberconvertform.h"
#include <QFile>
#include <QDebug>
#include <QAxObject>
#include <QDir>
#include <QScrollBar>
#include <QMetaEnum>
#include <QTextBlock>

NumberConvertForm::NumberConvertForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NumberConvertForm)
{
    ui->setupUi(this);

//    ui->tableWidget->clear();
//    // 只设置列数，行数动态增加
//    ui->tableWidget->setColumnCount(6);
    // 设置整行选中的方式
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    // 设置不可编辑
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置可以选中单行
    ui->tableWidget->setSelectionMode(QAbstractItemView::/*ExtendedSelection*/SingleSelection);
    //设置无边框
//    ui->tableWidget->setFrameShape(QFrame::NoFrame);
    //设置不显示格子线
//    ui->tableWidget->setShowGrid(false);
    //设置行号列,true为显示
//    ui->tableWidget->verticalHeader()->setVisible(false);
    // 设置表头高度
    ui->tableWidget->horizontalHeader()->setFixedHeight(30);
    // 设置选中行背景色
//    ui->tableWidget->setStyleSheet("selection-background-color:blue");
    // 设置表头 背景色
//    ui->tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{background:red;}");

    //设置水平、垂直滚动条样式,添加头文件 #include <QScrollBar>
    ui->tableWidget->horizontalScrollBar()->setStyleSheet("QScrollBar{background:transparent; height:10px;}"
                                                          "QScrollBar::handle{background:lightgray;"
                                                          "border:2px solid transparent; border-radius:5px;}"
                                                          "QScrollBar::handle:hover{background:gray;}"
                                                          "QScrollBar::sub-line{background:transparent;}"
                                                          "QScrollBar::add-line{background:transparent;}");
    ui->tableWidget->verticalScrollBar()->setStyleSheet("QScrollBar{background:transparent; width: 10px;}"
                                                        "QScrollBar::handle{background:lightgray;"
                                                        "border:2px solid transparent; border-radius:5px;}"
                                                        "QScrollBar::handle:hover{background:gray;}"
                                                        "QScrollBar::sub-line{background:transparent;}"
                                                        "QScrollBar::add-line{background:transparent;}");
    // 调整内容大小
//    ui->tableWidget->resizeRowsToContents();
    // 标题头的大小
//    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(200);
    // 横向先自适应宽度
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //设置标题头的字体样式
    QFont font = ui->tableWidget->horizontalHeader()->font();
    font.setBold(true);
    ui->tableWidget->horizontalHeader()->setFont(font);
    //设置充满表宽度
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    //设置行距
//    ui->tableWidget->verticalHeader()->setDefaultSectionSize(10);

    // 由于读取Excel文件速度慢，所以打开窗体前读取CRC配置文件一次，保存到Vector中
    QString fileName = tr("%1/config/data/CRC.xlsx").arg(QDir::currentPath());
    QFile file(fileName);
    if(!file.exists())
    {
        qInfo().noquote() << fileName << "文件不存在！";
        return;
    }

    // Excel应用程序
    QAxObject *excel = new QAxObject("Excel.Application");
    // true：操作文件时Excel可见，false：操作文件时Excel不可见
    excel->dynamicCall("SetVisible(bool)", false);
    // 所有Excel文件
    QAxObject *workBooks = excel->querySubObject("WorkBooks");
    // 按照路径获取文件
    QAxObject *workBook = workBooks->querySubObject("Open(QString&）", fileName);
    // 获取文件的所有Sheet页
    QAxObject *sheets = workBook->querySubObject("WorkSheets");
    // 获取文件Sheet页
    QAxObject *sheet = sheets->querySubObject("Item(QString)", "CRC16");
    if(nullptr == sheet)
    {
        qInfo().noquote() << "CRC16 Sheet页不存在";
        return;
    }
    // 有数据的矩形区域
    QAxObject *usedRange = sheet->querySubObject("UsedRange");
    // 获取Sheet中数据，并保存到QVector<QVector<QString>>中
    QVariant var = usedRange->dynamicCall("Value");
    // 保存CRC16配置参数
    foreach(QVariant varRow, var.toList())
    {
        QVector<QString> vecDataRow;
        foreach(QVariant var, varRow.toList())
        {
            vecDataRow.push_back(var.toString());
        }
        crc16Data.push_back(vecDataRow);
    }

    // 初始化CRC8算法配置QVector
    sheet = sheets->querySubObject("Item(QString)", "CRC8");
    if(nullptr == sheet)
    {
        qInfo().noquote() << "CRC8 Sheet页不存在";
        return;
    }
    // 有数据的矩形区域
    usedRange = sheet->querySubObject("UsedRange");
    // 获取Sheet中数据，并保存到QVector<QVector<QString>>中
    var = usedRange->dynamicCall("Value");
    // 保存CRC8配置参数
    foreach(QVariant varRow, var.toList())
    {
        QVector<QString> vecDataRow;
        foreach(QVariant var, varRow.toList())
        {
            vecDataRow.push_back(var.toString());
        }
        crc8Data.push_back(vecDataRow);
    }

    // 初始化CRC32算法配置QVector
    sheet = sheets->querySubObject("Item(QString)", "CRC32");
    if(nullptr == sheet)
    {
        qInfo().noquote() << "CRC32 Sheet页不存在";
        return;
    }
    // 有数据的矩形区域
    usedRange = sheet->querySubObject("UsedRange");
    // 获取Sheet中数据，并保存到QVector<QVector<QString>>中
    var = usedRange->dynamicCall("Value");
    // 保存CRC32配置参数
    foreach(QVariant varRow, var.toList())
    {
        QVector<QString> vecDataRow;
        foreach(QVariant var, varRow.toList())
        {
            vecDataRow.push_back(var.toString());
        }
        crc32Data.push_back(vecDataRow);
    }

    // 关闭文件
    workBook->dynamicCall("Close()");
    excel->dynamicCall("Quit()");
    if(excel)
    {
        delete excel;
        excel = nullptr;
    }

    //******************************//
    QStringList byteLengthList = {"1子节", "2子节", "4子节"};
    ui->comboBox_ChecksumLength->blockSignals(true);
    ui->comboBox_ChecksumLength->addItems(byteLengthList);
    ui->comboBox_ChecksumLength->blockSignals(false);
    ui->comboBox_ChecksumLength->setCurrentIndex(1);

    //zjk 20221102
    // 初始化字节序单选按钮分组
    sendModeGroup = new QButtonGroup(this);
    sendModeGroup->addButton(ui->radioButton_SmallStorage, 0);
    sendModeGroup->addButton(ui->radioButton_BigStorage, 1);
    // 初始设置小端存储
    ui->radioButton_SmallStorage->setChecked(true);
    // 绑定信号与槽
    connect(ui->radioButton_SmallStorage, SIGNAL(clicked()), this, SLOT(onRadioClickSelecByteOrder()));
    connect(ui->radioButton_BigStorage, SIGNAL(clicked()), this, SLOT(onRadioClickSelecByteOrder()));

    // 文本框中显示提示信息
    ui->textEdit_ByteString->setPlaceholderText("请输出十六进制字符数据！");

}

NumberConvertForm::~NumberConvertForm()
{
    delete ui;
}

// 显示CRC校验算法参数配置列表 20221103
void NumberConvertForm::on_pushButton_ShowCRCParams_clicked()
{
    ui->tableWidget->clear();
    // 删除表格所有行
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->clearContents();

//    ui->tableWidget->removeRow()
    // 调整内容大小
//    ui->tableWidget->resizeRowsToContents();
    // 标题头的大小
    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(200);
    // 横向先自适应宽度
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //设置标题头的字体样式
    QFont font = ui->tableWidget->horizontalHeader()->font();
    font.setBold(true);
    ui->tableWidget->horizontalHeader()->setFont(font);
    //设置充满表宽度
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    //设置行距
//    ui->tableWidget->verticalHeader()->setDefaultSectionSize(10);

    // 行数、列数
    const quint16 rowCount = crc16Data.size();
    quint16 columnCount = 0;
    if(rowCount > 0)
        columnCount = crc16Data[0].size();
    // 只设置列数，行数动态增加
    ui->tableWidget->setColumnCount(columnCount-1);
    quint16 rowNo = 0;
    bool headerFlag = true;
    for(auto i=0; i<rowCount; ++i)
    {
        QStringList header;
        for(auto j=0; j<columnCount-1; ++j)
        {
            // 读取文件首行作为表头
            if(headerFlag)
            {
                //设置标题头的文字
                header<< crc16Data[i][j];
                if(j < 5)
                    continue;
                else
                {
                    ui->tableWidget->setHorizontalHeaderLabels(header);
                    headerFlag = false;
                }
            }
            else
            { // 从第2行开始插入表格内容
                // 处理行首元素时插入新行
                if(j == 0)
                    ui->tableWidget->insertRow(rowNo);
                ui->tableWidget->setItem(rowNo, j, new QTableWidgetItem(crc16Data[i][j]));
                if(j > 0)
                    //内容水平垂直居中 Note:要先设置内容，该属性才有效
                    ui->tableWidget->item(rowNo, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                if(j == 5)
                { // 当前行处理完毕，行号+1
                    ++rowNo;
                }
            }
        }
    }
    // 然后设置要根据内容适应宽度的列。Note：需要在表格添加内容之后再设置
//    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    //清除表格数据区的内容，但不清除表头
//    ui->tableWidget->clearContents();
}

// 初始化校验算法下拉列表框
void NumberConvertForm::on_comboBox_ChecksumLength_currentIndexChanged(int index)
{
    ui->comboBox_CheckAlgorithm->clear();
    QMetaEnum me;
    switch (index)
    {
    case 0:
        // 获得枚举类型的元数据对象
        me = QMetaEnum::fromType<NumberConvertForm::CRC8_Mode>();
        for(auto i = 0; i < me.keyCount(); ++i)
            ui->comboBox_CheckAlgorithm->addItem(me.key(i));
        // 显示支持的CRC8算法配置列表
        DisPlay_CRC8_Configation_List();
        break;
    case 1:
        me = QMetaEnum::fromType<NumberConvertForm::CRC16_Mode>();
        for(auto i = 0; i < me.keyCount(); ++i)
            ui->comboBox_CheckAlgorithm->addItem(me.key(i));
        // 显示支持的CRC16算法列表
        DisPlay_CRC16_Configation_List();
        break;
    case 2:
        me = QMetaEnum::fromType<NumberConvertForm::CRC32_Mode>();
        for(auto i = 0; i < me.keyCount(); ++i)
            ui->comboBox_CheckAlgorithm->addItem(me.key(i));
        // 显示支持的CRC32算法配置列表
        DisPlay_CRC32_Configation_List();
        break;
    default:
        break;
    }
}


// 产生校验码函数  20221102
void NumberConvertForm::on_pushButton_Generate_Checkcode_clicked()
{
    QString hexStr = ui->textEdit_ByteString->toPlainText();
    QByteArray ba = tcInstance.HexStringToByteArray(hexStr);
    NumberConvertForm& pCRC = NumberConvertForm::getDCFInstance();
    char* pCrcCheckSum = ba.data();    // QByteArray转char*
    quint8 ret8 = 0;
    quint16 ret16 = 0;
    quint32 ret32 = 0;
    QString methodName = ui->comboBox_CheckAlgorithm->currentText().toUpper();
    bool callResult = false;
    switch (ui->comboBox_ChecksumLength->currentIndex())
    {
    case 0: // 处理1字节校验码
        callResult = QMetaObject::invokeMethod(&pCRC, methodName.toLatin1().data(), Qt :: AutoConnection,
                                               Q_RETURN_ARG(quint8, ret8),
                                               Q_ARG(char*, pCrcCheckSum), Q_ARG(quint16, ba.size()));
        break;
    case 1: // 处理2字节校验码
        // Note：被调用QMetaObject::invokeMethod的参数类型必须严格一致。Q_ARG()宏不识别quint8类型，需要使用char
        callResult = QMetaObject::invokeMethod(&pCRC, methodName.toLatin1().data(), Qt :: AutoConnection,
                                               Q_RETURN_ARG(quint16, ret16),
                                               Q_ARG(char*, pCrcCheckSum), Q_ARG(quint16, ba.size()));
        break;
    case 2: // 处理4字节校验码
        qRegisterMetaType<quint32>("quint32");
        callResult = QMetaObject::invokeMethod(&pCRC, methodName.toLatin1().data(), Qt :: AutoConnection,
                                               Q_RETURN_ARG(quint32, ret32),
                                               Q_ARG(char*, pCrcCheckSum), Q_ARG(quint16, ba.size()));
        break;
    default:
        ;
    }
    if(!callResult)
    {
        ui->lineEdit_Checkcode->setText("调用校验函数失败！");
        return;
    }
    // 根据字节长度返回校验码
    QString checkcode = "";
    switch (ui->comboBox_ChecksumLength->currentIndex())
    {
    case 0:
        qDebug().noquote() << "十进制校验码：" << ret8;
        checkcode = tcInstance.DecToHexString(ret8, 1, !crcByteOrder);
        qDebug().noquote() << "十六进制校验码：" << checkcode;
        ui->lineEdit_Checkcode_Dec->setText(QString::number(ret8));
        break;
    case 1:
        qDebug().noquote() << "十进制校验码：" << ret16;
        checkcode = tcInstance.DecToHexString(ret16, 2, !crcByteOrder);
        qDebug().noquote() << "十六进制校验码：" << checkcode;
        ui->lineEdit_Checkcode_Dec->setText(QString::number(ret16));
        break;
    case 2:
        qDebug().noquote() << "十进制校验码：" << ret32;
        checkcode = tcInstance.DecToHexString(ret32, 4, !crcByteOrder);
        qDebug().noquote() << "十六进制校验码：" << checkcode;
        ui->lineEdit_Checkcode_Dec->setText(QString::number(ret32));
        break;
    default:
        ;
    }
    ui->lineEdit_Checkcode->setText(checkcode);
    ui->textEdit_ByteString->setText(tcInstance.StringNoNullToNull(hexStr+checkcode));

}

void NumberConvertForm::onRadioClickSelecByteOrder()
{
    // 通过ID来获取选中的radioButton的方法
    int idNumber = sendModeGroup->checkedId();
    switch (idNumber)
    {
    case 0: // 小端字节序存储：低在前高在后
        crcByteOrder = false;
        break;
    case 1:
        crcByteOrder = true;
        break;
    default:
        crcByteOrder = false;
        break;
    }
}

void NumberConvertForm::on_checkBox_FormatData_stateChanged(int arg1)
{
    QString sendStr = ui->textEdit_ByteString->toPlainText();
    if(arg1 == 2)
    {
        QByteArray ba = tcInstance.HexStringToByteArray(sendStr);
        sendStr = ba.toHex(' ').data();
    }
    else if(arg1 == 0)
    {
        sendStr = sendStr.remove(" ");
    }
    ui->textEdit_ByteString->setText(sendStr.toUpper());
}


void NumberConvertForm::on_textEdit_ByteString_textChanged()
{
    QTextCursor cursor = ui->textEdit_ByteString->textCursor();   // 光标
    quint16 lineNum = cursor.blockNumber(); // 行号
    quint16 posInBlock = cursor.positionInBlock(); // 光标所在行的位置，从左边开始计算
    QString lineText = ui->textEdit_ByteString->document()->findBlockByLineNumber(lineNum).text();

    /* 按回车会新开一行，如果判断光标在行首位置且行号大于0则认为新开一行，把光标向下移动一行*/
    if(posInBlock == 0 && lineNum > 0)
    {
        cursor.movePosition(QTextCursor::Down);
        ui->textEdit_ByteString->setTextCursor(cursor);
        return;
    }
    char ch = ' ';
    // 光标所在行的位置大于等于1，才取前一个字符，这是为了预防按下Backspace键回退到一行开始处的位置，会导致lineText.at(posInBlock-1)访问越界
    if(posInBlock >= 1)
    {
        ch = lineText.at(posInBlock-1).toLatin1();
    }
    if((ch == ' ') || (ch >= '0' && ch <='9') || (ch >='a' && ch <= 'f') || (ch >='A' && ch <= 'F'))
    {
        return;
    }
    else
    {
        cursor.deletePreviousChar();
        ui->textEdit_ByteString->setTextCursor(ui->textEdit_ByteString->textCursor());
    }
}

quint16 NumberConvertForm::CRC16_USB(char *data, quint16 dataLen)
{
    quint16 initValue = 0xFFFF;
    quint16 poly = 0x8005;

    InvertUint16(&poly, &poly);
    while (dataLen--)
    {
        initValue ^= *(data++);
        for(auto i = 0; i < 8; i++)
        {
            if(initValue & 0x01)
            {
                initValue = (initValue >> 1) ^ poly;
            }
            else
            {
                initValue = initValue >> 1;
            }
        }
    }
    return (initValue ^ 0xFFFF);

}

quint16 NumberConvertForm::CRC16_DNP(char *data, quint16 dataLen)
{
    quint16 initValue = 0x0000;
    quint16 poly = 0x3D65;

    InvertUint16(&poly, &poly);
    while (dataLen--)
    {
        initValue ^= *(data++);
        for(int i = 0;i < 8;i++)
        {
            if(initValue & 0x01)
            {
                initValue = (initValue >> 1) ^ poly;
            }
            else
            {
                initValue = (initValue >> 1);
            }
        }
    }

    return (initValue ^ 0xFFFF);
}

quint32 NumberConvertForm::CRC32_WINRAR(char *data, quint16 dataLen)
{
    quint32 initValue = 0xFFFFFFFF;
    quint32 poly = 0x04C11DB7;

    InvertUint32(&poly, &poly);
    while (dataLen--)
    {
        initValue ^= *(data++);
        for(int i = 0;i < 8;i++)
        {
            if(initValue & 0x01)
            {
                initValue = (initValue >> 1) ^ poly;
            }
            else
            {
                initValue = initValue >> 1;
            }
        }
    }

    return (initValue ^ 0xFFFFFFFF) ;
}

quint16 NumberConvertForm::CRC16_IBM(char *data, quint16 dataLen)
{
    quint16 initValue = 0x0000;
    quint16 poly = 0x8005;

    InvertUint16(&poly, &poly);
    while (dataLen--)
    {
        initValue ^= *(data++);
        for(int i = 0;i < 8;i++)
        {
            if(initValue & 0x01)
                initValue = (initValue >> 1) ^ poly;
            else
                initValue = initValue >> 1;
        }
    }
    return initValue;
}

quint16 NumberConvertForm::CRC16_MAXIM(char *data, quint16 dataLen)
{
    quint16 initValue = 0x0000;
    quint16 poly = 0x8005;

    InvertUint16(&poly, &poly);
    while (dataLen--)
    {
        initValue ^= *(data++);
        for(auto i = 0; i < 8; i++)
        {
            if(initValue & 0x01)
            {
                initValue = (initValue >> 1) ^ poly;
            }
            else
            {
                initValue = initValue >> 1;
            }
        }
    }

    return (initValue ^ 0xFFFF);
}

/****************************Info**********************************************
* Name:    CRC-32/MPEG-2  x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1
* Width:	32
* Poly:    0x4C11DB7
* Init:    0xFFFFFFF
* Refin:   False
* Refout:  False
* Xorout:  0x0000000
* Note:
*****************************************************************************/
quint32 NumberConvertForm::CRC32_MPEG(char *data, quint16 dataLen)
{
    quint32 initValue = 0xFFFFFFFF;
    quint32 poly = 0x04C11DB7;
    quint8 uCh = 0;

    while (dataLen--)
    {
        uCh = *(data++);
        initValue ^= (uCh << 24);
        for(auto i = 0; i < 8; i++)
        {
            if(initValue & 0x80000000)
            {
                initValue = (initValue << 1) ^ poly;
            }
            else
            {
                initValue = initValue << 1;
            }
        }
    }

    return initValue;
}

quint8 NumberConvertForm::CRC8(char *data, quint16 dataLen)
{
    quint8 initValue = 0x00;
    quint8 poly = 0x07;

    while (dataLen--)
    {
        initValue ^= *(data++);
        for(auto i = 0; i < 8; i++)
        {
            if(initValue & 0x80)
            {
                initValue = (initValue << 1) ^ poly;
            }
            else
            {
                initValue = initValue << 1;
            }
        }
    }

    return initValue;
}

quint8 NumberConvertForm::CRC8_MAXIM(char *data, quint16 dataLen)
{
    quint8 initValue = 0x00;
    quint8 poly = 0x31;

    InvertUint8(&poly, &poly);
    while (dataLen--)
    {
        initValue ^= *(data++);
        for(auto i = 0; i < 8; i++)
        {
            if(initValue & 0x01)
            {
                initValue = (initValue >> 1) ^ poly;
            }
            else
            {
                initValue = initValue >> 1;
            }
        }
    }

    return initValue;
}

/****************************Info**********************************************
* Name:    CRC-8/ITU           x8+x2+x+1
* Width:	8
* Poly:    0x07
* Init:    0x00
* Refin:   False
* Refout:  False
* Xorout:  0x55
* Alias:   CRC-8/ATM
*****************************************************************************/
quint8 NumberConvertForm::CRC8_ITU(char *data, quint16 dataLen)
{
    quint8 initValue = 0x00;
    quint8 poly = 0x07;

    while (dataLen--)
    {
        initValue ^= *(data++);
        for(auto i = 0;i < 8;i++)
        {
            if(initValue & 0x80)
            {
                initValue = (initValue << 1) ^ poly;
            }
            else
            {
                initValue = initValue << 1;
            }
        }
    }

    return (initValue ^ 0x55);
}

/****************************Info**********************************************
* Name:    CRC-8/ROHC          x8+x2+x+1
* Width:	8
* Poly:    0x07
* Init:    0xFF
* Refin:   True
* Refout:  True
* Xorout:  0x00
* Note:
*****************************************************************************/
quint8 NumberConvertForm::CRC8_ROHC(char *data, quint16 dataLen)
{
    quint8 initValue = 0xFF;
    quint8 poly = 0x07;

    InvertUint8(&poly, &poly);
    while (dataLen--)
    {
        initValue ^= *(data++);
        for(auto i = 0; i < 8; i++)
        {
            if(initValue & 0x01)
            {
                initValue = (initValue >> 1) ^ poly;
            }
            else
            {
                initValue = initValue >> 1;
            }
        }
    }

    return initValue;
}

//这里为了效率，我们不需要将所有Refin和refout为true的输入输出数据移位转换
//只需要将poly二项式转换后，运算时将左移变为右移
quint16 NumberConvertForm::CRC16_CCITT_TRUE(char *data, quint16 dataLen)
{
    quint16 initValue = 0x0000;     // 初始值
    quint16 poly = 0x1021;          // 多项式

    InvertUint16(&poly, &poly);
    while (dataLen--)
    {
        initValue ^= *(data++);
        for(int i = 0;i < 8;i++)
        {
            if(initValue & 0x01)
            {
                initValue = (initValue >> 1) ^ poly;
            }
            else
            {
                initValue = initValue >> 1;
            }
        }
    }

    return initValue;
}

/****************************Info**********************************************
* Name:    CRC-16/CCITT-FALSE   x16+x12+x5+1
* Width:	16
* Poly:    0x1021
* Init:    0xFFFF
* Refin:   False
* Refout:  False
* Xorout:  0x0000
* Note:
*****************************************************************************/
quint16 NumberConvertForm::CRC16_CCITT_FALSE(char *data, quint16 dataLen)
{
    quint16 initValue = 0xFFFF;
    quint16 poly = 0x1021;

    while (dataLen--)
    {
        initValue ^= *(data++) << 8;
        for(int i = 0;i < 8; i++)
        {
            if(initValue & 0x8000)
            {
                initValue = (initValue << 1) ^ poly;
            }
            else
            {
                initValue = initValue << 1;
            }
        }
    }

    return initValue;
}

/****************************Info**********************************************
* Name:    CRC-16/XMODEM       x16+x12+x5+1
* Width:	16
* Poly:    0x1021
* Init:    0x0000
* Refin:   False
* Refout:  False
* Xorout:  0x0000
* Alias:   CRC-16/ZMODEM,CRC-16/ACORN
*****************************************************************************/
quint16 NumberConvertForm::CRC16_XMODEM(char *data, quint16 dataLen)
{
    quint16 initValue = 0x0000;
    quint16 poly = 0x1021;

    while (dataLen--)
    {
        initValue ^= (*(data++) << 8);
        for(int i = 0; i < 8; i++)
        {
            if(initValue & 0x8000)
            {
                initValue = (initValue << 1) ^ poly;
            }
            else
            {
                initValue = initValue << 1;
            }
        }
    }

    return initValue;
}

quint16 NumberConvertForm::CRC16_X25(char *data, quint16 dataLen)
{
    quint16 initValue = 0xFFFF;
    quint16 poly = 0x1021;

    InvertUint16(&poly, &poly);
    while (dataLen--)
    {
        initValue ^= *(data++);
        for(auto i = 0; i < 8; i++)
        {
            if(initValue & 0x01)
            {
                initValue = (initValue >> 1) ^ poly;
            }
            else
            {
                initValue = initValue >> 1;
            }
        }
    }

    return (initValue ^ 0xFFFF);
}

quint16 NumberConvertForm::CRC16_MODBUS(char *data, quint16 dataLen)
{
    quint16 initValue = 0xFFFF;
    quint16 poly = 0x8005;

    InvertUint16(&poly, &poly);
    while (dataLen--)
    {
        initValue ^= *(data++);
        for(int i = 0;i < 8;i++)
        {
            if(initValue & 0x01)
            {
                initValue = (initValue >> 1) ^ poly;
            }
            else
            {
                initValue = initValue >> 1;
            }
        }
    }

    return initValue;
}

// 把quint16数（十六bit）颠倒过来，算法同InvertUint8
void NumberConvertForm::InvertUint16(quint16 *destUShort, quint16 *srcUShort)
{
    quint16 uShort = 0;

    for(auto i = 0; i < 16; i++)
    {
        if(*srcUShort & (1 << i))
        {
            uShort |= 1 << (15 - i);
        }
    }

    *destUShort = uShort;
}

/****************************Info**********************************************
* Name: InvertUint8
* Note: 把字节颠倒过来，如0x12变成0x48
*       0x12: 0001 0010
*       0x48: 0100 1000
*****************************************************************************/
void NumberConvertForm::InvertUint8(quint8 *destUch, quint8 *srcUch)
{
    quint8 uCh = 0;  // 临时变量，暂存翻转后的值

    for(auto i = 0; i < 8; i++)
    {
        if(*srcUch & (1 << i))
        {   // 判断srcUch的第i位，若为1，则翻转后的第7-i位置1
            uCh |= 1 << (7-i);   // 将ch的第7-i位置1
        }
    }

    *destUch = uCh;
}

void NumberConvertForm::InvertUint32(quint32 *destUInt, quint32 *srcUInt)
{
    quint32 uInt = 0;

    for(auto i=0; i< 32; i++)
    {
        if(*srcUInt & (1 << i))
            uInt |= 1 << (31 - i);
    }

    *destUInt = uInt;
}

// 显示CRC8配置列表 20221106
void NumberConvertForm::DisPlay_CRC8_Configation_List()
{
    ui->tableWidget->clear();
    // 删除表格所有行
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->clearContents();

    // 行数、列数
    const quint16 rowCount = crc8Data.size();
    quint16 columnCount = 0;
    if(rowCount > 0)
        columnCount = crc8Data[0].size();
    // 只设置列数，行数动态增加
    ui->tableWidget->setColumnCount(columnCount-1);
    quint16 rowNo = 0;
    bool headerFlag = true;
    for(auto i=0; i<rowCount; ++i)
    {
        QStringList header;
        for(auto j=0; j<columnCount-1; ++j)
        {
            // 读取文件首行作为表头
            if(headerFlag)
            {
                //设置标题头的文字
                header<< crc8Data[i][j];
                if(j < 5)
                    continue;
                else
                {
                    ui->tableWidget->setHorizontalHeaderLabels(header);
                    headerFlag = false;
                }
            }
            else
            { // 从第2行开始插入表格内容
                // 处理行首元素时插入新行
                if(j == 0)
                    ui->tableWidget->insertRow(rowNo);
                ui->tableWidget->setItem(rowNo, j, new QTableWidgetItem(crc8Data[i][j]));
                if(j > 0)
                    //内容水平垂直居中 Note:要先设置内容，该属性才有效
                    ui->tableWidget->item(rowNo, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                if(j == 5)
                { // 当前行处理完毕，行号+1
                    ++rowNo;
                }
            }
        }
    }
}

// 显示CRC校验算法参数配置列表 20221103
void NumberConvertForm::DisPlay_CRC16_Configation_List()
{
    ui->tableWidget->clear();
    // 删除表格所有行
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->clearContents();

    // 行数、列数
    const quint16 rowCount = crc16Data.size();
    quint16 columnCount = 0;
    if(rowCount > 0)
        columnCount = crc16Data[0].size();
    // 只设置列数，行数动态增加
    ui->tableWidget->setColumnCount(columnCount-1);
    quint16 rowNo = 0;
    bool headerFlag = true;
    for(auto i=0; i<rowCount; ++i)
    {
        QStringList header;
        for(auto j=0; j<columnCount-1; ++j)
        {
            // 读取文件首行作为表头
            if(headerFlag)
            {
                //设置标题头的文字
                header<< crc16Data[i][j];
                if(j < 5)
                    continue;
                else
                {
                    ui->tableWidget->setHorizontalHeaderLabels(header);
                    headerFlag = false;
                }
            }
            else
            { // 从第2行开始插入表格内容
                // 处理行首元素时插入新行
                if(j == 0)
                    ui->tableWidget->insertRow(rowNo);
                ui->tableWidget->setItem(rowNo, j, new QTableWidgetItem(crc16Data[i][j]));
                if(j > 0)
                    //内容水平垂直居中 Note:要先设置内容，该属性才有效
                    ui->tableWidget->item(rowNo, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                if(j == 5)
                { // 当前行处理完毕，行号+1
                    ++rowNo;
                }
            }
        }
    }
    // 然后设置要根据内容适应宽度的列。Note：需要在表格添加内容之后再设置
//    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    //清除表格数据区的内容，但不清除表头
//    ui->tableWidget->clearContents();
}

void NumberConvertForm::DisPlay_CRC32_Configation_List()
{
    ui->tableWidget->clear();
    // 删除表格所有行
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->clearContents();

    // 行数、列数
    const quint16 rowCount = crc32Data.size();
    quint16 columnCount = 0;
    if(rowCount > 0)
        columnCount = crc32Data[0].size();
    // 只设置列数，行数动态增加
    ui->tableWidget->setColumnCount(columnCount-1);
    quint16 rowNo = 0;
    bool headerFlag = true;
    for(auto i=0; i<rowCount; ++i)
    {
        QStringList header;
        for(auto j=0; j<columnCount-1; ++j)
        {
            // 读取文件首行作为表头
            if(headerFlag)
            {
                //设置标题头的文字
                header<< crc32Data[i][j];
                if(j < 5)
                    continue;
                else
                {
                    ui->tableWidget->setHorizontalHeaderLabels(header);
                    headerFlag = false;
                }
            }
            else
            { // 从第2行开始插入表格内容
                // 处理行首元素时插入新行
                if(j == 0)
                    ui->tableWidget->insertRow(rowNo);
                ui->tableWidget->setItem(rowNo, j, new QTableWidgetItem(crc32Data[i][j]));
                if(j > 0)
                    //内容水平垂直居中 Note:要先设置内容，该属性才有效
                    ui->tableWidget->item(rowNo, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                if(j == 5)
                { // 当前行处理完毕，行号+1
                    ++rowNo;
                }
            }
        }
    }
}

