#pragma once

#include <string>
#include <math.h>
#include <sstream>

#ifndef __DISABLE_CANVAS
#include "canvas.hpp"

struct plot_theme {
	paint background, grid, foreground, marker_color;
	unsigned int v_padding = 40u, h_padding = 40u, axis_padding = 10u;
	unsigned char font_size = 2u;
	static const plot_theme light, dark;
};


const plot_theme plot_theme::light = { 0xFFFFFFFF, 0xFFD3D3D3, 0xFF000000, 0xFF2E7BFF };
const plot_theme plot_theme::dark = { 0xFF111217, 0xFF878787, 0xFFFFFFFF, 0xFF0077FF };

#endif

#include <typeinfo>
#include <iostream>

#ifdef __CPP_PLOT_PATH
class _plot {

	template <typename T>
	static constexpr bool is_number_pointer() {
		return std::is_pointer<T>::value && std::is_arithmetic<typename std::remove_cv<typename std::remove_pointer<T>::type>::type>::value;
	}

	template <typename T>
	static constexpr bool is_pointer_to_const() {
		return std::is_pointer<T>::value && std::is_const<typename std::remove_pointer<T>::type>::value;
	}

	template <typename T>
	using inner_t = typename std::remove_cv<typename std::remove_pointer<T>::type>::type;

	enum ValueType {
		Int8,
		UInt8,
		Int16,
		UInt16,
		Int32,
		UInt32,
		Int64,
		UInt64,
		Float,
		Double,
		Unknown
	};

	template <typename T>
	static constexpr ValueType get_value_type() {
		using raw_t = typename std::remove_cv<T>::type;
		if constexpr (std::is_same<raw_t, int8_t>::value) return ValueType::Int8;
		else if constexpr (std::is_same<raw_t, uint8_t>::value) return ValueType::UInt8;
		else if constexpr (std::is_same<raw_t, int16_t>::value) return ValueType::Int16;
		else if constexpr (std::is_same<raw_t, uint16_t>::value) return ValueType::UInt16;
		else if constexpr (std::is_same<raw_t, int32_t>::value) return ValueType::Int32;
		else if constexpr (std::is_same<raw_t, uint32_t>::value) return ValueType::UInt32;
		else if constexpr (std::is_same<raw_t, int64_t>::value) return ValueType::Int64;
		else if constexpr (std::is_same<raw_t, uint64_t>::value) return ValueType::UInt64;
		else if constexpr (std::is_same<raw_t, float>::value) return ValueType::Float;
		else if constexpr (std::is_same<raw_t, double>::value) return ValueType::Double;
		else return ValueType::Unknown;
	}

public:

	enum SOURCE_MODE {
		AUTO = 0, COPY, REF, OWN,
		CONST = 0b10000000
	};

	struct data_set {

		unsigned char info = 0u;
		union {
			struct {
				void* data = nullptr;
				double (*to_double)(const void* const&, const size_t&) = nullptr;
				double min, max;
			} array;
			struct {
				double start, end;
			} range;
		};

		template <typename iterator, typename type = double>
		data_set(iterator itr, size_t size, const SOURCE_MODE& mode = AUTO, double (*convert)(const void* const&, const size_t&) = nullptr) {
			if (is_number_pointer<iterator>() && ((mode & 0b011111111) != COPY) || convert != nullptr) {
				array.data = (void*)itr;
				info = (convert != nullptr) ? ValueType::Unknown : get_value_type<inner_t<iterator>>();
				if (is_pointer_to_const<iterator>() || mode & 0b10000000) info |= 0b10000000;
				if ((mode & 0b011111111) == OWN) info |= 0b01000000; // should delete
				array.to_double = convert;
				if (convert == nullptr) {
					array.min = array.max = (double)*itr++;
					while (--size) {
						if ((double)*itr > array.max) array.max = (double)*itr;
						else if ((double)*itr < array.min) array.min = (double)*itr;
						++itr;
					}
				}
				else {
					array.min = array.max = convert(itr, 0);
					for (size_t i = 1; i < size; ++i) {
						const double value = convert(itr, i);
						if (value > array.max) array.max = value;
						else if (value < array.min) array.min = value;
					}
				}
			}
			else {
				array.data = malloc(sizeof(type) * size);
				type* i = (type*)array.data;
				array.min = array.max = (double)*i;
				while (size--) {
					*i = *itr++;
					if ((double)*i > array.max) array.max = (double)*i;
					else if ((double)*i < array.min) array.min = (double)*i;
					++i;
				}
				info = get_value_type<type>();
				info |= 0b01000000; // should delete
				info |= 0b10000000; /// const?????
			}		
		}

