#pragma once

#ifndef __CPP_PLOT_PATH
#define __CPP_PLOT_PATH
#endif

#include <string>
#include <sstream>
//#include "canvas.hpp"

template <typename prec = float>
class path {

public:

    struct element {
        element* next = nullptr;
        prec* points = nullptr;
        unsigned char type = 0u;

        element() = default;

        element(element&& e) {
            next = e.next;
            points = e.points;
            type = e.type;
            e.next = nullptr;
            e.points = nullptr;
            e.type = 0u;
        }

        template <typename T>
        element(const typename path<T>::element& e) {
            type = e.type;
            switch (type) {
                case 0u:
                case 1u:
                    points = malloc(sizeof(prec) * 2);
                    points[0] = (prec)e.points[0];
                    points[1] = (prec)e.points[1];
                    break;
                case 2u:
                case 3u:
                    points = malloc(sizeof(prec));
                    points[0] = (prec)e.points[0];
                    break;
                case 254u: //text
                    size_t len = 1;
                    char* p_str = (char*)(e.points + 3);
                    while (*p_str++ != '\0') ++len;
                    points = malloc(sizeof(prec) * 3 + len);
                    points[0] = (prec)e.points[0];
                    points[1] = (prec)e.points[1];
                    points[2] = (prec)e.points[2];
                    --p_str;
                    char* str = (char*)(points + 3 + len);
                    while (len--) *str-- = *p_str--;
            }
        }

        element& operator=(const element& e) {
            type = e.type;
            next = e.next;
            switch (type) {
                case 0u:
                case 1u:
                    points = (prec*)malloc(sizeof(prec) * 2);
                    points[0] = (prec)e.points[0];
                    points[1] = (prec)e.points[1];
                    break;
                case 2u:
                case 3u:
                    points = (prec*)malloc(sizeof(prec));
                    points[0] = (prec)e.points[0];
                    break;
                case 254u: //text
                    size_t len = 1;
                    char* p_str = (char*)(e.points + 3);
                    while (*p_str++ != '\0') ++len;
                    points = (prec*)malloc(sizeof(prec) * 3 + len);
                    points[0] = (prec)e.points[0];
                    points[1] = (prec)e.points[1];
                    points[2] = (prec)e.points[2];
                    --p_str;
                    char* str = (char*)(points + 3 + len);
                    while (len--) *str-- = *p_str--;
            }
            return *this;
        }

        template <typename T>
        element& operator=(const typename path<T>::element& e) {
            type = e.type;
            next = e.next;
            switch (type) {
                case 0u:
                case 1u:
                    points = malloc(sizeof(prec) * 2);
                    points[0] = (prec)e.points[0];
                    points[1] = (prec)e.points[1];
                    break;
                case 2u:
                case 3u:
                    points = malloc(sizeof(prec));
                    points[0] = (prec)e.points[0];
                    break;
                case 254u: //text
                    size_t len = 1;
                    char* p_str = (char*)(e.points + 3);
                    while (*p_str++ != '\0') ++len;
                    points = malloc(sizeof(prec) * 3 + len);
                    points[0] = (prec)e.points[0];
                    points[1] = (prec)e.points[1];
                    points[2] = (prec)e.points[2];
                    --p_str;
                    char* str = (char*)(points + 3 + len);
                    while (len--) *str-- = *p_str--;
            }
        }

        element& operator=(element&& e) {
            next = e.next;
            points = e.points;
            type = e.type;
            e.next = nullptr;
            e.points = nullptr;
            e.type = 0u;
            return *this;
        }

