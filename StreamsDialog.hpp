#ifndef STREAMSDIALOG_HPP
#define STREAMSDIALOG_HPP

#include <QDialog>

namespace Ui {
class StreamsDialog;
}

class StreamsDialog : public QDialog {
	Q_OBJECT

public:
	explicit StreamsDialog(QWidget * parent = nullptr);
	~StreamsDialog();

public slots:
	void Update();

private:
	Ui::StreamsDialog * ui;
};

#endif // STREAMSDIALOG_HPP
