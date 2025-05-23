/*
 * \brief  Implementation of 'Text_painter::Font' for VFS-mounted fonts
 * \author Norman Feske
 * \date   2018-03-26
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__GEMS__VFS_FONT_T_
#define _INCLUDE__GEMS__VFS_FONT_T_

#include <os/vfs.h>
#include <nitpicker_gfx/text_painter.h>

namespace Genode { class Vfs_font; }


class Genode::Vfs_font : public Text_painter::Font
{
	public:

		using Glyph = Glyph_painter::Glyph;

		static constexpr Vfs::file_size GLYPH_SLOT_BYTES = 64*1024;

		class Glyph_header
		{
			private:

				uint8_t _width              = 0;
				uint8_t _height             = 0;
				uint8_t _vpos               = 0;
				int8_t  _advance_decimal    = 0;
				uint8_t _advance_fractional = 0;
				uint8_t _reserved[3] { };

				Glyph::Opacity _values[];

				float _advance() const
				{
					float value = 256.0f*_advance_decimal + _advance_fractional;
					return value/256;
				}

			public:

				Glyph_header(Glyph const &glyph)
				:
					_width ((uint8_t)min(255U, glyph.width)),
					_height((uint8_t)min(255U, glyph.height)),
					_vpos  ((uint8_t)min(255U, glyph.vpos)),
					_advance_decimal((int8_t)max(-127, min(127, glyph.advance.decimal()))),
					_advance_fractional((uint8_t)glyph.advance.value & 0xff)
				{ }

				Glyph_header() { }

				Advance_info advance_info() const
				{
					return { .width = _width, .advance = _advance() };
				}

				void with_glyph(Text_painter::Area const bb, auto const &fn) const
				{
					/* consider glyph bounds exceeding the font's bounding box */
					unsigned const clipped_h = _width ? unsigned(bb.count() / _width) : 0u;

					fn(Glyph { .width   = _width,
					           .height  = min(unsigned(_height), clipped_h),
					           .vpos    = _vpos,
					           .advance = _advance(),
					           .values  = _values });
				}

		} __attribute__((packed));

	private:

		using Codepoint = Text_painter::Codepoint;
		using Area      = Text_painter::Area;
		using Path      = Directory::Path;

		Directory const _font_dir;
		unsigned  const _baseline;
		Area      const _bounding_box;
		unsigned  const _height;

		struct Glyph_buffer : Byte_range_ptr
		{
			Allocator &_alloc;

			Glyph_header &header { *(Glyph_header *)start };

			static size_t _bytes(Area size) { return sizeof(Glyph_buffer) + size.count()*4; }

			Glyph_buffer(Allocator &alloc, Area size)
			:
				Byte_range_ptr((char *)alloc.alloc(_bytes(size)), _bytes(size)),
				_alloc(alloc)
			{ }

			~Glyph_buffer() { _alloc.free(start, num_bytes); }
		};

		Glyph_buffer mutable _buffer;

		Readonly_file _glyphs_file;

		template <typename T, unsigned MAX_LEN = 128>
		static T _value_from_file(Directory const &dir, Path const &path,
		                          T const &default_value)
		{
			T result = default_value;
			try {
				Readonly_file const file(dir, path);
				char buf[MAX_LEN + 1] { };
				if (file.read(Byte_range_ptr { buf, sizeof(buf) }) <= MAX_LEN)
					if (ascii_to(buf, result))
						return result;
			}
			catch (Readonly_file::Open_failed) { }
			return default_value;
		}

		static Readonly_file::At _file_pos(Codepoint c)
		{
			return Readonly_file::At{(Vfs::file_size)c.value*GLYPH_SLOT_BYTES};
		}

	public:

		struct Unavailable : Exception { };

		/**
		 * Constructor
		 *
		 * \param alloc  allocator for glyph buffer
		 * \param dir    directory
		 * \param path   path to font
		 *
		 * \throw Unavailable  unable to obtain font data
		 */
		Vfs_font(Allocator &alloc, Directory const &dir, Path const &path)
		:
			_font_dir(dir, path),
			_baseline(_value_from_file(_font_dir, "baseline", 0U)),
			_bounding_box(_value_from_file(_font_dir, "max_width",  0U),
			              _value_from_file(_font_dir, "max_height", 0U)),
			_height(_value_from_file(_font_dir, "height", 0U)),
			_buffer(alloc, _bounding_box),
			_glyphs_file(_font_dir, "glyphs")
		{ }

		void _apply_glyph(Codepoint c, Apply_fn const &fn) const override
		{
			_glyphs_file.read(_file_pos(c), _buffer);

			_buffer.header.with_glyph(_bounding_box, [&] (Glyph const &glyph) {
				fn.apply(glyph); });
		}

		Advance_info advance_info(Codepoint c) const override
		{
			Byte_range_ptr header_buffer { _buffer.start, sizeof(Glyph_header) };

			_glyphs_file.read(_file_pos(c), header_buffer);

			return _buffer.header.advance_info();
		}

		unsigned baseline() const override { return _baseline; }
		unsigned   height() const override { return _height; }
		Area bounding_box() const override { return _bounding_box; }
};

#endif /* _INCLUDE__GEMS__TTF_FONT_T_ */
