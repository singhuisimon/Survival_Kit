#pragma once

#ifndef __SAMPLER_H__
#define __SAMPLER_H__

#include <optional>
#include "../Graphics/Common.h"

namespace gam300 {

	// Sampler State
	enum class Wrap : uint8_t {
		CLAMP = 0,
		REPEAT,
		MIRROR
	};

	enum class Filter : uint8_t {
		NEAREST = 0,
		LINEAR
	};

	enum class MipFilter : uint8_t {
		NONE = 0,
		NEAREST,
		LINEAR
	};

    // ---------- Sampler = sampling rules (no pixels) ----------
    struct SamplerDesc {
        Wrap  wrap_u = Wrap::REPEAT;
        Wrap  wrap_v = Wrap::REPEAT;
        Wrap  wrap_w = Wrap::REPEAT; // used for 3D textures/cubemaps
        Filter min_filter = Filter::LINEAR;
        Filter mag_filter = Filter::LINEAR;
        MipFilter mip_filter = MipFilter::LINEAR;
        uint32_t max_anisotropy = 1; // 1 = off
    };

    class Sampler {
    public:
        Sampler() = delete;

        static std::optional<Sampler> create(const SamplerDesc& desc);

        Sampler(Sampler&& o) noexcept { move_from(o); }
        Sampler& operator=(Sampler&& o) noexcept {
            if (this != &o) { destroy(); move_from(o); }
            return *this;
        }
        ~Sampler() noexcept { destroy(); }

        uint64_t handle() const noexcept { return m_handle; }
        bool     valid()  const noexcept { return m_handle != kInvalid; }

    private:
        explicit Sampler(uint64_t h) noexcept : m_handle(h) {}

        // ---- platform hooks (you implement these) ----
        static uint64_t create_gpu_sampler(const SamplerDesc& d);
        static void     destroy_gpu_sampler(uint64_t handle);

        void destroy() noexcept {
            if (m_handle != kInvalid) { destroy_gpu_sampler(m_handle); m_handle = kInvalid; }
        }

        void move_from(Sampler& o) noexcept {
            m_handle = o.m_handle; o.m_handle = kInvalid;
        }

        static constexpr uint64_t kInvalid = 0;
        uint64_t m_handle = kInvalid;  // e.g., GL sampler object, D3D sampler state, VkSampler
    };
}

#endif // !__SAMPLER_H__


