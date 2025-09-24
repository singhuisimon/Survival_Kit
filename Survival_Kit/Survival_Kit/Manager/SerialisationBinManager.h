#pragma once
#ifndef __SERIALISATION_MANAGER_H__
#define __SERIALISATION_MANAGER_H__

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

namespace gam300
{
	//===================== Reflection core =====================

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

	// Helper macro for user types:
	//   struct Transform { float3 pos, rot, scale; };
	//   REFLECT_TYPE(Transform, &Transform::pos, &Transform::rot, &Transform::scale)
#define REFLECT_TYPE(TYPE, ...)                    \
	  template<> struct ::gam300::reflect<TYPE> {      \
	    static constexpr auto fields() {               \
	      return std::make_tuple(__VA_ARGS__);         \
	    }                                              \
	  }

	/*!***********************************************************************
	Binary (de)serialization for PODs, contiguous containers (string/vector),
	and user types declaring REFLECT_TYPE(T, &T::field, ...).
	*************************************************************************/
	class SerializerBin
	{
	public:
		//===================== File helpers =====================

		template <class T>
		static inline void save(const char *path, const T &obj)
		{
			std::ofstream os(path, std::ios::binary);
			write_any(os, obj);
		}

		template <class T>
		static inline void load(const char *path, T &obj)
		{
			std::ifstream is(path, std::ios::binary);
			read_any(is, obj);
		}

		//===================== Core dispatch =====================

		template <class T>
		static void write_any(std::ofstream &os, const T &v)
		{
			if constexpr (is_contig_resizable_v<T>)
			{
				write_container(os, v);
			}
			else if constexpr (std::is_trivially_copyable_v<T>)
			{
				write_data(os, v);
			}
			else if constexpr (is_reflected_v<T>)
			{
				write_reflected(os, v);
			}
			else
			{
				static_assert(is_reflected_v<T>,
					"Type is neither trivially copyable, contiguous container, nor reflected. "
					"Add REFLECT_TYPE(T, &T::field, ...) for this type.");
			}
		}

		template <class T>
		static void read_any(std::ifstream &is, T &v)
		{
			if constexpr (is_contig_resizable_v<T>)
			{
				read_container(is, v);
			}
			else if constexpr (std::is_trivially_copyable_v<T>)
			{
				read_data(is, v);
			}
			else if constexpr (is_reflected_v<T>)
			{
				read_reflected(is, v);
			}
			else
			{
				static_assert(is_reflected_v<T>,
					"Type is neither trivially copyable, contiguous container, nor reflected. "
					"Add REFLECT_TYPE(T, &T::field, ...) for this type.");
			}
		}

	private:
		//===================== u64 size I/O =====================

		template <typename T>
		static inline void write_u64(std::ofstream &os, T n)
		{
			static_assert(std::is_integral_v<T>);
			std::uint64_t v = static_cast<std::uint64_t>(n);
			os.write(reinterpret_cast<const char *>(&v), sizeof(v));
		}

		static inline std::uint64_t read_u64(std::ifstream &is)
		{
			std::uint64_t v{};
			is.read(reinterpret_cast<char *>(&v), sizeof(v));
			return v;
		}

		//===================== traits & tuple utils =====================

		// Detect "contiguous + resizable" containers (std::string, std::vector, etc.)
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

		//===================== POD (trivially copyable) =====================

		template <class T>
		static std::enable_if_t<std::is_trivially_copyable_v<T> && !is_contig_resizable_v<T>>
			write_data(std::ofstream &os, const T &value)
		{
			os.write(reinterpret_cast<const char *>(&value), sizeof(T));
		}

		template <class T>
		static std::enable_if_t<std::is_trivially_copyable_v<T> && !is_contig_resizable_v<T>>
			read_data(std::ifstream &is, T &value)
		{
			is.read(reinterpret_cast<char *>(&value), sizeof(T));
		}

		//===================== Containers (contiguous, resizable) =====================

		// Trivial element -> bulk size + data
		template <class C>
		static std::enable_if_t<is_contig_resizable_v<C> &&std::is_trivially_copyable_v<typename C::value_type>>
			write_container(std::ofstream &os, const C &c)
		{
			write_u64(os, c.size());
			if (!c.empty())
				os.write(reinterpret_cast<const char *>(c.data()),
					sizeof(typename C::value_type) * c.size());
		}

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

		// Fixed-size arrays: size is compile-time; stream elements one-by-one.
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

		template <class T>
		static void write_any(std::ofstream &os, const std::optional<T> &opt)
		{
			std::uint8_t has = opt.has_value() ? 1 : 0;
			os.write(reinterpret_cast<const char *>(&has), sizeof(has));
			if (has) write_any(os, *opt);
		}

		template <class T>
		static void read_any(std::ifstream &is, std::optional<T> &opt)
		{
			std::uint8_t has{};
			is.read(reinterpret_cast<char *>(&has), sizeof(has));
			if (has) { T tmp{}; read_any(is, tmp); opt = std::move(tmp); }
			else { opt.reset(); }
		}

		template <class A, class B>
		static void write_any(std::ofstream &os, const std::pair<A, B> &p)
		{
			write_any(os, p.first);
			write_any(os, p.second);
		}

		template <class A, class B>
		static void read_any(std::ifstream &is, std::pair<A, B> &p)
		{
			read_any(is, p.first);
			read_any(is, p.second);
		}

		// std::map
		template <class K, class V, class Cmp, class Alloc>
		static void write_container(std::ofstream &os, const std::map<K, V, Cmp, Alloc> &m)
		{
			write_u64(os, m.size());
			for (auto const &kv : m) { write_any(os, kv.first); write_any(os, kv.second); }
		}

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

		// std::unordered_map
		template <class K, class V, class Hash, class Eq, class Alloc>
		static void write_container(std::ofstream &os, const std::unordered_map<K, V, Hash, Eq, Alloc> &m)
		{
			write_u64(os, m.size());
			for (auto const &kv : m) { write_any(os, kv.first); write_any(os, kv.second); }
		}

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


		// Non-trivial elements in std::vector -> element-wise recurse
		template <class T, class Alloc>
		static std::enable_if_t<!std::is_trivially_copyable_v<T>>
			write_container(std::ofstream &os, const std::vector<T, Alloc> &v)
		{
			write_u64(os, v.size());
			for (auto const &e : v) write_any(os, e);
		}

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

		//===================== Reflected components (member-pointer list) =====================

		template <typename T>
		static void write_reflected(std::ofstream &os, const T &obj)
		{
			auto fptrs = reflect<T>::fields(); // tuple of pointers-to-members
			tuple_for_each(fptrs, [&](auto memptr)
				{
					const auto &field = obj.*memptr;
					write_any(os, field);
				});
		}

		template <typename T>
		static void read_reflected(std::ifstream &is, T &obj)
		{
			auto fptrs = reflect<T>::fields(); // tuple of pointers-to-members
			tuple_for_each(fptrs, [&](auto memptr)
				{
					auto &field = obj.*memptr;
					read_any(is, field);
				});
		}
	};

} // namespace gam300

#endif // __SERIALISATION_MANAGER_H__
