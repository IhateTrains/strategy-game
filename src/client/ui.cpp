#include <cstdlib>
#include <cstring>
#include <string>

#ifdef _MSC_VER
#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif
#endif

#include <GL/glew.h>
#include <GL/gl.h>
#include <math.h>

#include <algorithm>

#include "../path.hpp"
#include "../print.hpp"
#include "ui.hpp"

#define NOMINMAX

using namespace UI;

static Context* g_ui_context = nullptr;
SDL_Color text_color = {0, 0, 0, 0};

Context::Context() {
    if (g_ui_context != nullptr) {
        throw std::runtime_error("UI context already constructed");
    }
    default_font = TTF_OpenFont(Path::get("ui/fonts/FreeMono.ttf").c_str(), 16);
    if (default_font == nullptr) {
        throw std::runtime_error("Font could not be loaded, exiting");
    }

    widgets.reserve(255);

    background = &g_texture_manager->load_texture(Path::get("ui/background.png"));
    window_top = &g_texture_manager->load_texture(Path::get("ui/window_top.png"));
    button = &g_texture_manager->load_texture(Path::get("ui/button.png"));
    tooltip = &g_texture_manager->load_texture(Path::get("ui/tooltip.png"));
    piechart_overlay = &g_texture_manager->load_texture(Path::get("ui/piechart.png"));

    g_ui_context = this;
    is_drag = false;
}

void Context::add_widget(Widget* widget) {
    widget->is_show = 1;

    // Not already here
    if (std::count(widgets.begin(), widgets.end(), widget))
        return;

    widgets.push_back(widget);
}

void Context::remove_widget(Widget* widget) {
    for (size_t i = 0; i < widgets.size(); i++) {
        if (widgets[i] != widget)
            continue;

        widgets.erase(widgets.begin() + i);
        break;
    }
}

void Context::clear(void) {
    // Remove all widgets
    for (auto& widget : widgets) {
        delete widget;
    }
    widgets.clear();
}

void Context::clear_dead() {
    // Quite stupid way to remove dead widgets
    // Does it safely though
    for (auto it = widgets.begin(); it != widgets.end();) {
        if ((*it)->dead) {
            delete (*it);
            clear_dead();
            return;
        } else {
            ++it;
        }
    }
}

// void Context::clear_dead_recursive(Widget* w) {
//     // Remove dead widgets
//     for (auto it = w->children.begin(); it != w->children.end();) {
//         if ((*it)->dead) {
//             delete (*it);
//             it = w->children.erase(it);
//         } else {
//             clear_dead_recursive(*it);
//             ++it;
//         }
//     }
// }

void Context::render_recursive(Widget& w, int x_off, int y_off) {
    if (!w.width || !w.height)
        return;

    x_off += w.x;
    y_off += w.y;

    glPushMatrix();
    glTranslatef(x_off, y_off, 0.f);
    w.on_render(*this);
    if (w.on_update) {
        w.on_update(w, w.user_data);
    }
    glPopMatrix();

    for (auto& child : w.children) {
        child->is_show = true;
        if ((child->x < 0 || child->x > w.width || child->y < 0 || child->y > w.height)) {
            child->is_show = false;
        }

        if (!child->is_show)
            continue;

        render_recursive(*child, x_off, y_off);
    }
}

void Context::render_all(const int width, const int height) {
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.f, (float)width, (float)height, 0.f, 0.0f, 1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.f, 0.f, 0.f);
    for (auto& widget : this->widgets) {
        render_recursive(*widget, 0, 0);
    }
    glPopMatrix();
}

int Context::check_hover_recursive(Widget& w, const unsigned int mx, const unsigned int my, int x_off, int y_off) {
    x_off += w.x;
    y_off += w.y;

    w.is_hover = false;

    if (!w.is_show)
        return 0;

    if (!((int)mx >= x_off && mx <= x_off + w.width && (int)my >= y_off && my <= y_off + w.height))
        return 0;

    w.is_hover = true;
    if (w.on_hover)
        w.on_hover(w, w.user_data);

    for (auto& child : w.children) {
        check_hover_recursive(*child, mx, my, x_off, y_off);
    }
    return 1;
}

void Context::check_hover(const unsigned mx, const unsigned my) {
    if (is_drag) {
        std::pair<int, int> offset = std::make_pair(mx - this->drag_x, my - this->drag_y);
        std::pair<int, int> diff = std::make_pair(offset.first - dragged_widget->x, offset.second - dragged_widget->y);

        dragged_widget->move_by(diff.first, diff.second);
        return;
    }

    for (int i = widgets.size() - 1; i >= 0; i--) {
        check_hover_recursive(*widgets[i], mx, my, 0, 0);
    }
    return;
}

