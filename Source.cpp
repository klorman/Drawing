#include "C:\Users\Nikita\source\repos\TX\TXLib.h"

#include <iostream> 
#include <algorithm>
#include <ctime>
#include <fstream>
#include <string.h>
#include <queue>


typedef void (*func_t) (void);


class Button {
public:
	size_t mode_ = 0;
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

	virtual void pressed() {
		func_();
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
	//size_t mode_ = 0;

	Canvas(const char* name, RECT rect, func_t func) :
		RectButton(name, rect, func) {}

	virtual void pressed() override;

	void pencil();
	void spray();
	void fill(COLORREF old_color, std::queue <POINT> points);

};

void Canvas::pressed() {
	switch (mode_)
	{
	case 1:
		pencil();
		break;

	case 2:
		spray();
		break;

	case 3:
		POINT start_pos = txMousePos();
		std::queue <POINT> points;
		points.push(start_pos);

		fill(txGetPixel(start_pos.x, start_pos.y), points);
		break;
	}
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

Manager manager;

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

void Canvas::spray() {
	txSetColor(TX_BLACK);
	txSetFillColor(TX_BLACK);

	int R = 50;

	while (txMouseButtons() == 1) {
		int dist = 0 + rand() % (R - 0), angle = 0 + rand() % (360 - 0);

		double x = txMouseX() + dist * cos(angle), y = txMouseY() + dist * sin(angle);

		if (x - 3 > rect_.left && x + 3 < rect_.right && y - 3 > rect_.top && y + 3 < rect_.bottom)
			txCircle(x, y, 2);

		txSleep();
	}
}

void Canvas::fill(COLORREF old_color, std::queue <POINT> points) {
	std::cout << points.size() << "\n";

	if (points.size() == 0) return; 

	POINT point = points.front();
	points.pop();

	COLORREF color_of_pixel = txGetPixel(point.x, point.y);

	//std::cout << (color_of_pixel == old_color) << "\n";

	if (color_of_pixel != old_color || point.x == rect_.left || point.x == rect_.right || point.y == rect_.bottom || point.y == rect_.top )
		return;


	//std::cout << x << " " << y << "\n";

	txSetPixel(point.x, point.y, TX_GREEN);

	points.push(POINT{ point.x + 1, point.y });
	points.push(POINT{ point.x - 1, point.y });
	points.push(POINT{ point.x, point.y + 1 });
	points.push(POINT{ point.x, point.y - 1 });

	fill(old_color, points);

}

void clear() {
	txSetFillColor(TX_WHITE);
	txRectangle(50, 50, 780, 580);
}

void pencil_mode() {
	manager.buttons_[0]->mode_ = 1;
}

void spray_mode() {
	manager.buttons_[0]->mode_ = 2;
}

void fill_mode() {
	manager.buttons_[0]->mode_ = 3;
}

int main() {
	srand(time(0));

	int width = 800, height = 600;
	txCreateWindow(width, height);
	txSetColor(TX_BLACK);

	manager.add(new Canvas("", RECT{ 50, 50, 780, 580 }, NULL));
	manager.add(new RectButton("clear", RECT{ 10, 10, 50, 40 }, clear));
	manager.add(new RectButton("pen", RECT{ 10, 60, 30, 80 }, pencil_mode));
	manager.add(new RectButton("spray", RECT{ 10, 90, 30, 110 }, spray_mode));
	manager.add(new RectButton("fill", RECT{ 10, 120, 30, 140 }, fill_mode));

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