		data_set(const double& start, const double& end) {
			info = 0b001000000; // type : range
			range.start = start;
			range.end = end;
		}

		double min() const {
			if (info & 0b001000000) return range.start;
			else return array.min;
		}

		double max() const {
			if (info & 0b001000000) return range.end;
			else return array.max;
		}

		double double_value(const size_t& i) const {
			switch (info & 0b000111111) {
				case Int8: return (double)((char*)array.data)[i];
				case UInt8: return (double)((unsigned char*)array.data)[i];
				case Int16: return (double)((short*)array.data)[i];
				case UInt16: return (double)((unsigned short*)array.data)[i];
				case Int32: return (double)((int*)array.data)[i];
				case UInt32: return (double)((unsigned int*)array.data)[i];
				case Int64: return (double)((long long*)array.data)[i];
				case UInt64: return (double)((unsigned long long*)array.data)[i];
				case Float: return (double)((float*)array.data)[i];
				case Double: return ((double*)array.data)[i];
			}
			if (array.to_double != nullptr) return array.to_double(array.data, i);
			return 0;
		}

		bool to_delete() const {
			return info & 0b01000000;
		}

		bool source_type() const {
			return info & 0b00100000;
		}

		bool is_const() const {
			return info & 0b10000000;
		}
	};

private:

	struct data_pair {
		data_set x, y;
		size_t _size = 0u;
		unsigned char copy_info = 0u;
		unsigned int color = 0u;
	};
	data_pair* pairs = nullptr;
	size_t _size = 0u;
	std::string title, x_label, y_label;

	double _step(double& start, double& end, const double& scale = 5) const {
		const double range = std::abs(end - start);
		const double magnitude = log10(range) / log10(scale);
		double s = pow(scale, (int)magnitude);
		while (range / s < 10) s /= scale;
		if (range / s != 10) s *= scale;
		if (fmod(start, s) != 0) {
			if (start > 0) start = start - fmod(start, s);
			else start = start - fmod(start, s) - s;
		}
		if (fmod(end, s) != 0) {
			if (end > 0) end = end - fmod(end, s) + s;
			else end = end - fmod(end, s);
		}
		return s;
	}

	bool const_sources = true;

	double __min_x, __max_x, __min_y, __max_y;

public:

	path_f draw_layer;

	void add(data_set x, data_set y, const size_t& size, const unsigned int& color) {
		data_pair* resized = (data_pair*)malloc(sizeof(data_pair) * ++_size);
		if (_size - 1) {
			memcpy(resized, pairs, sizeof(data_pair) * (_size - 1));
			free(pairs);
		}
		pairs = resized;
		resized += _size - 1;
		resized->_size = size;
		resized->color = color;
		resized->x = x;
		resized->y = y;
		const_sources &= x.is_const() & y.is_const();

		if (!_size) {
			__min_x = x.min();
			__max_x = x.max();
			__min_y = y.min();
			__max_y =  y.max();
		}
		else {
			double value = x.min();
			if (value < __min_x) __min_x = value;
			value = x.max();
			if (value > __max_x) __max_x = value;
			value = y.min();
			if (value < __min_y) __min_y = value;
			value = y.max();
			if (value > __max_y) __max_y = value;
		}
	}

	void set_title(const std::string& str) { title = str; }

