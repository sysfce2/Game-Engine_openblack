/* openblack - A reimplementation of Lionhead's Black & White.
 *
 * openblack is the legal property of its developers, whose names
 * can be found in the AUTHORS.md file distributed with this source
 * distribution.
 *
 * openblack is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * openblack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with openblack. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstdint>
#include <memory>

#include <Graphics/Texture2D.h>

namespace openblack::graphics {

class FrameBuffer {
public:
	FrameBuffer() = delete;
	FrameBuffer(uint32_t width, uint32_t height, Format format);
	~FrameBuffer();

	void Bind();
	void Unbind();

	Texture2D& GetTexture() { return *_texture; }

	//inline void Bind() { glBindTexture(GL_TEXTURE_RECTANGLE, _textureID); }

	[[nodiscard]] uint32_t GetWidth() const { return _width; }
	[[nodiscard]] uint32_t GetHeight() const { return _height; }

private:
	uint32_t _handle;

	uint32_t _width;
	uint32_t _height;
	Format _format;

	std::unique_ptr<Texture2D> _texture;
};

}  // namespace openblack::graphics
