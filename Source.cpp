#include "C:\Users\Nikita\source\repos\TX\TXLib.h"

#include <iostream> 
#include <algorithm>
#include <ctime>
#include <fstream>
#include <string.h>

typedef void (*func_t) (void);


class Button {
public:
	const char* name_;
	RECT rect_;
	func_t func_;

	Button(const char* name, RECT rect, func_t func) :
		name_(name),
		rect_(rect),
		func_(func)
	{};

	virtual void draw_button() {
		txSetColor(TX_RED);
		txLine(rect_.left, rect_.top, rect_.right, rect_.bottom);
		txLine(rect_.left, rect_.bottom, rect_.right, rect_.top);
		txSetColor(TX_WHITE);
	}

	virtual bool is_mouse_on_button() {
		double x = txMouseX(), y = txMouseY();

		if (x == rect_.right + 0.5 * (rect_.right - rect_.left) && y == rect_.top + 0.5 * (rect_.bottom - rect_.top)) return TRUE;

		return FALSE;
	}

	virtual void pressed() {
		std::cout << "Button is pressed!";
	};
};

class RectButton : public Button {
public:
	RectButton(const char* name, RECT rect, func_t func) :
		Button(name, rect, func) {}

	virtual void draw_button() override {
		txSetFillColor(TX_WHITE);
		txRectangle(rect_.left, rect_.top, rect_.right, rect_.bottom);

		txSetColor(TX_BLACK);
		txDrawText(rect_.left, rect_.top, rect_.right, rect_.bottom, name_);
	}

	virtual bool is_mouse_on_button() override {
		double x = txMouseX(), y = txMouseY();

		if (x >= rect_.left && x <= rect_.right && y >= rect_.top && y <= rect_.bottom) return TRUE;

		return FALSE;
	}
};

class CircleButton : public Button {
public:
	CircleButton(const char* name, RECT rect, func_t func) :
		Button(name, rect, func) {}

	virtual void draw_button() override {
		txSetFillColor(TX_WHITE);
		txEllipse(rect_.left, rect_.top, rect_.right, rect_.bottom);

		txSetColor(TX_BLACK);
		txDrawText(rect_.left, rect_.top, rect_.right, rect_.bottom, name_);
	}

	virtual bool is_mouse_on_button() override {
		double R = 0.5 * (rect_.right - rect_.left);
		double x = txMouseX(), y = txMouseY(), x0 = rect_.left + R, y0 = rect_.top + R;

		if ((x - x0) * (x - x0) + (y - y0) * (y - y0) <= R * R)	return TRUE;

		return FALSE;
	}
};


class Canvas : public RectButton {
public:
	Canvas(const char* name, RECT rect, func_t func) :
		RectButton(name, rect, func) {}

	virtual void pressed() override;

	void pencil();

};

void Canvas::pressed() {
	pencil();
}

class Manager {
private:
	static const size_t Number_of_buttons = 50;

public:
	size_t count_ = 0;
	Button* buttons_[Number_of_buttons] = {};

	Manager() {
		for (size_t button = 0; button < Number_of_buttons; button++)
			buttons_[button] = NULL;
	}

	void add(Button* button);
	void draw();
	void run();

	~Manager() {
		for (size_t button = 0; button < count_; button++)
			delete buttons_[button];
	}
};


//template <size_t N>

void Manager::draw() {
	txSetFillColor(RGB(40, 40, 40));
	txClear();
	

	for (size_t button = 0; button < count_; button++)
		buttons_[button]->draw_button();
}

void Manager::add(Button* button) {
	buttons_[count_] = button;
	count_++;
}

void Manager::run() {
	while (!txGetAsyncKeyState(VK_ESCAPE)) {
		if (txMouseButtons() == 1)
			for (size_t button = 0; button < count_; button++)
				if (buttons_[button]->is_mouse_on_button()) {
					//draw();
					buttons_[button]->pressed();
				}
	}
}

void Canvas::pencil() {
	double x0 = txMouseX(), y0 = txMouseY(), x1 = 0, y1 = 0;

	while (txMouseButtons() == 1) {
		txSleep();

		x1 = txMouseX();
		y1 = txMouseY();

		if (!is_mouse_on_button()) {
			while (txMouseButtons() == 1)
				if (is_mouse_on_button())
					break;

			x0 = txMouseX();
			y0 = txMouseY();

			continue;
		}

		if (x0 != x1 || y0 != y1) {
			txLine(x0, y0, x1, y1);

			x0 = x1;
			y0 = y1;
		}

	}
}



int main() {
	int width = 800, height = 600;
	txCreateWindow(width, height);
	txSetColor(TX_BLACK);


	Manager manager;

	manager.add(new Canvas("", RECT{ 50, 50, 780, 580 }, NULL));
	manager.add(new RectButton("clear", RECT{ 10, 10, 60, 60 }, manager.buttons_[0]->draw_button));

	/*
	manager.add(new RectButton("SIN", RECT{ 100, 480, 300, 520 }, sin));
	manager.add(new CircleButton("COS", RECT{ 175, 540, 225, 590 }, cos));
	manager.add(new EllipseButton("TAN", RECT{ 500, 480, 700, 520 }, tan));
	manager.add(new Button("EXP", RECT{ 500, 540, 700, 580 }, exp));
	*/

	manager.draw();

	manager.run();

	txDisableAutoPause();

}