int Context::check_click_recursive(Widget& w, const unsigned int mx, const unsigned int my, int x_off, int y_off) {
    x_off += w.x;
    y_off += w.y;

    // Widget must be displayed
    if (!w.is_show)
        return 0;

    // Click must be within the widget's box
    if (!((int)mx >= x_off && mx <= x_off + w.width && (int)my >= y_off && my <= y_off + w.height))
        return 0;

    switch (w.type) {
        case UI_WIDGET_SLIDER: {
            Slider* wc = (Slider*)&w;
            wc->value = ((float)std::abs((int)mx - x_off) / (float)wc->width) * wc->max;
        } break;
        default:
            break;
    }

    if (w.on_click)
        w.on_click(w, w.user_data);

    for (auto& child : w.children)
        check_click_recursive(*child, mx, my, x_off, y_off);
    return 1;
}

int Context::check_click(const unsigned mx, const unsigned my) {
    is_drag = false;
    for (int i = widgets.size() - 1; i >= 0; i--) {
        int r = check_click_recursive(*widgets[i], mx, my, 0, 0);
        if (r > 0) {
            return 1;
        }
    }
    return 0;
}

void Context::check_drag(const unsigned mx, const unsigned my) {
    for (int i = widgets.size() - 1; i >= 0; i--) {
        Widget& widget = *widgets[i];
        if (widget.type != UI_WIDGET_WINDOW)
            continue;

        if ((int)mx >= widget.x && mx <= widget.x + widget.width && (int)my >= widget.y && my <= widget.y + 24) {
            Window& c_widget = dynamic_cast<Window&>(widget);
            if (!c_widget.is_movable)
                continue;

            if (!is_drag) {
                drag_x = mx - widget.x;
                drag_y = my - widget.y;
                is_drag = true;
                dragged_widget = &widget;
                break;
            }
        }
    }
}

void check_text_input_recursive(Widget& widget, const char* _input) {
    if (widget.type == UI_WIDGET_INPUT && widget.is_hover) {
        Input& c_widget = dynamic_cast<Input&>(widget);
        c_widget.on_textinput(c_widget, _input, c_widget.user_data);
    }

    for (const auto& children : widget.children) {
        check_text_input_recursive(*children, _input);
    }
}

void Context::check_text_input(const char* _input) {
    for (const auto& widget : widgets) {
        check_text_input_recursive(*widget, _input);
    }
}

int Context::check_wheel(unsigned mx, unsigned my, int y) {
    for (const auto& widget : widgets) {
        if ((int)mx >= widget->x && mx <= widget->x + widget->width && (int)my >= widget->y && my <= widget->y + widget->height && widget->type == UI_WIDGET_WINDOW) {
            for (auto& child : widget->children) {
                if (!child->is_pinned)
                    child->y += y;
            }
            return 1;
        }
    }
    return 0;
}

void Widget::draw_rectangle(int _x, int _y, unsigned _w, unsigned _h, const GLuint tex) {
    // Texture switching in OpenGL is expensive
    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.f, 0.f);
    glVertex2f(_x, _y);
    glTexCoord2f(1.f, 0.f);
    glVertex2f(_x + _w, _y);
    glTexCoord2f(1.f, 1.f);
    glVertex2f(_x + _w, _y + _h);
    glTexCoord2f(1.f, 1.f);
    glVertex2f(_x + _w, _y + _h);
    glTexCoord2f(0.f, 1.f);
    glVertex2f(_x, _y + _h);
    glTexCoord2f(0.f, 0.f);
    glVertex2f(_x, _y);
    glEnd();
}

