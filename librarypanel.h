// librarypanel.h - Version 1.2 (Cải tiến chức năng Xóa)
#ifndef LIBRARYPANEL_H
#define LIBRARYPANEL_H

#include <QWidget>
#include "helpers.h" 

class LibraryWidget;
class LibraryItemDelegate;
class QPushButton;
class QListWidgetItem;

class LibraryPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LibraryPanel(QWidget *parent = nullptr);
    LibraryWidget* getLibraryWidget() const;

signals:
    void addImagesClicked();
    void viewAndCropClicked(QListWidgetItem* item);
    void deleteCheckedClicked(); // Thay đổi: Signal để xóa các item đã check
    void quickExportRequested(QListWidgetItem* item);
    void selectionChanged();
    void itemsChanged(QListWidgetItem* item);
    void imagesDropped(const QList<QUrl>& urls);
    void itemDoubleClicked(QListWidgetItem* item);

private slots:
    void updateButtonStates(); // Thêm slot để cập nhật trạng thái các nút

private:
    void setupUi();

    TitleEventFilter* m_titleFilter;
    LibraryWidget* m_libraryWidget;
    LibraryItemDelegate* m_libraryDelegate;
    QPushButton* m_viewAndCropButton;
    QPushButton* m_deleteButton;
    QPushButton* m_addImagesButton;
};

#endif // LIBRARYPANEL_H
