#pragma once
#ifndef __SERIALISATIONBIN_MANAGER_H__
#define __SERIALISATIONBIN_MANAGER_H__

#include <array>
#include <cstdint>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ECSManager.h"
#include "LogManager.h"
#include "../Component/Transform3D.h"


#define SEB gam300::SerializerBin::getInstance()

namespace gam300
{
	//===================== Reflection core =====================
	/**************************************************************************
	 * @brief
	 * Trait to detect whether a type provides `reflect<T>::fields()`.
	 * @tparam T
	 * Candidate type.
	 * @return
	 * `true_type` if `reflect<T>::fields()` is well-formed; otherwise false.
	 **************************************************************************/
	template <typename T>
	struct reflect
	{
		// Specialize with: static constexpr auto fields() { return std::make_tuple(&T::a, &T::b, ...); }
	};

	template <typename T, typename = void>
	struct is_reflected : std::false_type {};
	template <typename T>
	struct is_reflected<T, std::void_t<decltype(reflect<T>::fields())>> : std::true_type {};

	template <typename T>
	inline constexpr bool is_reflected_v = is_reflected<T>::value;

	/**************************************************************************
	 * @brief
	 * Helper macro to declare reflection for a user type.
	 * @param TYPE
	 * The user type being reflected.
	 * @param ...
	 * Pointers-to-members in the serialization order.
	 * @details
	 * Example:
	 *   struct Transform { Vec3 pos, rot, scale; };
	 *   REFLECT_TYPE(Transform, &Transform::pos, &Transform::rot, &Transform::scale);
	 **************************************************************************/

#define REFLECT_TYPE(TYPE, ...)                    \
	  template<> struct ::gam300::reflect<TYPE> {      \
	    static constexpr auto fields() {               \
	      return std::make_tuple(__VA_ARGS__);         \
	    }                                              \
	  }

	struct SceneFileHeader
	{
		std::uint32_t magic = 0x424E4353; // 'SCNB' in LE
		std::uint16_t major = 1;
		std::uint16_t minor = 0;
	};
	REFLECT_TYPE(SceneFileHeader, &SceneFileHeader::magic, &SceneFileHeader::major, &SceneFileHeader::minor);

	// Basic math bin type
	struct Vector3DBin
	{
		float x{}, y{}, z{};
	};
	REFLECT_TYPE(Vector3DBin, &Vector3DBin::x, &Vector3DBin::y, &Vector3DBin::z);

	// Transform component payload (binary)
	struct Transform3DBin
	{
		Vector3DBin position{};
		Vector3DBin prev_position{};   // kept for parity with your JSON
		Vector3DBin rotation{};
		Vector3DBin scale{ 1.f, 1.f, 1.f };
	};

	REFLECT_TYPE(Transform3DBin,
		&Transform3DBin::position,
		&Transform3DBin::prev_position,
		&Transform3DBin::rotation,
		&Transform3DBin::scale
	);

	// One scene object (entity)
	struct SceneObjectBin
	{
		std::string name;
		std::optional<Transform3DBin> transform3d; // optional = present or not
	};
	REFLECT_TYPE(SceneObjectBin, &SceneObjectBin::name, &SceneObjectBin::transform3d);

	// Whole scene
	struct SceneBin
	{
		std::vector<SceneObjectBin> objects;
	};
	REFLECT_TYPE(SceneBin, &SceneBin::objects);

	/**************************************************************************
	 * @brief
	 * Binary (de)serialization utilities with compile-time type reflection.
	 * Supports PODs, contiguous containers (std::string/std::vector),
	 * fixed-size arrays, optionals, pairs, maps/unordered_maps, and user
	 * types that declare fields via REFLECT_TYPE.
	 * @details
	 * Reflection is provided by specializing `reflect<T>::fields()` to return
	 * a tuple of pointers-to-members. The serializer iterates fields in that
	 * order and recursively (de)serializes each.
	 **************************************************************************/
	class SerializerBin
	{
	public:
		// Instance
		static SerializerBin &getInstance() // <-- make static
		{
			static SerializerBin instance{};
			return instance;
		}

		bool saveScene(const std::string &filename);
		bool loadScene(const std::string &filename);

		//===================== File helpers =====================
		template <class T>
		static inline bool save(const char *path, const T &obj)
		{
			std::ofstream os(path, std::ios::binary);
			if (!os.is_open()) return false;
			write_any(os, obj);
			return static_cast<bool>(os);
		}