#include <deque>
void Widget::on_render(Context& ctx) {
    // Shadows
    if (type == UI_WIDGET_WINDOW) {
        // Shadow
        glBindTexture(GL_TEXTURE_2D, 0);
        glColor4f(0.f, 0.f, 0.f, 0.75f);
        glBegin(GL_TRIANGLES);
        glVertex2f(16, 16);
        glVertex2f(width + 16, 16);
        glVertex2f(width + 16, height + 16);
        glVertex2f(width + 16, height + 16);
        glVertex2f(16, height + 16);
        glVertex2f(16, 16);
        glEnd();
    }

    glColor3f(1.f, 1.f, 1.f);
    if (on_click && is_hover) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }

    // Background (tile) display
    if (type == UI_WIDGET_INPUT) {
        glBindTexture(GL_TEXTURE_2D, 0);

        glColor3f(0.f, 0.f, 1.f);
        glBegin(GL_TRIANGLES);
        glVertex2f(0.f, 0.f);
        glVertex2f(width, 0.f);
        glVertex2f(width, height);
        glVertex2f(width, height);
        glVertex2f(0.f, height);
        glVertex2f(0.f, 0.f);
        glEnd();
    } else if (type != UI_WIDGET_IMAGE && type != UI_WIDGET_LABEL) {
        glBindTexture(GL_TEXTURE_2D, ctx.background->gl_tex_num);
        glBegin(GL_TRIANGLES);
        glTexCoord2f(0.f, 0.f);
        glVertex2f(0.f, 0.f);
        glTexCoord2f(width / ctx.background->width, 0.f);
        glVertex2f(width, 0.f);
        glTexCoord2f(width / ctx.background->width, height / ctx.background->height);
        glVertex2f(width, height);
        glTexCoord2f(width / ctx.background->width, height / ctx.background->height);
        glVertex2f(width, height);
        glTexCoord2f(0.f, height / ctx.background->height);
        glVertex2f(0.f, height);
        glTexCoord2f(0.f, 0.f);
        glVertex2f(0.f, 0.f);
        glEnd();
    }

    glColor3f(1.f, 1.f, 1.f);
    // Top bar of windows display
    if (type == UI_WIDGET_WINDOW) {
        draw_rectangle(
            0, 0,
            width, 24,
            ctx.window_top->gl_tex_num);
    }

    if (current_texture != nullptr) {
        draw_rectangle(
            0, 0,
            width, height,
            current_texture->gl_tex_num);
    }

    if (type == UI_WIDGET_BUTTON) {
        const size_t padding = 4;
        // Put a "grey" inner background
        glBindTexture(GL_TEXTURE_2D, ctx.button->gl_tex_num);
        glBegin(GL_TRIANGLES);
        glTexCoord2f(0.f, 0.f);
        glVertex2f(padding, padding);
        glTexCoord2f(width / ctx.button->width, 0.f);
        glVertex2f(width - padding, padding);
        glTexCoord2f(width / ctx.button->width, height / ctx.button->height);
        glVertex2f(width - padding, height - padding);
        glTexCoord2f(width / ctx.button->width, height / ctx.button->height);
        glVertex2f(width - padding, height - padding);
        glTexCoord2f(0.f, height / ctx.button->height);
        glVertex2f(padding, height - padding);
        glTexCoord2f(0.f, 0.f);
        glVertex2f(padding, padding);
        glEnd();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glLineWidth(3.f);

    if (type == UI_WIDGET_INPUT) {
        glColor3f(1.f, 1.f, 1.f);
    } else {
        glColor3f(0.f, 0.f, 0.f);
    }
    if (1) {
        // Outer black border
        glBegin(GL_LINE_STRIP);
        glVertex2f(0, 0);
        glVertex2f(width, 0);
        glVertex2f(width, height);
        glVertex2f(0, height);
        glVertex2f(0, 0);
        glEnd();
    }

    if (text_texture != nullptr) {
        glColor3f(0.f, 0.f, 0.f);
        draw_rectangle(
            4, 4,
            text_texture->width, text_texture->height,
            text_texture->gl_tex_num);
    }
}

void input_ontextinput(Input& w, const char* input, void* data) {
    if (input != nullptr) {
        w.buffer += input;
    }

    if (w.buffer.length() > 0) {
        if (input == nullptr) {
            w.buffer.resize(w.buffer.length() - 1);
        }

        if (w.buffer.length() == 0) {
            w.text(" ");
        } else {
            w.text(w.buffer);
        }
    }
}

Widget::Widget(Widget* _parent, int _x, int _y, const unsigned w, const unsigned h, int _type, const UnifiedRender::Texture* tex)
    : is_show(1), type(_type), x(_x), y(_y), width(w), height(h), parent(_parent), current_texture(tex) {
    if (parent != nullptr) {
        parent->add_child(this);
    } else {
        // Add the widget to the context in each construction without parent
        g_ui_context->add_widget(this);
    }
}

Widget::~Widget() {
    // Delete the children recursively
    for (auto& child : children) {
        delete child;
    }
    children.clear();

    if (parent == nullptr) {
        // Hide widget immediately upon destruction
        g_ui_context->remove_widget(this);
    }
}

void Widget::move_by(int _x, int _y) {
    x += _x;
    y += _y;
}

void Widget::add_child(Widget* child) {
    // Not already in list
    if (std::count(children.begin(), children.end(), child))
        return;

    // Add to list
    children.push_back(child);
    child->parent = this;
}

