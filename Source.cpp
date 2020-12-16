#include "C:\Users\Nikita\source\repos\TX\TXLib.h"

#include <iostream> 
#include <algorithm>
#include <ctime>
#include <fstream>
#include <string.h>
#include <queue>

#pragma comment(linker, "/STACK:257772160")

const int win_width = 1280, win_height = 720;

RGBQUAD* Video_memory = txVideoMemory();

typedef void (*func_t) (void);


//=======================================================//
typedef struct RgbColor
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
} RgbColor;

typedef struct HsvColor
{
	unsigned char h;
	unsigned char s;
	unsigned char v;
} HsvColor;

RgbColor HsvToRgb(HsvColor hsv)
{
	RgbColor rgb;
	unsigned char region, remainder, p, q, t;

	if (hsv.s == 0)
	{
		rgb.r = hsv.v;
		rgb.g = hsv.v;
		rgb.b = hsv.v;
		return rgb;
	}

	region = hsv.h / 43;
	remainder = (hsv.h - (region * 43)) * 6;

	p = (hsv.v * (255 - hsv.s)) >> 8;
	q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
	t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

	switch (region)
	{
	case 0:
		rgb.r = hsv.v; rgb.g = t; rgb.b = p;
		break;
	case 1:
		rgb.r = q; rgb.g = hsv.v; rgb.b = p;
		break;
	case 2:
		rgb.r = p; rgb.g = hsv.v; rgb.b = t;
		break;
	case 3:
		rgb.r = p; rgb.g = q; rgb.b = hsv.v;
		break;
	case 4:
		rgb.r = t; rgb.g = p; rgb.b = hsv.v;
		break;
	default:
		rgb.r = hsv.v; rgb.g = p; rgb.b = q;
		break;
	}

	return rgb;
}

HsvColor RgbToHsv(RgbColor rgb)
{
	HsvColor hsv;
	unsigned char rgbMin, rgbMax;

	rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
	rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

	hsv.v = rgbMax;
	if (hsv.v == 0)
	{
		hsv.h = 0;
		hsv.s = 0;
		return hsv;
	}

	hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
	if (hsv.s == 0)
	{
		hsv.h = 0;
		return hsv;
	}

	if (rgbMax == rgb.r)
		hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
	else if (rgbMax == rgb.g)
		hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
	else
		hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

	return hsv;
}
//=======================================================//

class Button;

void set_color(size_t size);
void set_pixel(RgbColor rgb, RECT rect, POINT pos);


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

Manager manager;

class RectButton : public Button {
public:
	RectButton(const char* name, RECT rect, func_t func) :
		Button(name, rect, func) {}

	virtual void draw_button() override {
		txSetColor(TX_BLACK);
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

class ColorButton : public RectButton {
public:
	COLORREF color_;

	ColorButton(RECT rect, COLORREF color) :
		color_(color),
		RectButton(NULL, rect, NULL)
	{};

	virtual void draw_button() override {
		txSetColor(TX_BLACK);
		txSetFillColor(TX_WHITE);
		txRectangle(rect_.left, rect_.top, rect_.right, rect_.bottom);

		txSetColor(color_);
		txSetFillColor(color_);
		txRectangle(rect_.left + 2, rect_.top + 2, rect_.right - 2, rect_.bottom - 2);
	}

	virtual void pressed() {
		COLORREF temp = ((ColorButton*)manager.buttons_[1])->color_;

		if (color_ == temp)
			return;

		((ColorButton*)manager.buttons_[1])->color_ = color_;
		color_ = temp;

		manager.buttons_[2]->draw_button();
		manager.buttons_[1]->draw_button();

		while (txMouseButtons() == 1)
			txSleep(100);
	}
};

class Canvas : public RectButton {
private:
	

public:
	size_t size_ = 50;
	//size_t mode_ = 0;

	Canvas(const char* name, RECT rect, func_t func) :
		RectButton(name, rect, func) {}

	virtual void pressed() override;

	void draw_circle(POINT pos);

	void pencil();
	void spray();
	void fill(COLORREF old_color, POINT pos, POINT prev);

};

void Canvas::pressed() {
	switch (mode_)
	{
	case 0:
		draw_circle(txMousePos());
		break;

	case 1:
		//std::cout << "pencil\n";
		pencil();
		break;

	case 2:
		//std::cout << "spray\n";
		spray();
		break;

	case 3:
		//std::cout << "fill\n";
		POINT pixel = txMousePos();
		
		COLORREF old_color = txGetPixel(pixel.x, pixel.y);

		if (old_color != ((ColorButton*)manager.buttons_[1])->color_)
			fill(old_color, pixel, POINT{ NULL, NULL });

		break;
	}
}


//template <size_t N>

void Manager::draw() {
	txSetFillColor(RGB(40, 40, 40));
	txClear();


	for (size_t button = 1; button <= count_; button++)
		buttons_[count_ - button]->draw_button();
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

					break;
				}

		if (txGetAsyncKeyState(VK_MENU)) {
			if (txMouseButtons() == 2) {
				int x0 = txMouseX(), y0 = txMouseY();
				size_t old_size = ((Canvas*)buttons_[0])->size_;

				HDC save = txCreateCompatibleDC(win_width, win_height);
				txBitBlt(save, 0, 0, win_width, win_height, txDC(), 0, 0);

				txSetColor(TX_RED);
				txSetFillColor(TX_RED);

				while (txGetAsyncKeyState(VK_MENU) && txMouseButtons() == 2) {
					txSleep();
					
					((Canvas*)buttons_[0])->size_ = MIN(MAX(old_size + txMouseX() - x0, 1), 999);

					txBitBlt(txDC(), 0, 0, win_width, win_height, save, 0, 0);

					txBegin();
					((Canvas*)buttons_[0])->draw_circle(POINT{ x0, y0 });
					txEnd();
				}

				txBitBlt(txDC(), 0, 0, win_width, win_height, save, 0, 0);

				txDeleteDC(save);
			}
		}

		txSleep();
	}
}

