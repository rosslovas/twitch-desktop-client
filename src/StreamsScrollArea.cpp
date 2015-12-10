#include "StreamsScrollArea.hpp"

#include <QDebug>
#include <QGridLayout>
#include <QResizeEvent>

StreamsScrollArea::StreamsScrollArea(QWidget * parent /* = nullptr */) : QScrollArea(parent)
{
}

StreamsScrollArea::~StreamsScrollArea()
{
}

void StreamsScrollArea::SetStreamsGrid(QGridLayout * value)
{
	streamsGrid = value;
}

void StreamsScrollArea::RearrangeGrid(int columns /* = 0 */)
{
	if (streamsGrid) {

		qDebug() << "Rearranging grid";
		const auto itemCount = streamsGrid->count();

		// Decide on the column count. One row width:
		//   3 (padding) + (320 * columns) (streams) + (6 * (columns - 1)) (streams) + 3 (padding)
		// = 3 + 320 * columns + 6 * (columns - 1) + 3
		// = (320 + 6) * columns
		// Therefore, columns = width / 326
		if (columns < 1) {
			columns = width() / 326;
			if (columns < 1) {
				columns = 1;
			}
		}

		// First, retrieve all the items,
		QList<QLayoutItem *> items;
		for (int itemIndex = 0; itemIndex < itemCount; ++itemIndex) {
			auto item = streamsGrid->itemAt(itemIndex);
			items.append(item);
		}

		// Then remove them all from the GridLayout,
		for (auto & item : items) {
			streamsGrid->removeItem(item);
		}

		// Then add them back in at the proper locations.
		for (int itemIndex = 0; itemIndex < itemCount; ++itemIndex) {
			auto item = items[itemIndex];
			streamsGrid->addItem(item, itemIndex / columns, itemIndex % columns);
		}
	}
}

void StreamsScrollArea::resizeEvent(QResizeEvent * event)
{
	if (streamsGrid) {

		auto previousColumns = event->oldSize().width() / 326;
		auto columns = event->size().width() / 326;
		if (previousColumns < 1) {
			previousColumns = 1;
		}
		if (columns < 1) {
			columns = 1;
		}

		if (streamsGrid->count() && previousColumns != columns) {
			RearrangeGrid(columns);
		}
	}
	QScrollArea::resizeEvent(event);
}