void Widget::text(const std::string& _text) {
    SDL_Surface* surface;

    if (text_texture != nullptr) {
        glDeleteTextures(1, &text_texture->gl_tex_num);
        delete text_texture;
    }

    TTF_SetFontStyle(g_ui_context->default_font, TTF_STYLE_BOLD);

    surface = TTF_RenderUTF8_Solid(g_ui_context->default_font, _text.c_str(), text_color);
    if (surface == nullptr) {
        print_error("Cannot create text surface: %s", TTF_GetError());
        return;
    }

    text_texture = new UnifiedRender::Texture(surface->w, surface->h);
    text_texture->gl_tex_num = 0;

    for (size_t i = 0; i < (size_t)surface->w; i++) {
        for (size_t j = 0; j < (size_t)surface->h; j++) {
            uint8_t r, g, b, a;
            uint32_t pixel = ((uint8_t*)surface->pixels)[i + j * surface->pitch];
            SDL_GetRGBA(pixel, surface->format, &a, &b, &g, &r);

            uint32_t final_pixel;
            if (a == 0xff) {
                final_pixel = 0;
            } else {
                final_pixel = 0xffffffff;
            }
            text_texture->buffer[i + j * text_texture->width] = final_pixel;
        }
    }
    SDL_FreeSurface(surface);
    text_texture->to_opengl();
}

/**
* Constructor implementations for the different types of widgets
 */
Window::Window(int _x, int _y, unsigned w, unsigned h, Widget* _parent)
    : Widget(_parent, _x, _y, w, h, UI_WIDGET_WINDOW), is_movable(true) {
}

Checkbox::Checkbox(int _x, int _y, unsigned w, unsigned h, Widget* _parent)
    : Widget(_parent, _x, _y, w, h, UI_WIDGET_CHECKBOX) {
}

Button::Button(int _x, int _y, unsigned w, unsigned h, Widget* _parent)
    : Widget(_parent, _x, _y, w, h, UI_WIDGET_BUTTON) {
}

CloseButton::CloseButton(int _x, int _y, unsigned w, unsigned h, Widget* _parent)
    : Widget(_parent, _x, _y, w, h, UI_WIDGET_BUTTON) {
    on_click = &CloseButton::on_click_default;
}

Input::Input(int _x, int _y, unsigned w, unsigned h, Widget* _parent)
    : Widget(_parent, _x, _y, w, h, UI_WIDGET_INPUT) {
    this->on_textinput = input_ontextinput;
}

Image::Image(int _x, int _y, unsigned w, unsigned h, const UnifiedRender::Texture* tex, Widget* _parent)
    : Widget(_parent, _x, _y, w, h, UI_WIDGET_IMAGE) {
    current_texture = tex;
}

Label::Label(int _x, int _y, const char* _text, Widget* _parent)
    : Widget(_parent, _x, _y, 0, 0, UI_WIDGET_LABEL) {
    text(_text);
    width = text_texture->width;
    height = text_texture->height;
}

void Label::on_render(Context& ctx) {
    if (text_texture != nullptr) {
        if (!text_texture->gl_tex_num) {
            text_texture->to_opengl();
        }
    }
    if (text_texture != nullptr) {
        glColor3f(0.f, 0.f, 0.f);
        draw_rectangle(
            4, 2,
            text_texture->width, text_texture->height,
            text_texture->gl_tex_num);
    }
}

Chart::Chart(int _x, int _y, unsigned w, unsigned h, Widget* _parent)
    : Widget(_parent, _x, _y, w, h, UI_WIDGET_LABEL) {
}

void Chart::on_render(Context& ctx) {
    glColor3f(1.f, 1.f, 1.f);
    if (text_texture != nullptr) {
        if (!text_texture->gl_tex_num) {
            text_texture->to_opengl();
        }
    }

    if (current_texture != nullptr && current_texture->gl_tex_num) {
        draw_rectangle(
            0, 0,
            width, height,
            current_texture->gl_tex_num);
    }

    if (data.size() > 1) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glLineWidth(2.f);

        // Obtain the highest and lowest values
        double max = *std::max_element(data.begin(), data.end());
        double min = *std::min_element(data.begin(), data.end());

        // Works on zero-only graphs
        if (max == min) {
            max += 1.f;
        }

        // Do not allow graph lines to be on the "border" black thing
        size_t real_height = height - 3;

        size_t time = 0;
        glBegin(GL_LINE_STRIP);
        glColor3f(0.f, 0.f, 0.f);
        time = 0;
        for (const auto& node : data) {
            glVertex2f(
                time * (width / (data.size() - 1)),
                (real_height - (((node - min) / (max - min)) * real_height)) + 2.f);
            time++;
        }
        glEnd();

        glBegin(GL_LINE_STRIP);
        glColor3f(1.f, 0.f, 0.f);
        time = 0;
        for (const auto& node : data) {
            glVertex2f(
                time * (width / (data.size() - 1)),
                real_height - (((node - min) / (max - min)) * real_height));
            time++;
        }
        glEnd();
    }

    if (text_texture != nullptr) {
        glColor3f(0.f, 0.f, 0.f);
        draw_rectangle(
            4, 2,
            text_texture->width, text_texture->height,
            text_texture->gl_tex_num);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glLineWidth(3.f);
    glColor3f(0.f, 0.f, 0.f);

    // Inner black border
    glBegin(GL_LINE_STRIP);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glVertex2f(0, 0);
    glEnd();
}

