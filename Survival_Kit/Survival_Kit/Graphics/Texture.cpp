#include "../Graphics/Texture.h"
#include "../Graphics/stb_image.h"
#include "../Graphics/Common.h"
#include "../Manager/LogManager.h"

namespace gam300 {

	std::optional<std::vector<uint8_t>> Texture::load_pixels_rgba8(const std::filesystem::path& path,
																   bool flip_vertical,
																   int& out_w, 
																   int& out_h, 
																   int& out_channels) {

		stbi_set_flip_vertically_on_load(flip_vertical ? 1 : 0);
		int channels = 0;
		unsigned char* data = stbi_load(path.string().c_str(), &out_w, &out_h, &out_channels, 4); 
		if (!data) return std::nullopt; 

		size_t size = static_cast<size_t>(out_w) * static_cast<size_t>(out_h) * 4; 
		std::vector<uint8_t> pixels(size);
		std::memcpy(pixels.data(), data, size);
		stbi_image_free(data);
		return pixels;
	}

	static inline uint32_t calc_mip_count(uint32_t w, uint32_t h) {
		uint32_t m = 1; // Start with base level

		while (w > 1 || h > 1) { 

			// continues dividing width and height by half using bitwise shift
			// every division increase the mipmap level, std::max to ensure w and h never go below 1

			w = std::max(1u, w >> 1); 
			h = std::max(1u, h >> 1); 
			++m; 
		}

		return m;
	}

	uint64_t Texture::create_gpu_texture_rgba8(const uint8_t* pixels, uint32_t w, uint32_t h,
											   bool srgb, bool gen_mips, uint32_t& out_mip_levels) {

		GLuint tex = 0;
		glCreateTextures(GL_TEXTURE_2D, 1, &tex);
		const GLenum internalFmt = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
		const u32 mips = gen_mips ? calc_mip_count(w, h) : 1;
		glTextureStorage2D(tex, mips, internalFmt, w, h);

		glTextureSubImage2D(tex, 0, 0, 0, w, h, GL_RGBA8, GL_UNSIGNED_BYTE, pixels);

		if (gen_mips && mips > 1) {
			glGenerateTextureMipmap(tex);
		}

		out_mip_levels = mips;
		return static_cast<u64>(tex);
	}

	void Texture::destroy_gpu_texture(uint64_t handle) {
		if (handle == 0) return;

		GLuint tex = static_cast<GLuint>(handle);
		glDeleteTextures(1, &tex);
	}

    std::optional<Texture> Texture::load_from_file(const std::filesystem::path& path,
												   const TextureDesc& desc) {

		int w = 0, h = 0, c;
		auto pixels = load_pixels_rgba8(path, desc.flip_verticals, w, h, c);
		if (!pixels) {
			LM.writeLog("Failed to load texture from: %s", path.string().c_str());
			return std::nullopt;
		}

		u32 mip_levels = 0;
		const u64 handle = create_gpu_texture_rgba8(pixels->data(), 
												    static_cast<u32>(w), 
													static_cast<u32>(h), 
													desc.srgb, 
													desc.generate_mips,
													mip_levels);

		if (handle == 0) {
			LM.writeLog("Failed to generate texture handle");
			return std::nullopt;
		}

		return Texture{ handle, static_cast<u32>(w), static_cast<u32>(h), mip_levels, desc.srgb };
	}
}