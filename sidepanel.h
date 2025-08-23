// sidepanel.h - Version 2.4 (Thêm phím Delete)
#ifndef SIDEPANEL_H
#define SIDEPANEL_H

#include <QWidget>
#include "stylepanel.h" 

class LibraryPanel;
class ViewPanel;
class ExportPanel;
class QListWidgetItem;
class LibraryWidget;

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
    void onDeleteChecked();
    void onDeleteSelection(); // Slot mới cho phím Delete
    void onQuickExportItem(QListWidgetItem* item);
    void onLibraryItemsChanged(QListWidgetItem* item);
    void applyStylesToViewPanel(const StyleOptions& options);
    void onExportClicked();
    void onItemDoubleClicked(QListWidgetItem* item);
    void onViewPanelCrop();

private:
    void setupUi();

    LibraryPanel* m_libraryPanel;
    ViewPanel* m_viewPanel;
    StylePanel* m_stylePanel;
    ExportPanel* m_exportPanel;
};

#endif // SIDEPANEL_H
