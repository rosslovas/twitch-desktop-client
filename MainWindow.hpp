#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class StreamsDialog;

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget * parent = nullptr);
	~MainWindow();

	void LoadChannel(const QString & channel);

private:
	Ui::MainWindow * ui;
	StreamsDialog * streams;

private slots:
	void InjectBTTV();
};

#endif // MAINWINDOW_HPP