		template <class T>
		static inline bool load(const char *path, T &obj)
		{
			std::ifstream is(path, std::ios::binary);
			if (!is.is_open()) return false;
			read_any(is, obj);
			return static_cast<bool>(is);
		}

		//===================== Core dispatch =====================
		/**************************************************************************
		 * @brief
		 * Serialize any supported type using compile-time dispatch.
		 * @tparam T
		 * Type to serialize.
		 * @param os
		 * Output stream (binary).
		 * @param v
		 * Value to serialize.
		 * @details
		 * Order of preference:
		 * 1) contiguous + resizable containers (bulk or element-wise),
		 * 2) trivially copyable PODs (raw bytes),
		 * 3) reflected types (field-wise), else static-assert.
		 **************************************************************************/
		template <class T>
		static void write_any(std::ofstream &os, const T &v)
		{
			if constexpr (is_supported_container_v<T>)
			{
				// std::string, std::vector<T>, std::array<T,N>, etc.
				write_container(os, v);
			}
			else if constexpr (std::is_trivially_copyable_v<T>)
			{
				// PODs / trivially copyable types
				write_data(os, v);
			}
			else if constexpr (is_reflected_v<T>)
			{
				// User-reflected types
				write_reflected(os, v);
			}
			else
			{
				static_assert(is_reflected_v<T>,
					"Type is neither trivially copyable, supported container, nor reflected. "
					"Add REFLECT_TYPE(T, &T::field, ...) or provide a write_any/read_any overload.");
			}
		}

		/**************************************************************************
		 * @brief
		 * Deserialize any supported type using compile-time dispatch.
		 * @tparam T
		 * Type to deserialize.
		 * @param is
		 * Input stream (binary).
		 * @param v
		 * Destination variable to populate.
		 * @details
		 * Mirrors `write_any` path selection.
		 **************************************************************************/
		template <class T>
		static void read_any(std::ifstream &is, T &v)
		{
			if constexpr (is_supported_container_v<T>)
			{
				// std::string, std::vector<T>, std::array<T,N>, etc.
				read_container(is, v);
			}
			else if constexpr (std::is_trivially_copyable_v<T>)
			{
				// PODs / trivially copyable types
				read_data(is, v);
			}
			else if constexpr (is_reflected_v<T>)
			{
				// User-reflected types
				read_reflected(is, v);
			}
			else
			{
				static_assert(is_reflected_v<T>,
					"Type is neither trivially copyable, supported container, nor reflected. "
					"Add REFLECT_TYPE(T, &T::field, ...) or provide a write_any/read_any overload.");
			}
		}

	private:
		//===================== u64 size I/O =====================
		/**************************************************************************
		 * @brief
		 * Write an integer as a 64-bit little-endian size tag.
		 * @tparam T
		 * Integral type accepted.
		 * @param os
		 * Output stream.
		 * @param n
		 * Value to cast and write.
		 **************************************************************************/
		template <typename T>
		static inline void write_u64(std::ofstream &os, T n)
		{
			static_assert(std::is_integral_v<T>);
			std::uint64_t v = static_cast<std::uint64_t>(n);
			os.write(reinterpret_cast<const char *>(&v), sizeof(v));
		}

		/**************************************************************************
		 * @brief
		 * Read a 64-bit little-endian size tag.
		 * @param is
		 * Input stream.
		 * @return
		 * Read size as std::uint64_t.
		 **************************************************************************/
		static inline std::uint64_t read_u64(std::ifstream &is)
		{
			std::uint64_t v{};
			is.read(reinterpret_cast<char *>(&v), sizeof(v));
			return v;
		}

		//===================== traits & tuple utils =====================
		/**************************************************************************
		 * @brief
		 * Detect "contiguous + resizable" containers (e.g., std::string/std::vector).
		 * Requires: value_type, size(), data(), resize(size_t).
		 **************************************************************************/
		template <class C>
		struct is_contig_resizable
		{
		private:
			template <class T>
			static auto test(int) -> decltype(
				(void)typename T::value_type{},
				(void)std::declval<T &>().size(),
				(void)std::declval<T &>().data(),
				(void)std::declval<T &>().resize(std::size_t{}),
				std::true_type{}
				);
			template <class>
			static auto test(...) -> std::false_type;
		public:
			static constexpr bool value = decltype(test<C>(0))::value;
		};

