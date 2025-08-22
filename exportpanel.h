// exportpanel.h - Version 1.0
#ifndef EXPORTPANEL_H
#define EXPORTPANEL_H

#include <QWidget>
#include "helpers.h"

class QLineEdit;
class QComboBox;
class QPushButton;

class ExportPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ExportPanel(QWidget *parent = nullptr);
    QString getSavePath() const;
    QString getSelectedFormat() const;

public slots:
    void setSavePath(const QString& path);

signals:
    void exportClicked();

private:
    void setupUi();

    TitleEventFilter* m_titleFilter;
    QLineEdit *m_savePathEdit;
    QComboBox *m_formatComboBox;
    QPushButton *m_exportButton;
};

#endif // EXPORTPANEL_H