	void set_x_label(const std::string& label) { x_label = label; }
	void set_y_label(const std::string& label) { y_label = label; }

#ifndef __DISABLE_CANVAS
	template <typename Iterable1, typename Iterable2>
	static int* render(Iterable1 x, Iterable2 y, const unsigned int& size, const unsigned int& width = 500u, const unsigned int& height = 500u, const plot_theme& theme = plot_theme::light) {
		double min_x = *x, max_x = *x, max_y = *y, min_y = *y;
		{
			Iterable1 x_i = x;
			Iterable2 y_i = y;
			for (unsigned int i = 0; i < size; ++i) {
				if (x[i] > max_x) max_x = *x_i;
				else if (x[i] < min_x) min_x = *x_i;
				if (y[i] > max_y) max_y = *y_i;
				else if (y[i] < min_y) min_y = *y_i;
				++x_i;
				++y_i;
			}
		}
		const size_t size1 = width * height;
		int* const __restrict data = (int*)malloc(size1 * sizeof(int));
		if (!theme.background.is_shader())
			memset(data, theme.background.argb(), size1 * sizeof(int));
		else {
			shader_info info {0, 0, width, height, 0, 0, 0, 0};
			for (unsigned int i = 0; i < size1; ++i) {
				info.x = i % width;
				info.y = i / width;
				data[i] = theme.background.shader(info);
			}
		}
		canvas canvas{ data, width, height };

		//std::cout << min_x << ' ' << max_x << ' ' << min_y << ' ' << max_y << '\n';

		const unsigned int view_height = height - (theme.v_padding << 1), view_width = width - (theme.h_padding << 1);
			
			{
				double step = _step(min_x, max_x);
				max_x -= min_x;
				//printf("st %lf\n", step);
				//std::cout << "-------------Step: " << step << '\n';

				//char* const buffer = (char*)alloca(50);
				const double step_i = (step / max_x) * (view_width - 1);
				for (double i = (width - view_width) / 2 + -fmod(min_x, step) / max_x * (view_width - 1), j = min_x - fmod(min_x, step); i < width - theme.h_padding; i += step_i, j += step) {
					//unsigned int text_width = sprintf_s(buffer, 49, "%lf", j) * (3 * 3 + 1) - 1;
					std::ostringstream oss;
					oss << std::noshowpoint << j;
					unsigned int text_width = oss.str().size() * (3 * theme.font_size + 1) - 1;
					canvas.draw_line(i, height - (height - view_height) / 2 - 1, i, (height - view_height) / 2, theme.grid); //change with canvas.drawVerticalLine
					canvas.draw_text(oss.str().c_str(), i - (text_width >> 1), view_height + (height - view_height) / 2 + theme.axis_padding, theme.foreground, theme.font_size);
				}
			}

			{
				double step = _step(min_y, max_y);
				//puts("y step done");
				max_y -= min_y;
				//std::cout << "-------------Step_y: " << step << '\n';

				//char* const buffer = (char*)alloca(50);
				const double step_i = (step / max_y) * (view_height - 1);
				//std::cout << max_y << " xd\n";
				for (double i = view_height - (-fmod(min_y, step) / max_y * (view_height - 1)) + (height - view_height) / 2, j = min_y - fmod(min_y, step); i >= theme.v_padding; i -= step_i, j += step) {
					//puts("enter");
					//unsigned int text_width = sprintf_s(buffer, 49, "%lf", j) * (3 * 3 + 1) - 1;
					std::ostringstream oss;
					oss << std::noshowpoint << j;
					//printf("%s\n", oss.str().c_str());
					unsigned int text_width = oss.str().size() * (3 * theme.font_size + 1) - 1;
					canvas.draw_line((width - view_width) / 2, i, (width - view_width) / 2 + view_width, i, theme.grid); //change with canvas.drawVerticalLine
					int pos_x = theme.h_padding - text_width - theme.axis_padding;
					if (pos_x < 0) pos_x = 0;
					canvas.draw_text(oss.str().c_str(), pos_x, i - 7, theme.foreground, theme.font_size);
					//puts(oss.str().c_str());
					//puts("i");
				}
			}


			/*int floor_max_x = (int)max_x, ceil_min_x = (int)min_x;
			unsigned int range = floor_max_x - ceil_min_x;

			double tick_steps[] = { .1, .2, .5 };
			const unsigned int max_ticks = 10;
			double step = *(std::end(tick_steps) - 1);
			bool found = false;
			while (!found)
				for (double& d : tick_steps) {
					if (range / d <= max_ticks) {
						step = d;
						found = true;
						break;
					}
					d *= 10;
				}
			
			char* const buffer = (char*)alloca((int)log10(floor_max_x) + 5);
			const double step_i = (step / max_x) * (view_width - 1);
			for (double i = (width - view_width) / 2 + ((double)(ceil_min_x) - min_x) / max_x * (view_width - 1), j = ceil_min_x; i < width; i += step_i, j += step) {
				unsigned int text_width = sprintf_s(buffer, (int)log10(floor_max_x) + 5, "%.1lf", j) * (3 * 3 + 1) - 1;
				canvas.drawLine2(i, view_height - 1, i, 0u, theme.foreground); //change with canvas.drawVerticalLine
				canvas.draw_text(buffer, i - (text_width >> 1), view_height + (30 - 15) / 2, theme.foreground, 3);
			}*/
		//}

		unsigned char* const alpha_map = (unsigned char*)calloc(view_width * view_height, 1);
		canvas = { alpha_map, view_width, view_height, IMAGE_GRAYSCALE };
		canvas.set_blend_mode(blend_mode::LIGHTER);
		double norm_prev_x = (double)(*x++ - min_x) / max_x * (view_width - 1), norm_prev_y = (double)(max_y - *y++ + min_y) / max_y * (view_height - 1);
		bool d = true; ///// DEBUG
		for (unsigned int i = 1; i < size; ++i) {
			double norm_x = (double)(*x++ - min_x) / max_x * (view_width - 1), norm_y = (double)(max_y - *y++ + min_y) / max_y * (view_height - 1);

			//std::cout << norm_prev_x << ' ' << norm_prev_y << ' ' << norm_x << ' ' << norm_y << '\n';
			if (d) canvas.draw_line_2(norm_prev_x, norm_prev_y, norm_x, norm_y, 0xFFFFFFFF);
			//d = !d;
			norm_prev_x = norm_x;
			norm_prev_y = norm_y;
		}

		// ANTIALIAS
		/*unsigned int a_y = 0, a_x = 0;
		while (alpha_map[view_width * a_y] == 0) ++a_y;
		--a_y;
		while (a_x < view_width) {
			alpha_map[view_width * a_y + a_x] = 100;
			if (alpha_map[view_width * a_y + a_x + 1] == 0xFFu && a_y) --a_y;
			else {
				++a_x;
			}
		}*/

		canvas = { data, width, height };
		canvas.draw(alpha_map, view_width, view_height, (width - view_width) / 2, (height - view_height) / 2, theme.marker_color);
		free(alpha_map);

		//canvas.draw_line(theme.h_padding / 2, theme.v_padding / 2, theme.h_padding / 2, height - theme.v_padding / 2, 0xFF000000);

		return data;
	}
#endif

