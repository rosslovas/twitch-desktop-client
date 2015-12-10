#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget * parent = nullptr);
	~MainWindow();

	void LoadChannel(const QString & channel);

public slots:
	void UpdateStreams();

private:
	Ui::MainWindow * ui;

private slots:
	void InjectBTTV();
};

#endif // MAINWINDOW_HPP
