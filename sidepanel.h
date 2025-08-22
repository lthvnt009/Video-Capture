// sidepanel.h - Version 2.2 (Sửa lỗi biên dịch)
#ifndef SIDEPANEL_H
#define SIDEPANEL_H

#include <QWidget>
#include "stylepanel.h" 

// --- Forward declarations ---
class LibraryPanel;
class ViewPanel;
class ExportPanel;
class QListWidgetItem;
class LibraryWidget; // THÊM MỚI: Khai báo để getLibraryWidget() hợp lệ

class SidePanel : public QWidget
{
    Q_OBJECT

public:
    explicit SidePanel(QWidget *parent = nullptr);
    
    LibraryWidget* getLibraryWidget() const;
    ViewPanel* getViewPanel() const;
    ExportPanel* getExportPanel() const;

signals:
    void exportImageRequested(const QImage& image);
    void addImagesToLibraryRequested();
    void newImagesDropped(const QList<QUrl>& urls);
    void fileDeleted(const QString& filePath); 

public slots:
    void onCroppedImageSaveFinished(bool success, const QString& filePath, const QImage& savedImage);

private slots:
    void onViewAndCropItem(QListWidgetItem* item);
    void onDeleteItem(QListWidgetItem* item);
    void onQuickExportItem(QListWidgetItem* item);
    void onLibraryItemsChanged(QListWidgetItem* item);
    void applyStylesToViewPanel(const StyleOptions& options);
    void onExportClicked();
    void onItemDoubleClicked(QListWidgetItem* item);
    // THÊM MỚI: Khai báo slot đã thiếu
    void onViewPanelCrop();

private:
    void setupUi();

    // Modules con
    LibraryPanel* m_libraryPanel;
    ViewPanel* m_viewPanel;
    StylePanel* m_stylePanel;
    ExportPanel* m_exportPanel;
};

#endif // SIDEPANEL_H