	template <typename prec = float>
	path<prec>* to_paths(const unsigned short& scale_x = 1u, const unsigned short& scale_y = 1u, const prec& simplify_threshold = 1, const prec& offset_x = 0, const prec& offset_y = 0, const prec& font_size = 0) const {
		double min_x = pairs->x.double_value(0), max_x = pairs->x.double_value(0), max_y = pairs->y.double_value(0), min_y = pairs->y.double_value(0);
		{
			for (unsigned int j = 0; j < _size; ++j) {
				//printf("%u\n", pairs[j]._size);
				for (unsigned int i = 0; i < pairs[j]._size; ++i) {
					const double x_i = pairs[j].x.double_value(i), y_i = pairs[j].y.double_value(i);
					if (x_i > max_x) max_x = x_i;
					else if (x_i < min_x) min_x = x_i;
					if (y_i > max_y) max_y = y_i;
					else if (y_i < min_y) min_y = y_i;
				}
			}
		}
		
		const double step_x = _step(min_x, max_x, 2), step_y = _step(min_y, max_y, 2); // breaks if max = min

		const size_t layers_size = 2 + _size + !draw_layer.empty();
		path<prec>* layers = new path<prec>[layers_size]{};

		max_x -= min_x;
		max_y -= min_y;

		const double font = font_size ? font_size : (.03 * (scale_x > scale_y ? scale_x : scale_y));

		// grid & numbers
		layers[0].set_stroke_fill(0xD3D3D3FF);
		layers[0].set_stroke_width(.001 * ((scale_x > scale_y) ? scale_y : scale_x));
		path<prec>* fore = layers + _size + 1;
		for (double x_i = 0; x_i <= max_x; x_i += step_x) {
			layers[0].move_to(x_i / max_x * scale_x + offset_x, offset_y);
			layers[0].vertical_line_to(1 * scale_y + offset_y);
			std::ostringstream oss;
			oss << std::noshowpoint << min_x + x_i;
			fore->text(x_i / max_x * scale_x + offset_x, 1.1 * scale_y + offset_y, oss.str().c_str(), font);
			fore->move_to(x_i / max_x * scale_x + offset_x, .98 * scale_y + offset_y);
			fore->vertical_line_to(1.02 * scale_y + offset_y);
		}
		for (double y_i = 0; y_i <= max_y; y_i += step_y) {
			layers[0].move_to(offset_x, y_i / max_y * scale_y + offset_y);
			layers[0].horizontal_line_to(1 * scale_x + offset_x);
			std::ostringstream oss;
			oss << std::noshowpoint << min_y + y_i;
			fore->text(-.1 * scale_x + offset_x, (max_y - y_i) / max_y * scale_y + offset_y, oss.str().c_str(), font);
			fore->move_to(-.02 * scale_x + offset_x, y_i / max_y * scale_y + offset_y);
			fore->horizontal_line_to(.02 * scale_x + offset_x);
		}
		fore->set_stroke_fill(0xFF);
		fore->set_stroke_width(.003 * ((scale_x > scale_y) ? scale_y : scale_x));
		fore->move_to(offset_x, offset_y);
		fore->vertical_line_to(1.02 * scale_y + offset_y);
		fore->move_to(-.02 * scale_x + offset_x, 1 * scale_y + offset_y);
		fore->horizontal_line_to(1 * scale_x + offset_x);
		if (!title.empty()) fore->text(scale_x / 2. + offset_x, -.1 * scale_y + offset_y, title.c_str(), font);
		if (!x_label.empty()) fore->text(1.05 * scale_x + offset_x, scale_y + offset_y, x_label.c_str(), font);
		if (!y_label.empty()) fore->text(offset_x, -.05 * scale_y + offset_y, y_label.c_str(), font);
		// plot
		const prec stroke_width = .005 * ((scale_x > scale_y) ? scale_y : scale_x);
		for (unsigned int j = 0; j < _size; ++j) {
			layers[1 + j].set_stroke_fill(pairs[j].color);
			layers[1 + j].set_stroke_width(stroke_width);
			layers[1 + j].set_stroke_line_cap(path<prec>::stroke_line_cap::ROUND);
			layers[1 + j].set_stroke_line_join(path<prec>::stroke_line_join::ROUND);
			layers[1 + j].move_to((pairs[j].x.double_value(0) - min_x) / max_x * scale_x + offset_x, (max_y - pairs[j].y.double_value(0) + min_y) / max_y * scale_y + offset_y);
			unsigned int count = 0;
			prec y_origin = pairs[j].y.double_value(0), x_origin = pairs[j].x.double_value(0);
			for (unsigned int i = 1; i < pairs[j]._size; ++i) {
				//if (abs(pairs[j].y[i] - y_origin) / max_y > simplify_threshold || i == pairs[j]._size - 1) {
				//if ((abs(pairs[j].y[i] - y_origin) / max_y * scale_y) > stroke_width / 2 * simplify_threshold || i == pairs[j]._size - 1) {
				//if (abs(abs(pairs[j].y[i - 1] - y_origin) - abs(pairs[j].y[i] - pairs[j].y[i - 1])) / max_y * scale_y > stroke_width / 2 * simplify_threshold) {
				//const double ratio_1 = (pairs[j].y[i - 1] - y_origin) / (pairs[j].x[i - 1] - x_origin);
				//const double ratio_2 = (pairs[j].y[i] - pairs[j].y[i - 1]) / (pairs[j].x[i] - pairs[j].x[i - 1]);
				//if (ratio_1 / ratio_2 > simplify_threshold) {
				//if (abs(abs(pairs[j].y[i - 1] - y_origin) - abs(pairs[j].y[i] - pairs[j].y[i - 1])) / max_y * scale_y > stroke_width / 2 * simplify_threshold) {
					//layers[1 + j].line_to((pairs[j].x[i - 1] - min_x) / max_x * scale_x, (max_y - pairs[j].y[i - 1] + min_y) / max_y * scale_y);
					layers[1 + j].line_to((pairs[j].x.double_value(i) - min_x) / max_x * scale_x + offset_x, (max_y - pairs[j].y.double_value(i) + min_y) / max_y * scale_y + offset_y);
					y_origin = pairs[j].y.double_value(i - 1);
					x_origin = pairs[j].x.double_value(i - 1);
				//}
				//else ++count;
			}
			//printf("Saved: %u\n", count);
		}
		if (!draw_layer.empty()) {
			// (x - min_x) / (max_x - min_x) * scale_x + offset_x
			const prec mul_x = scale_x / max_x, mul_y = scale_y / max_y;
			layers[layers_size - 1] = draw_layer.__transform(min_x, min_y, max_y, mul_x, mul_y, offset_x, offset_y);
		}
		//printf("viewbox: %lf %lf %lf %lf\n", 0, 0, 1., 1.);
		return layers;
	}

