#pragma once

#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <cstdint>
#include <optional>
#include <filesystem>
#include <string>

namespace gam300 {

	// Load time options (texture data)
	struct TextureDesc {
		bool srgb = true;
		bool generate_mips = true;
		bool flip_verticals = true;
	};

	class Texture {
	public:

		// Disable default ctor, construct textures only via factory method
		Texture() = delete;

		// Factory
		static std::optional<Texture> load_from_file(const std::filesystem::path& path,
													 const TextureDesc& desc);

		Texture(Texture&& other) noexcept { move_from(other); }
		Texture& operator=(Texture&& other) noexcept {
			if (this != &other) { destroy(); move_from(other); }
			return *this;
		}

		~Texture() noexcept { destroy(); }

		uint32_t width()  const noexcept { return m_width; }
		uint32_t height() const noexcept { return m_height; }
		uint32_t mips()   const noexcept { return m_mip_levels; }
		bool     is_srgb()const noexcept { return m_srgb; }
		bool     valid()  const noexcept { return m_handle != kInvalid; }

		uint64_t handle() const noexcept { return m_handle; }

	private:
		// Private ctor used by factory
		Texture(uint64_t handle, uint32_t w, uint32_t h, uint32_t mip_levels, bool srgb) noexcept
			: m_handle(handle), m_width(w), m_height(h), m_mip_levels(mip_levels), m_srgb(srgb) { }

		// GL hooks
		static std::optional<std::vector<uint8_t>> load_pixels_rgba8(const std::filesystem::path& path,
																	 bool flip_vertical,
																	 int& out_w, int& out_h, int& out_channels);

		static uint64_t create_gpu_texture_rgba8(const uint8_t* pixels, uint32_t w, uint32_t h,
												 bool srgb, bool gen_mips, uint32_t& out_mip_levels);

		static void     destroy_gpu_texture(uint64_t handle);

		// RAII Helpers
		void destroy() noexcept {
			if (m_handle != kInvalid) { destroy_gpu_texture(m_handle); m_handle = kInvalid; }
			m_width = m_height = m_mip_levels = 0; m_srgb = false;
		}

		void move_from(Texture& o) noexcept {
			m_handle = o.m_handle; m_width = o.m_width; m_height = o.m_height;
			m_mip_levels = o.m_mip_levels; m_srgb = o.m_srgb;
			o.m_handle = kInvalid; o.m_width = o.m_height = o.m_mip_levels = 0; o.m_srgb = false;
		}

		// Data
		static constexpr uint64_t kInvalid = 0;
		uint64_t m_handle = kInvalid;      // e.g., GL texture id, D3D SRV handle, VkImageView+layout token, etc.
		uint32_t m_width = 0, m_height = 0, m_mip_levels = 0;
		bool     m_srgb = false;
	};
}

#endif // !__TEXTURE_H__