        // (x - min_x) / (max_x - min_x) * scale_x + offset_x
        element __transform(const prec& plot_min_x, const prec& plot_min_y, const prec& plot_max_y, const prec& plot_mul_x, const prec& plot_mul_y, const prec& offset_x, const prec& offset_y) const {
            element e;
            e.type = type;
            e.next = next;
            switch (type) {
                case 0u:
                case 1u:
                    e.points = (prec*)malloc(sizeof(prec) * 2);
                    e.points[0] = (points[0] - plot_min_x) * plot_mul_x + offset_x;
                    e.points[1] = (plot_max_y - points[1] + plot_min_y) * plot_mul_y + offset_y;
                    break;
                case 2u: // horizontal
                    e.points = (prec*)malloc(sizeof(prec));
                    e.points[0] = (points[0] - plot_min_x) * plot_mul_x + offset_x;
                    break;
                case 3u: // vertical
                    e.points = (prec*)malloc(sizeof(prec));
                    e.points[0] = (plot_max_y - points[0] + plot_min_y) * plot_mul_y + offset_y;
                    break;
                case 254u: //text
                    size_t len = 1;
                    char* p_str = (char*)(points + 3);
                    while (*p_str++ != '\0') ++len;
                    e.points = (prec*)malloc(sizeof(prec) * 3 + len);
                    e.points[0] = (points[0] - plot_min_x) * plot_mul_x + offset_x;
                    e.points[1] = (plot_max_y - points[1] + plot_min_y) * plot_mul_y + offset_y;
                    e.points[2] = points[2] * (plot_mul_x > plot_mul_y ? plot_mul_x : plot_mul_y);
                    --p_str;
                    char* str = (char*)(e.points + 3 + len);
                    while (len--) *str-- = *p_str--;
            }
            return e;
        }

        std::string to_string(const bool& symbol = true) {
			std::ostringstream oss;
            switch (type) {
                case 0u: // move_to
                    if (symbol) oss << 'M';
                    //s += std::to_string(points[0]);
                    //s += ',';
                    //s += std::to_string(points[1]);
                    oss << std::noshowpoint << points[0] << ',' << std::noshowpoint << points[1];
                break;
                case 1u: // line_to
                    if (symbol) oss << 'L';
                    oss << std::noshowpoint << points[0] << ',' << std::noshowpoint << points[1];
                break;
                case 2u: // horizontal_line_to
                    if (symbol) oss << 'H';
                    oss << std::noshowpoint << points[0];
                break;
                case 3u: // vertical_line_to
                    if (symbol) oss << 'V';
                    oss << std::noshowpoint << points[0];
                break;
            }
            return oss.str();
        }

        ~element() { 
            if (points != nullptr) {
                free(points); 
                points = nullptr;
            }
        }
    } * start = nullptr, * end = nullptr;

    prec min_x = 0, min_y = 0, max_x = 0, max_y = 0;

    bool close = false;

    double stroke_width = 0;
    unsigned int stroke_fill = 0, fill = 0;

public:
    enum class stroke_line_cap { BUTT, ROUND, SQUARE };
    enum class stroke_line_join { MITER, ROUND, BEVEL };

private:
    stroke_line_cap cap;
    stroke_line_join join;
    
public:

    path() = default;

    path(path&& p) {
        start = p.start;
        end = p.end;
        close = p.close;
        min_x = p.min_x;
        min_y = p.min_y;
        max_x = p.max_x;
        max_y = p.max_y;
        stroke_width = p.stroke_width;
        stroke_fill = p.stroke_fill;
        fill = p.fill;
        p.min_x = p.min_y = p.max_x = p.max_y = p.stroke_width = p.stroke_fill = p.fill = p.close = 0;
        p.start = p.end = nullptr;
    }

    path& operator=(path&& p) {
        start = p.start;
        end = p.end;
        close = p.close;
        min_x = p.min_x;
        min_y = p.min_y;
        max_x = p.max_x;
        max_y = p.max_y;
        stroke_width = p.stroke_width;
        stroke_fill = p.stroke_fill;
        fill = p.fill;
        p.min_x = p.min_y = p.max_x = p.max_y = p.stroke_width = p.stroke_fill = p.fill = p.close = 0;
        p.start = p.end = nullptr;
        return *this;
    }

    path& operator=(const path& p) {
        if (p.start != nullptr) {
            element* p_itr = p.start;
            element* itr = start = new element;
            *itr = *p_itr;
            while (p_itr->next != nullptr) {
                itr->next = new element;
                itr = itr->next;
                p_itr = p_itr->next;
                *itr = *p_itr;
            }
            end = itr;
            itr->next = nullptr;
        }
        min_x = p.min_x;
        min_y = p.min_y;
        max_x = p.max_x;
        max_y = p.max_y;
        stroke_width = p.stroke_width;
        stroke_fill = p.stroke_fill;
        fill = p.fill;
        return *this;
    }