		template <class C>
		inline static constexpr bool is_contig_resizable_v = is_contig_resizable<C>::value;

		// ---- std::array detection (needed by dispatch) ----
		template <typename> struct is_std_array : std::false_type {};
		template <typename T, std::size_t N>
		struct is_std_array<std::array<T, N>> : std::true_type {};
		template <typename T>
		inline static constexpr bool is_std_array_v = is_std_array<T>::value;

		// Unified container trait (string/vector) OR std::array
		template <typename T>
		inline static constexpr bool is_supported_container_v =
			is_contig_resizable_v<T> || is_std_array_v<T>;

		// tuple for_each
		template <typename Tuple, typename F, std::size_t... Is>
		static inline void tuple_for_each_impl(Tuple &&tp, F &&f, std::index_sequence<Is...>)
		{
			(f(std::get<Is>(std::forward<Tuple>(tp))), ...);
		}

		template <typename Tuple, typename F>
		static inline void tuple_for_each(Tuple &&tp, F &&f)
		{
			constexpr auto N = std::tuple_size<std::remove_reference_t<Tuple>>::value;
			tuple_for_each_impl(std::forward<Tuple>(tp), std::forward<F>(f), std::make_index_sequence<N>{});
		}

		//===================== trivially copyable =====================
		/**************************************************************************
		 * @brief
		 * Write trivially copyable as raw bytes.
		 * @tparam T
		 * Trivially copyable type.
		 * @param os
		 * Output stream.
		 * @param value
		 * Value to write.
		 **************************************************************************/
		template <class T>
		static std::enable_if_t<std::is_trivially_copyable_v<T> && !is_contig_resizable_v<T>>
			write_data(std::ofstream &os, const T &value)
		{
			os.write(reinterpret_cast<const char *>(&value), sizeof(T));
		}

		/**************************************************************************
		 * @brief
		 * Read trivially copyable from raw bytes.
		 * @tparam T
		 * Trivially copyable type.
		 * @param is
		 * Input stream.
		 * @param value
		 * Destination variable.
		 **************************************************************************/
		template <class T>
		static std::enable_if_t<std::is_trivially_copyable_v<T> && !is_contig_resizable_v<T>>
			read_data(std::ifstream &is, T &value)
		{
			is.read(reinterpret_cast<char *>(&value), sizeof(T));
		}

		//===================== Containers (contiguous, resizable) =====================

		/**************************************************************************
		 * @brief
		 * Write contiguous, resizable container with trivial elements in bulk.
		 * @tparam C
		 * Container type (e.g., std::string, std::vector<T> trivial).
		 * @param os
		 * Output stream.
		 * @param c
		 * Container to write (size + raw data).
		 **************************************************************************/
		template <class C>
		static std::enable_if_t<is_contig_resizable_v<C> &&std::is_trivially_copyable_v<typename C::value_type>>
			write_container(std::ofstream &os, const C &c)
		{
			write_u64(os, c.size());
			if (!c.empty())
				os.write(reinterpret_cast<const char *>(c.data()),
					sizeof(typename C::value_type) * c.size());
		}

		/**************************************************************************
		 * @brief
		 * Read contiguous, resizable container with trivial elements in bulk.
		 * @tparam C
		 * Container type (e.g., std::string, std::vector<T> trivial).
		 * @param is
		 * Input stream.
		 * @param c
		 * Destination container (resized then filled).
		 **************************************************************************/
		template <class C>
		static std::enable_if_t<is_contig_resizable_v<C> &&std::is_trivially_copyable_v<typename C::value_type>>
			read_container(std::ifstream &is, C &c)
		{
			const auto n = static_cast<std::size_t>(read_u64(is));
			c.resize(n);
			if (n)
				is.read(reinterpret_cast<char *>(c.data()),
					sizeof(typename C::value_type) * n);
		}

		/**************************************************************************
		 * @brief
		 * Write fixed-size std::array (bulk for trivial T, else element-wise).
		 * @tparam T
		 * Element type.
		 * @tparam N
		 * Number of elements.
		 * @param os
		 * Output stream.
		 * @param arr
		 * Array to write.
		 **************************************************************************/
		template <class T, std::size_t N>
		static void write_container(std::ofstream &os, const std::array<T, N> &arr)
		{
			if constexpr (std::is_trivially_copyable_v<T>)
			{
				os.write(reinterpret_cast<const char *>(arr.data()), sizeof(T) * N);
			}
			else
			{
				for (auto const &e : arr) write_any(os, e);
			}
		}