void Canvas::pencil() {
	double x0 = txMouseX(), y0 = txMouseY(), x1 = 0, y1 = 0;

	set_color(size_);

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
	double R = MAX(size_ * 1. / 20, 1);

	set_color(R);

	while (txMouseButtons() == 1) {
		int dist = 0 + rand() % (size_ - 0), angle = 0 + rand() % (360 - 0);

		double x = txMouseX() + dist * cos(angle), y = txMouseY() + dist * sin(angle);

		if (x - R > rect_.left && x + R < rect_.right && y - R > rect_.top && y + R < rect_.bottom)
			txCircle(x, y, R);

		Sleep(8);
	}
}

void Canvas::fill(COLORREF old_color, POINT pos, POINT prev) {

	if (old_color == txGetPixel(pos.x, pos.y) && 
		pos.x > rect_.left && 
		pos.x < rect_.right && 
		pos.y < rect_.bottom && 
		pos.y > rect_.top) {

		for (int i = -1; i <= 1; ++i)
			for (int j = -1; j <= 1; ++j) {
				//if (prev.x != pixel.x || prev.y != pixel.y)
				fill(old_color, POINT{ pos.x + i, pos.y + j }, pos);

				RgbColor color = { 
					txExtractColor(((ColorButton*)manager.buttons_[1])->color_, TX_RED),
					txExtractColor(((ColorButton*)manager.buttons_[1])->color_, TX_GREEN),
					txExtractColor(((ColorButton*)manager.buttons_[1])->color_, TX_BLUE) };

				//set_pixel(color, rect_, pos);
				txSetPixel(pos.x, pos.y, RGB((int)color.r, (int)color.g, (int)color.b));
			}
	}
	return;
}

class Palette : public Button {
public:
	COLORREF major_color = RGB(0, 0, 0), minor_color = RGB(255, 255, 255);
	int hue = 0, palette_mode = 0;

	Palette(const char* name, RECT rect, func_t func) :
		Button(name, rect, func) {}

	void delete_pointer() {
		txSetColor(RGB(40, 40, 40));
		txSetFillColor(RGB(40, 40, 40));

		int pos_pointer = rect_.left + hue;
		txRectangle(pos_pointer - 6, rect_.top - 19, pos_pointer + 6, rect_.top - 9);
		txRectangle(pos_pointer - 6, rect_.top - 60, pos_pointer + 6, rect_.top - 70);
	}