	double _min_x() const {
		return __min_x;
	}

	double _min_y() const {
		return __min_y;
	}

	double _max_x() const {
		return __max_x;
	}

	double _max_y() const {
		return __max_y;
	}

	size_t size() const { return _size; }

	size_t layer_count() const {
		return _size + 2 + !draw_layer.empty();
	}

	void clear() {
		for (unsigned int i = 0; i < _size; ++i) {
			if (pairs[i].x.to_delete()) {
				free(pairs[i].x.array.data);
				//printf("deleted x %u\n", i);
			}
			if (pairs[i].y.to_delete()) {
				free(pairs[i].y.array.data);
				//printf("deleted y %u\n", i);
			}
		}
		if (pairs != nullptr) {
			free(pairs);
			pairs = nullptr;
			_size = 0u;
		}
		if (!draw_layer.empty()) draw_layer.clear();
	}

	~_plot() { clear(); }
};
#endif

class subplot {
	unsigned short rows = 0u, cols = 0u, padding = 1u;
	
	struct plot_data {
		const _plot* obj = nullptr;
		unsigned short row, col;
		unsigned short width, height;
	} *plots = nullptr;
	unsigned char* occupied = nullptr;
	size_t _size = 0u;

public:

	subplot(const unsigned short& rows, const unsigned short& cols) : rows(rows), cols(cols) {
		const size_t count = rows * cols;
		occupied = (uint8_t*)calloc((count >> 3) + bool(count & 7), 1);
	}

