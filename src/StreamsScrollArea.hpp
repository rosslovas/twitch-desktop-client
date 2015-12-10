#ifndef STREAMSSCROLLAREA_HPP
#define STREAMSSCROLLAREA_HPP

#include <QScrollArea>

class QGridLayout;

class StreamsScrollArea : public QScrollArea {
	Q_OBJECT

public:
	explicit StreamsScrollArea(QWidget * parent = nullptr);
	~StreamsScrollArea();

	void SetStreamsGrid(QGridLayout * value);

public slots:
	void RearrangeGrid(int columns = 0);

private:
	virtual void resizeEvent(QResizeEvent * event) override;
	QGridLayout * streamsGrid;
};

#endif // STREAMSSCROLLAREA_HPP
