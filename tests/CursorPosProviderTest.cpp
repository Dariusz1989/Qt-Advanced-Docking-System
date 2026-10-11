// Tests for the ads::setCursorPosProvider() / ads::cursorPos() seam.
#include <QtTest/QtTest>
#include <QApplication>
#include <QCursor>
#include <QMainWindow>

#include "ads_globals.h"
#include "DockManager.h"
#include "DockWidget.h"
#include "DockAreaWidget.h"
#include "DockContainerWidget.h"
#include "DockOverlay.h"
#include "DockWidgetTab.h"

class TestManager : public ads::CDockManager
{
public:
	using ads::CDockManager::CDockManager;
	using ads::CDockManager::containerOverlay;
};

class CursorPosProviderTest : public QObject
{
	Q_OBJECT
private slots:
	void cleanup() { ads::setCursorPosProvider({}); }

	void defaultIsQCursorPos()
	{
		ads::setCursorPosProvider({});
		QCOMPARE(ads::cursorPos(), QCursor::pos());
	}

	void providerOverridesAndResets()
	{
		const QPoint Fake(12345, 6789);
		ads::setCursorPosProvider([=]() { return Fake; });
		QCOMPARE(ads::cursorPos(), Fake);
		ads::setCursorPosProvider({});
		QCOMPARE(ads::cursorPos(), QCursor::pos());
	}

	// The container overlay must pick the drop area from the provider
	// position, with the real cursor left alone.
	void overlayUsesProvider()
	{
		QMainWindow Window;
		Window.resize(800, 600);
		auto* Manager = new TestManager(&Window);
		auto* A = new ads::CDockWidget("A");
		A->setWidget(new QWidget());
		auto* B = new ads::CDockWidget("B");
		B->setWidget(new QWidget());
		Manager->addDockWidget(ads::CenterDockWidgetArea, A);
		Manager->addDockWidget(ads::CenterDockWidgetArea, B);
		Window.show();
		QVERIFY(QTest::qWaitForWindowExposed(&Window));
		QApplication::processEvents();

		ads::CDockOverlay* Overlay = Manager->containerOverlay();
		Overlay->setAllowedAreas(ads::AllDockAreas);
		QWidget* Target = Manager; // container is the manager itself
		const QRect G = QRect(Target->mapToGlobal(QPoint(0, 0)), Target->size());

		const QPoint Left(G.left() + 20, G.center().y());
		const QPoint Right(G.right() - 20, G.center().y());

		const QPoint RealBefore = QCursor::pos();
		ads::setCursorPosProvider([&]() { return Left; });
		QCOMPARE(Overlay->showOverlay(Target), ads::LeftDockWidgetArea);
		QCOMPARE(Overlay->dropAreaUnderCursor(), ads::LeftDockWidgetArea);
		ads::setCursorPosProvider([&]() { return Right; });
		QCOMPARE(Overlay->showOverlay(Target), ads::RightDockWidgetArea);
		Overlay->hideOverlay();
		QCOMPARE(QCursor::pos(), RealBefore); // the real pointer never moved
	}

	// Drag a tab out into a floating widget with synthetic mouse events and
	// provider positions; the real cursor is never used.
	void tabDragToFloatUsesProvider()
	{
		QMainWindow Window;
		Window.resize(800, 600);
		auto* Manager = new TestManager(&Window);
		auto* A = new ads::CDockWidget("A");
		A->setWidget(new QWidget());
		auto* B = new ads::CDockWidget("B");
		B->setWidget(new QWidget());
		Manager->addDockWidget(ads::CenterDockWidgetArea, A);
		Manager->addDockWidget(ads::CenterDockWidgetArea, B);
		Window.show();
		QVERIFY(QTest::qWaitForWindowExposed(&Window));
		QApplication::processEvents();

		QWidget* Tab = B->tabWidget();
		const QPoint TabCenter = Tab->mapToGlobal(Tab->rect().center());
		QPoint Provided = TabCenter;
		ads::setCursorPosProvider([&]() { return Provided; });

		QTest::mousePress(Tab, Qt::LeftButton, Qt::NoModifier, Tab->rect().center());
		const QPoint Target = Manager->mapToGlobal(QPoint(Manager->width() - 20, Manager->height() / 2));
		for (int i = 1; i <= 20; ++i)
		{
			Provided = TabCenter + (Target - TabCenter) * i / 20;
			QMouseEvent Move(QEvent::MouseMove, Tab->mapFromGlobal(Provided), Provided,
				Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
			QApplication::sendEvent(Tab, &Move);
			QApplication::processEvents();
		}
		const bool Floating = B->isFloating();
		const ads::DockWidgetArea Area = Manager->containerOverlay()->visibleDropAreaUnderCursor();
		QTest::mouseRelease(Tab, Qt::LeftButton, Qt::NoModifier, Tab->mapFromGlobal(Provided));
		QApplication::processEvents();
		qInfo() << "dragging, floating:" << Floating << "area under provider:" << Area;
		QVERIFY(B->dockAreaWidget() != A->dockAreaWidget()); // dropped on the right edge
		QCOMPARE(Area, ads::RightDockWidgetArea);
	}
};

QTEST_MAIN(CursorPosProviderTest)
#include "CursorPosProviderTest.moc"