	void set_padding(const uint16_t& p) { padding = p; }

	void add(const _plot& p, const uint16_t& row, const uint16_t& col, const uint16_t& width, const uint16_t& height) {
		plot_data* temp = (plot_data*)malloc(sizeof(plot_data) * (_size + 1));
		memcpy(temp, plots, sizeof(plot_data) * _size);
		free(plots);
		plots = temp;
		plots[_size] = { &p, row, col, width, height };
		++_size;
	}

#ifdef __CPP_PLOT_PATH
	template <typename prec = float>
	path<prec>* to_paths() {
		size_t paths_size = 0u;
		for (unsigned int i = 0; i < _size; ++i) paths_size += plots[i].obj->layer_count();
		path<prec>* paths = new path<prec>[paths_size]{};
		unsigned short pad = 0u;
		//printf("c: %u\n", paths_size);
		for (unsigned int i = 0, itr = 0; i < _size; ++i) {
			const prec width = plots[i].width * 5 + (plots[i].width - 1) * padding, height = plots[i].height * 5 + (plots[i].height - 1) * padding;

			//const prec ratio = (prec)(plots[i].height > plots[i].width ? plots[i].height : plots[i].width) / (rows < cols ? rows : cols);

			const prec font_size = .04;//.02 / ratio;
			path<prec>* const paths_i = plots[i].obj->to_paths(width, height, (prec)1., (prec)plots[i].col * (5 + padding), (prec)plots[i].row * (5 + padding), font_size);
			for (unsigned int j = 0; j < plots[i].obj->layer_count(); ++j)
				paths[itr++] = std::move(paths_i[j]);
			delete[] paths_i;
			pad += padding;
		}
		//printf("c: %u\n", paths_size);
		return paths;
	}
#endif

	size_t layer_count() const {
		size_t result = 0u;
		for (size_t i = 0; i < _size; ++i) result += plots[i].obj->layer_count();
		return result;
	}

	~subplot() {
		free(occupied);
		if (plots != nullptr) free(plots);
	}
};