    /*template <typename T>
    path& operator=(const path<T>& p) {
        if (p.start != nullptr) {
            typename path<T>::element* p_itr = p.start;
            element* itr = start = new element;
            *itr = *p_itr;
            while (p_itr->next != nullptr) {
                itr->next = new element;
                itr = itr->next;
                p_itr = p_itr->next;
                *itr = *p_itr;
            }
            end = *itr;
        }
        min_x = (prec)p.min_x;
        min_y = (prec)p.min_y;
        max_x = (prec)p.max_x;
        max_y = (prec)p.max_y;
        stroke_width = p.stroke_width;
        stroke_fill = p.stroke_fill;
        fill = p.fill;
    }*/
    
    static const char* to_hex(unsigned int color, char* const& buffer) {
        char* itr = buffer + 5;
        while (itr >= buffer) {
            unsigned char mod = color % 16;
            *itr-- = (mod > 9) ? ('A' + mod - 10) : ('0' + mod);
            color /= 16;
        }
        return buffer;
    }

    void set_stroke_width(const double& width) { stroke_width = width; }

    void set_stroke_fill(const int& rgba) { stroke_fill = rgba; }

    void set_fill(const int& rgba) { fill = rgba; }

    void set_stroke_line_cap(const stroke_line_cap& cap) { this->cap = cap; }
    void set_stroke_line_join(const stroke_line_join& join) { this->join = join; }

    void move_to(const prec& x, const prec& y) {
        element* const next = new element;
        next->points = (prec*)malloc(sizeof(prec) << 1); //new prec[2]{ x, y };
        next->points[0] = x;
        next->points[1] = y;
        if (x < min_x) min_x = x;
        else if (x > max_x) max_x = x;
        if (y < min_y) min_y = y;
        else if (y > max_y) max_y = y;
        if (start == nullptr) start = end = next;
        else end = end->next = next;
    }

    void close_path() { close = true; }

    void line_to(const prec& x, const prec& y) {
        element* const next = new element;
        next->points = (prec*)malloc(sizeof(prec) << 1); //new prec[2]{ x, y };
        next->points[0] = x;
        next->points[1] = y;
        next->type = 1u;
        if (x < min_x) min_x = x;
        else if (x > max_x) max_x = x;
        if (y < min_y) min_y = y;
        else if (y > max_y) max_y = y;
        if (start == nullptr) start = end = next;
        else end = end->next = next;
    }

    void horizontal_line_to(const prec& x) {
        element* const next = new element;
        next->points = (prec*)malloc(sizeof(prec)); //new prec[1]{ y };
        next->points[0] = x;
        next->type = 2u;
        if (x < min_x) min_x = x;
        else if (x > max_x) max_x = x;
        if (start == nullptr) start = end = next;
        else end = end->next = next;
    }

    void vertical_line_to(const prec& y) {
        element* const next = new element;
        next->points = (prec*)malloc(sizeof(prec)); //new prec[1]{ y };
        next->points[0] = y;
        next->type = 3u;
        if (y < min_y) min_y = y;
        else if (y > max_y) max_y = y;
        if (start == nullptr) start = end = next;
        else end = end->next = next;
    }

    void text(const prec& x, const prec& y, const char* str, const prec& font_size) {
        size_t len = 1;
        for (const char* ptr = str; *ptr != '\0'; ++ptr) ++len;
        element* const next = new element;
        next->points = (prec*)malloc(sizeof(prec) * 3 + len);
        next->points[0] = x;
        next->points[1] = y;
        next->points[2] = font_size;
        char* ptr = (char*)(next->points + 3);
        while (len--) *ptr++ = *str++;
        next->type = 254u;
        if (x < min_x) min_x = x;
        else if (x > max_x) max_x = x;
        if (y < min_y) min_y = y;
        else if (y > max_y) max_y = y;
        if (start == nullptr) start = end = next;
        else end = end->next = next;
    }