Slider::Slider(int _x, int _y, unsigned w, unsigned h, const float _min, const float _max, Widget* _parent)
    : max{_max}, min{_min}, Widget(_parent, _x, _y, w, h, UI_WIDGET_SLIDER) {
}

void Slider::on_render(Context& ctx) {
    glColor3f(1.f, 1.f, 1.f);
    if (text_texture != nullptr) {
        if (!text_texture->gl_tex_num) {
            text_texture->to_opengl();
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    const float end_x = (value / max) * width;
    glBegin(GL_POLYGON);
    glColor3f(0.f, 0.f, 0.7f);
    glVertex2f(0.f, 0.f);
    glVertex2f(end_x, 0.f);
    glColor3f(0.f, 0.f, 0.4f);
    glVertex2f(end_x, height);
    glVertex2f(0.f, height);
    glColor3f(0.f, 0.f, 0.7f);
    glVertex2f(0.f, 0.f);
    glEnd();

    if (text_texture != nullptr) {
        glColor3f(0.f, 0.f, 0.f);
        draw_rectangle(
            4, 2,
            text_texture->width, text_texture->height,
            text_texture->gl_tex_num);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glLineWidth(3.f);
    glColor3f(0.f, 0.f, 0.f);

    // Inner black border
    glBegin(GL_LINE_STRIP);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glVertex2f(0, 0);
    glEnd();
}

PieChart::PieChart(int _x, int _y, unsigned w, unsigned h, std::vector<ChartData> _data, Widget* _parent)
    : data{_data}, Widget(_parent, _x, _y, w, h, UI_WIDGET_PIE_CHART) {
    max = 0;
    for (auto& slice : data) {
        max += slice.num;
    }
}

void PieChart::set_data(std::vector<ChartData> new_data) {
    data = new_data;
    for (auto& slice : data) {
        max += slice.num;
    }
}

void PieChart::draw_triangle(float start_ratio, float end_ratio, Color color) {
    float x_center = x + width / 2.f;
    float y_center = y + height / 2.f;
    float radius = std::min(width, height) * 0.5;
    float x_offset, y_offset, scale;

    glColor3f(color.r, color.g, color.b);
    x_offset = cos((start_ratio - 0.25f) * 2 * M_PI);
    y_offset = sin((start_ratio - 0.25f) * 2 * M_PI);
    scale = std::min(1.f / abs(x_offset), 1.f / abs(y_offset));
    x_offset *= scale;
    y_offset *= scale;
    glTexCoord2f(0.5f + x_offset * 0.5f, 0.5f + y_offset * 0.5f);
    glVertex2f(x_center + x_offset * radius, y_center + y_offset * radius);

    glTexCoord2f(0.5f, 0.5f);
    glVertex2f(x_center, y_center);

    x_offset = cos((end_ratio - 0.25f) * 2 * M_PI);
    y_offset = sin((end_ratio - 0.25f) * 2 * M_PI);
    scale = std::min(1.f / abs(x_offset), 1.f / abs(y_offset));
    x_offset *= scale;
    y_offset *= scale;
    glTexCoord2f(0.5f + x_offset * 0.5f, 0.5f + y_offset * 0.5f);
    glVertex2f(x_center + x_offset * radius, y_center + y_offset * radius);
}

void PieChart::on_render(Context& ctx) {
    glBindTexture(GL_TEXTURE_2D, ctx.piechart_overlay->gl_tex_num);
    glBegin(GL_TRIANGLES);
    float counter = 0;
    float last_corner = -0.125f;
    float last_ratio = 0;

    for (auto& slice : data) {
        counter += slice.num;
        float ratio = counter / max;
        while (ratio > last_corner + 0.25f) {
            last_corner += 0.25f;
            draw_triangle(last_ratio, last_corner, slice.color);
            last_ratio = last_corner;
        }
        draw_triangle(last_ratio, ratio, slice.color);
        last_ratio = ratio;
    }
    glEnd();
}
