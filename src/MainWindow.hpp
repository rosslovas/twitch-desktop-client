#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <vector>

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QShortcut;

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
	std::vector<QShortcut *> shortcuts;

private slots:
	void InjectBTTV();
};

#endif // MAINWINDOW_HPP