		/**************************************************************************
		 * @brief
		 * Read fixed-size std::array (bulk for trivial T, else element-wise).
		 * @tparam T
		 * Element type.
		 * @tparam N
		 * Number of elements.
		 * @param is
		 * Input stream.
		 * @param arr
		 * Destination array.
		 **************************************************************************/
		template <class T, std::size_t N>
		static void read_container(std::ifstream &is, std::array<T, N> &arr)
		{
			if constexpr (std::is_trivially_copyable_v<T>)
			{
				is.read(reinterpret_cast<char *>(arr.data()), sizeof(T) * N);
			}
			else
			{
				for (auto &e : arr) read_any(is, e);
			}
		}

		/**************************************************************************
		 * @brief
		 * Write std::optional<T> as (has_value flag + payload if present).
		 * @tparam T
		 * Contained type.
		 * @param os
		 * Output stream.
		 * @param opt
		 * Optional to write.
		 **************************************************************************/
		template <class T>
		static void write_any(std::ofstream &os, const std::optional<T> &opt)
		{
			std::uint8_t has = opt.has_value() ? 1 : 0;
			os.write(reinterpret_cast<const char *>(&has), sizeof(has));
			if (has) write_any(os, *opt);
		}

		/**************************************************************************
		 * @brief
		 * Read std::optional<T> from (has_value flag + payload if present).
		 * @tparam T
		 * Contained type.
		 * @param is
		 * Input stream.
		 * @param opt
		 * Destination optional to populate.
		 **************************************************************************/
		template <class T>
		static void read_any(std::ifstream &is, std::optional<T> &opt)
		{
			std::uint8_t has{};
			is.read(reinterpret_cast<char *>(&has), sizeof(has));
			if (has) { T tmp{}; read_any(is, tmp); opt = std::move(tmp); }
			else { opt.reset(); }
		}

		/**************************************************************************
		 * @brief
		 * Write std::pair<A,B> as first then second.
		 * @tparam A
		 * First type.
		 * @tparam B
		 * Second type.
		 * @param os
		 * Output stream.
		 * @param p
		 * Pair to write.
		 **************************************************************************/
		template <class A, class B>
		static void write_any(std::ofstream &os, const std::pair<A, B> &p)
		{
			write_any(os, p.first);
			write_any(os, p.second);
		}

		/**************************************************************************
		 * @brief
		 * Read std::pair<A,B> as first then second.
		 * @tparam A
		 * First type.
		 * @tparam B
		 * Second type.
		 * @param is
		 * Input stream.
		 * @param p
		 * Destination pair.
		 **************************************************************************/
		template <class A, class B>
		static void read_any(std::ifstream &is, std::pair<A, B> &p)
		{
			read_any(is, p.first);
			read_any(is, p.second);
		}

		/**************************************************************************
		 * @brief
		 * Write std::map<K,V> as size then key/value pairs (in key order).
		 * @tparam K,V,Cmp,Alloc
		 * Map template parameters.
		 * @param os
		 * Output stream.
		 * @param m
		 * Map to write.
		 **************************************************************************/
		template <class K, class V, class Cmp, class Alloc>
		static void write_container(std::ofstream &os, const std::map<K, V, Cmp, Alloc> &m)
		{
			write_u64(os, m.size());
			for (auto const &kv : m) { write_any(os, kv.first); write_any(os, kv.second); }
		}

		/**************************************************************************
		 * @brief
		 * Read std::map<K,V> as size then key/value pairs (emplace in order).
		 * @tparam K,V,Cmp,Alloc
		 * Map template parameters.
		 * @param is
		 * Input stream.
		 * @param m
		 * Destination map (cleared then filled).
		 **************************************************************************/
		template <class K, class V, class Cmp, class Alloc>
		static void read_container(std::ifstream &is, std::map<K, V, Cmp, Alloc> &m)
		{
			const auto n = static_cast<std::size_t>(read_u64(is));
			m.clear();
			for (std::size_t i = 0; i < n; ++i)
			{
				K k{}; V v{};
				read_any(is, k); read_any(is, v);
				m.emplace(std::move(k), std::move(v));
			}
		}