    // (x - min_x) / (max_x - min_x) * scale_x + offset_x
    path __transform(const prec& plot_min_x, const prec& plot_min_y, const prec& plot_max_y, const prec& plot_mul_x, const prec& plot_mul_y, const prec& offset_x, const prec& offset_y) const { // for plor use only
        path p;
        if (start != nullptr) {
            element* this_itr = start;
            element* itr = p.start = new element;
            *itr = std::move(this_itr->__transform(plot_min_x, plot_min_y, plot_max_y, plot_mul_x, plot_mul_y, offset_x, offset_y));
            while (this_itr->next != nullptr) {
                itr->next = new element;
                itr = itr->next;
                this_itr = this_itr->next;
                *itr = std::move(this_itr->__transform(plot_min_x, plot_min_y, plot_max_y, plot_mul_x, plot_mul_y, offset_x, offset_y));
            }
            p.end = itr;
            itr->next = nullptr;
        }
        p.min_x = offset_x;
        p.min_y = offset_y;
        p.max_x = (max_x - plot_min_x) * plot_mul_x + offset_x;
        p.max_y = (plot_max_y - max_y + plot_min_y) * plot_mul_y + offset_y;
        p.stroke_width = stroke_width;
        p.stroke_fill = stroke_fill;
        p.fill = fill;
        return p;
    }

    // This should just be part of the canvas interface
    /*unsigned char* render(const unsigned int& width, const unsigned int& height) {
        unsigned char* const buffer = (unsigned char*)calloc(width * height, 1);
        canvas c { buffer, width, height, IMAGE_GRAYSCALE };

        unsigned int x = 0, y = 0, origin_x = 0, origin_y = 0;
        for (element* ptr = start; ptr != nullptr; ptr = ptr->next) {
            switch (ptr->type) {
                case 0u: // move_to
                    x = (ptr->points[0] - min_x) / (max_x - min_x) * (width - 1);
                    y = (ptr->points[1] - min_y) / (max_y - min_y) * (height - 1);
                    if (ptr == start) {
                        origin_x = x;
                        origin_y = y;
                    }
                    fputs("move_to ", stdout);
                break;
                case 1u: // line_to
                    const unsigned int x_i = (ptr->points[0] - min_x) / (max_x - min_x) * (width - 1);
                    const unsigned int y_i = (ptr->points[1] - min_y) / (max_y - min_y) * (height - 1);
                    c.draw_line_2(x, y, x_i, y_i, 0xFFu);
                    x = x_i;
                    y = y_i;
                    fputs("line_to ", stdout);
                break;
                //case 2u: // horizontal_line_to
                //break;
                //case 3u: // vertical_line_to
                //break;
            }
            printf("%u %u\n", x, y);
        }
        if (close && (origin_x != x || origin_y != y)) c.draw_line_2(x, y, origin_x, origin_y, 0xFFu);

        for (unsigned int i = 1; i < height - 1; ++i) {
            bool inside = buffer[i * width];
            bool empty = !inside;
            for (unsigned int j = 1; j < width - 1; ++j) {
                empty = !buffer[i * width + j];
                if (buffer[i * width + j] && empty) {
                    inside = !inside;
                }
                if (inside && buffer[(i - 1) * width + j]/* && buffer[i * width + j - 1]) {
                    buffer[i * width + j] = 0xFF;
                }
            }
        }

        return buffer;
    }*/

    void clear() {
        while (start != end) {
            element* temp = start;
            start = start->next;
            //printf("x: %s\n", temp->to_string().c_str());
            delete temp;
        }
        if (start != nullptr) {
            //printf("x: %s\n", start->to_string().c_str());
            delete start;
            start = end = nullptr;
        }
    }

    bool empty() const {
        return start == nullptr;
    }

    ~path() { clear(); }
};

typedef path<> path_f;
typedef path<double> path_d;

namespace svg {

    //path load(const char* const& path) {
    //    return {};
    //}

    template <typename prec>
    void save(const path<prec>& shape, const char* const& p) {
        std::string d = "d = \"";
        typename path<prec>::element* itr = shape.start;
        while (true) {
            d += itr->to_string();
            itr = itr->next;
            if (itr != nullptr) d += ' ';
            else break;
        }
        if (shape.close) d += " Z\"";
        else d += '"';
        printf("%s\n", d.c_str());
    }