	virtual void draw_button() override {
		txSetColor(TX_BLACK);
		txSetFillColor(TX_WHITE);

		txRectangle(rect_.left - 1, rect_.top - 60, rect_.right + 1, rect_.top - 18);
		txRectangle(rect_.left - 1, rect_.top,  rect_.right + 1, rect_.bottom + 1);

		int pos_pointer = rect_.left + hue;
		POINT pointer1[5] = { {pos_pointer, rect_.top - 20}, {pos_pointer - 5, rect_.top - 15}, {pos_pointer - 5, rect_.top - 10}, {pos_pointer + 5, rect_.top - 10}, {pos_pointer + 5, rect_.top - 15} },
			  pointer2[5] = { {pos_pointer, rect_.top - 60}, {pos_pointer - 5, rect_.top - 65}, {pos_pointer - 5, rect_.top - 70}, {pos_pointer + 5, rect_.top - 70}, {pos_pointer + 5, rect_.top - 65} };
		txPolygon(pointer1, 5);
		txPolygon(pointer2, 5);

		txBegin();

		for (int hue = 255; hue >= 0; hue--) {
			txSetColor(txHSL2RGB(RGB(hue, 100, 54)));
			txLine(rect_.left + hue, rect_.top - 20, rect_.left + hue, rect_.top - 60);
		}

		RgbColor rgb = { 0, 0, 0 };

		for (int brightness = 0; brightness < 256; brightness++) {
			for (int saturation = 0; saturation < 256; saturation++) {
				rgb = HsvToRgb(HsvColor{ (unsigned char)hue, (unsigned char)saturation, (unsigned char)brightness });

				set_pixel(rgb, rect_, POINT{ rect_.left + saturation, rect_.bottom - brightness });
			}
		}

		txEnd();

	}

	virtual bool is_mouse_on_button() override {
		double x = txMouseX(), y = txMouseY();

		if (x >= rect_.left && x <= rect_.right) {
			if (y >= rect_.top && y <= rect_.bottom) {
				palette_mode = 0;
				return TRUE;
			}

			if (y >= rect_.top - 60 && y <= rect_.top - 20) {
				palette_mode = 1;
				return TRUE;
			}
		}

		return FALSE;
	}

	virtual void pressed() override;
};

void Palette::pressed() {
	//std::cout << 1 << "\n";
	if (palette_mode == 1) {
		delete_pointer();

		hue = txMouseX() - (rect_.left);
		draw_button();
	}

	else {
		POINT point = txMousePos();
		COLORREF color = txGetPixel(point.x, point.y);

		((ColorButton*)manager.buttons_[1])->color_ = color;

		manager.buttons_[2]->draw_button();
		manager.buttons_[1]->draw_button();

		/*
		if (txExtractColor(txRGB2HSL(point_color), TX_LIGHTNESS) >= 54)
			txSetColor(TX_BLACK);

		else
			txSetColor(TX_WHITE);

		txSetFillColor(TX_NULL);
		txCircle(point.x, point.y, 5);
		*/
	}
}

void set_color(size_t size) {
	COLORREF color = ((ColorButton*)manager.buttons_[1])->color_;

	txSetColor(color, size);
	txSetFillColor(color);
}

void set_pixel(RgbColor rgb, RECT rect, POINT pos) {
	RGBQUAD* pixel = &Video_memory[(win_height - (int)(pos.y)) * win_width + (int)(pos.x)];

	pixel->rgbRed = (int)rgb.r;
	pixel->rgbGreen = (int)rgb.g;
	pixel->rgbBlue = (int)rgb.b;
}

void Canvas::draw_circle(POINT pos) {
	for (int x = MAX(pos.x - size_ / 2, rect_.left) + 1; x < MIN(pos.x + size_ / 2, rect_.right) - 1; x++) {
		for (int y = MAX(pos.y - size_ / 2, rect_.top) + 2; y < MIN(pos.y + size_ / 2, rect_.bottom); y++) {
			if (sqrt(pow(x - pos.x, 2) + pow(y - pos.y, 2)) <= size_ / 2)
				set_pixel(RgbColor{ 255, 0, 0 }, rect_, POINT{ x, y });
				//txSetPixel(x, y, TX_RED);	
		}
	}
}

void clear() {
	manager.buttons_[0]->draw_button(); 
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

	txCreateWindow(win_width, win_height);
	txSetColor(TX_BLACK);

	Video_memory = txVideoMemory();

	manager.add(new Canvas("", RECT{ 50, 50, win_width - 296, win_height - 20 }, NULL));
	manager.add(new ColorButton(RECT{ 7, win_height - 56, 32, win_height - 31 }, TX_BLACK));
	manager.add(new ColorButton(RECT{ 18, win_height - 45, 43, win_height - 20 }, TX_WHITE));

	manager.add(new RectButton("clear", RECT{ 10, 10, 50, 40 }, clear));
	manager.add(new RectButton("pen", RECT{ 10, 60, 30, 80 }, pencil_mode));
	manager.add(new RectButton("spray", RECT{ 10, 90, 30, 110 }, spray_mode));
	manager.add(new RectButton("fill", RECT{ 10, 120, 30, 140 }, fill_mode));
	manager.add(new Palette("", RECT{ win_width - 276, win_height - 276, win_width - 20, win_height - 20 }, NULL));

	manager.draw();
	
	manager.run();

	txDisableAutoPause();

}