		/**************************************************************************
		 * @brief
		 * Write std::unordered_map<K,V> as size then key/value pairs.
		 * @tparam K,V,Hash,Eq,Alloc
		 * Unordered map template parameters.
		 * @param os
		 * Output stream.
		 * @param m
		 * Unordered map to write (iteration order is not stable across builds).
		 **************************************************************************/
		template <class K, class V, class Hash, class Eq, class Alloc>
		static void write_container(std::ofstream &os, const std::unordered_map<K, V, Hash, Eq, Alloc> &m)
		{
			write_u64(os, m.size());
			for (auto const &kv : m) { write_any(os, kv.first); write_any(os, kv.second); }
		}

		/**************************************************************************
		 * @brief
		 * Write std::unordered_map<K,V> as size then key/value pairs.
		 * @tparam K,V,Hash,Eq,Alloc
		 * Unordered map template parameters.
		 * @param os
		 * Output stream.
		 * @param m
		 * Unordered map to write (iteration order is not stable across builds).
		 **************************************************************************/
		template <class K, class V, class Hash, class Eq, class Alloc>
		static void read_container(std::ifstream &is, std::unordered_map<K, V, Hash, Eq, Alloc> &m)
		{
			const auto n = static_cast<std::size_t>(read_u64(is));
			m.clear();
			m.reserve(n); // helps performance
			for (std::size_t i = 0; i < n; ++i)
			{
				K k{}; V v{};
				read_any(is, k); read_any(is, v);
				m.emplace(std::move(k), std::move(v));
			}
		}

		/**************************************************************************
		 * @brief
		 * Write std::vector<T> (non-trivial T) as size then per-element recurse.
		 * @tparam T,Alloc
		 * Vector template parameters.
		 * @param os
		 * Output stream.
		 * @param v
		 * Vector to write.
		 **************************************************************************/
		template <class T, class Alloc>
		static std::enable_if_t<!std::is_trivially_copyable_v<T>>
			write_container(std::ofstream &os, const std::vector<T, Alloc> &v)
		{
			write_u64(os, v.size());
			for (auto const &e : v) write_any(os, e);
		}

		/**************************************************************************
		 * @brief
		 * Read std::vector<T> (non-trivial T) as size then per-element recurse.
		 * @tparam T,Alloc
		 * Vector template parameters.
		 * @param is
		 * Input stream.
		 * @param v
		 * Destination vector (cleared, reserved, then emplaced).
		 **************************************************************************/
		template <class T, class Alloc>
		static std::enable_if_t<!std::is_trivially_copyable_v<T>>
			read_container(std::ifstream &is, std::vector<T, Alloc> &v)
		{
			const auto n = static_cast<std::size_t>(read_u64(is));
			v.clear();
			v.reserve(n);
			for (std::size_t i = 0; i < n; ++i)
			{
				T tmp{};
				read_any(is, tmp);
				v.emplace_back(std::move(tmp));
			}
		}

		/**************************************************************************
		 * @brief
		 * Write a reflected object by iterating its member-pointer tuple.
		 * @tparam T
		 * Reflected user type.
		 * @param os
		 * Output stream.
		 * @param obj
		 * Object to write.
		 * @details
		 * Fields are serialized in the exact order returned by reflect<T>::fields().
		 **************************************************************************/
		template <typename T>
		static void write_reflected(std::ofstream &os, const T &obj)
		{
			auto fptrs = reflect<T>::fields();
			tuple_for_each(fptrs, [&](auto memptr)
				{
					const auto &field = obj.*memptr;
					write_any(os, field);
				});
		}

		/**************************************************************************
		 * @brief
		 * Read a reflected object by iterating its member-pointer tuple.
		 * @tparam T
		 * Reflected user type.
		 * @param is
		 * Input stream.
		 * @param obj
		 * Object to populate.
		 * @details
		 * Fields are deserialized in the exact order returned by reflect<T>::fields().
		 **************************************************************************/
		template <typename T>
		static void read_reflected(std::ifstream &is, T &obj)
		{
			auto fptrs = reflect<T>::fields();
			tuple_for_each(fptrs, [&](auto memptr)
				{
					auto &field = obj.*memptr;
					read_any(is, field);
				});
		}
	};
} // namespace gam300

#endif // __SERIALISATIONBIN_MANAGER_H__