    template <typename prec>
    void save(const path<prec>* const& shapes, const size_t& size, const char* const& p) {
        bool font = false;
        unsigned char type = 255u;

        prec min_x = shapes->min_x, min_y = shapes->min_y, max_x = shapes->max_x, max_y = shapes->max_y;
        for (unsigned int i = 1; i < size; ++i) {
            if (shapes[i].min_x < min_x) min_x = shapes[i].min_x;
            else if (shapes[i].max_x > max_x) max_x = shapes[i].max_x;
            if (shapes[i].min_y < min_y) min_y = shapes[i].min_y;
            else if (shapes[i].max_y > max_y) max_y = shapes[i].max_y;
        }

        const prec width = max_x - min_x, height = max_y - min_y;

        min_x -= width * .1;
        max_x += width * .1;
        min_y -= height * .1;
        max_y += height * .1;

        FILE* const file = fopen(p, "w");
        //fputs("<svg xmlns:xlink = \"http://www.w3.org/1999/xlink\" viewBox = \"-0.2 -0.2 1.4 1.4\" xmlns = \"http://www.w3.org/2000/svg\" version = \"1.1\">\n", file);
        //fprintf(file, "<svg xmlns:xlink = \"http://www.w3.org/1999/xlink\" viewBox = \"%lf %lf %lf %lf\" xmlns = \"http://www.w3.org/2000/svg\" version = \"1.1\">\n", -.2 * 960, -.2 * 960, 1.4 * 960, 1.4 * 960);
        //fprintf(file, "<svg xmlns:xlink = \"http://www.w3.org/1999/xlink\" viewBox = \"%lf %lf %lf %lf\" xmlns = \"http://www.w3.org/2000/svg\" version = \"1.1\">\n", -.2 * 1080, -.2 * 960, 1.4 * 1080, 1.4 * 960);
        fprintf(file, "<svg xmlns:xlink = \"http://www.w3.org/1999/xlink\" viewBox = \"%lf %lf %lf %lf\" xmlns = \"http://www.w3.org/2000/svg\" version = \"1.1\">\n", min_x, min_y, max_x - min_x, max_y - min_y);
        for (unsigned int i = 0; i < size; ++i) {
            std::string d = "<path d = \"", text_buffer;
            char buffer[7]{};
            typename path<prec>::element* itr = shapes[i].start;
            if (itr == nullptr) continue;
            while (true) {
                if (itr->type == 254u) { // text
                    text_buffer += "<text x = \"";
                    text_buffer += std::to_string(itr->points[0]);
                    text_buffer += "\" y = \"";
                    text_buffer += std::to_string(itr->points[1]);
                    //text_buffer += "\" font-size = \".03\" text-anchor = \"middle\" dominant-baseline = \"middle\">";
                    text_buffer += "\" font-size = \"";
                    text_buffer += std::to_string(itr->points[2]);
                    text_buffer += "\" text-anchor = \"middle\" dominant-baseline = \"middle\">";
                    text_buffer += (const char*)(itr->points + 3);
                    text_buffer += "</text>\n";
                }
                else d += itr->to_string(type != itr->type);
                type = itr->type;
                itr = itr->next;
                if (itr != nullptr) d += ' ';
                else break;
            }
            if (shapes[i].close) d += " Z\"";
            else d += '"';
            if (shapes[i].fill) {
                d += " fill = \"#";
                d += path<prec>::to_hex(shapes[i].fill, buffer);
                d += '"';
            }
            else d += " fill = \"none\"";
            if (shapes[i].stroke_width > 0) {
                d += " stroke = \"#";
                d += path<prec>::to_hex(shapes[i].stroke_fill >> 8, buffer);
                if (~(shapes[i].stroke_fill & 0xFF)) {
                    d += "\" stroke-opacity = \"";
                    d += std::to_string((shapes[i].stroke_fill & 0xFF) / 255.);
                }
                d += "\" stroke-width = \"";
                d += std::to_string(shapes[i].stroke_width);
                d += '"';
            }
            d += " stroke-linejoin=\"round\"/>\n";
            fputs(d.c_str(), file);
            if (!text_buffer.empty()) {
                if (!font) {
                    fputs("<style>\n@import url('https://fonts.googleapis.com/css?family=JetBrains+Mono:400');\ntext {\nfont-family: \"JetBrains Mono\", monospace;\nfont-optical-sizing: auto;\nfont-style: normal;\n}\n</style>\n", file);
                    font = true;
                }
                fputs(text_buffer.c_str(), file);
            }
            //printf("%s\n", d.c_str());
        }
        fputs("</svg>", file);
        fclose(file);
    }

}