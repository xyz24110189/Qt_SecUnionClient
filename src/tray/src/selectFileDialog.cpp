#include "selectFileDialog.h"
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDir>

SelectFileDialog::SelectFileDialog(QWidget *parent)
    : QDialog(parent)
{
    InitWidget();
    InitConnection();
}

SelectFileDialog::~SelectFileDialog()
{

}

void SelectFileDialog::InitWidget()
{
	setWindowFlags(windowFlags() & 
		~Qt::WindowCloseButtonHint & 
		~Qt::WindowContextHelpButtonHint & 
		~Qt::WindowSystemMenuHint);
    m_selectOpenButton = new QPushButton(trUtf8("选择程序"), this);
    m_addProcessButton = new QPushButton(trUtf8("添 加"), this);
    m_programLineEdit  = new QLineEdit(this);
    m_argsLineEdit     = new QLineEdit(this);
    m_programLabel     = new QLabel(trUtf8("程序： "), this);
    m_argsLabel        = new QLabel(trUtf8("参数： "), this);

    m_programLineEdit->setFixedHeight(30);
    m_argsLineEdit->setFixedHeight(30);
    m_selectOpenButton->setFixedHeight(30);
    m_addProcessButton->setFixedSize(200, 35);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *selectLayout = new QHBoxLayout(this);
    selectLayout->addWidget(m_programLabel);
    selectLayout->addWidget(m_programLineEdit);
    selectLayout->addWidget(m_selectOpenButton);

    QHBoxLayout *argsLayout = new QHBoxLayout(this);
    argsLayout->addWidget(m_argsLabel);
    argsLayout->addWidget(m_argsLineEdit);
    argsLayout->addSpacing(80);

    QHBoxLayout *addProLayout = new QHBoxLayout(this);
    addProLayout->addWidget(m_addProcessButton);

    mainLayout->addLayout(selectLayout);
    mainLayout->addLayout(argsLayout);
    mainLayout->addLayout(addProLayout);

    setLayout(mainLayout);
    setFixedSize(500, 300);
    setWindowTitle(trUtf8("设置保活程序"));
}


 void SelectFileDialog::InitConnection()
{
    connect(m_selectOpenButton, SIGNAL(released()),
            this, SLOT(OnSelectButtonReleased()));
    connect(m_addProcessButton, SIGNAL(released()), this, SLOT(OnAddButtonReleased()));
}


void SelectFileDialog::OnSelectButtonReleased()
{
    QString fileName = QFileDialog::getOpenFileName(this,
          tr("程序选择"), QDir::homePath(), tr("程序文件 (*.exe)"));
    m_programLineEdit->setText(fileName);
}

void SelectFileDialog::OnAddButtonReleased()
{
    QString program = m_programLineEdit->text();
    QString args = m_argsLineEdit->text();
	if (program.isEmpty())
	{
		m_strProgram.clear();
		hide();
		return;
	}

    program += "&&";
    program += args;
    m_strProgram = program;

    hide();
}























