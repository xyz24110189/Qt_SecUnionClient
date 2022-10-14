#ifndef WIDGET_H
#define WIDGET_H

#include <QDialog>
#include <QString>

class QPushButton;
class QLineEdit;
class QLabel;
class SelectFileDialog : public QDialog
{
    Q_OBJECT

public:
    SelectFileDialog(QWidget *parent = 0);
    ~SelectFileDialog();

    const QString &GetProgramPath() const{
        return m_strProgram;
    }

protected slots:
    void OnSelectButtonReleased();
    void OnAddButtonReleased();

private:
    void InitWidget();
    void InitConnection();

    QPushButton *m_selectOpenButton;
    QPushButton *m_addProcessButton;
    QLineEdit *m_programLineEdit;
    QLineEdit *m_argsLineEdit;
    QLabel *m_programLabel;
    QLabel *m_argsLabel;

    QString m_strProgram;
};

#endif // WIDGET_H
