#pragma once
#ifndef __SERIALISATION_MANAGER_H__
#define __SERIALISATION_MANAGER_H__

#include <cstdint>
#include <fstream>
#include <tuple>
#include <type_traits>
#include <vector>
#include <string>

#include "ECSManager.h"

namespace gam300
{

	/*!***********************************************************************
	\brief
	Binary (de)serialization helpers for PODs, contiguous containers
	(std::string/std::vector), and user types exposing `as_tuple()`.
	*************************************************************************/
	class SerializerBin
	{
	public:
		void writeScene(ECSManager &ecs, char const *fp)
		{
			std::ofstream os(fp, std::ios::binary);
			if (!os) return;

			auto &ents{ ecs.getAllEntities() };
			if (ents.empty())
				return;

			// Write header of bin file
			write_header(os);

			// Write each entity
			for (auto &e : ents)
			{
				writeEntity(os, ecs, e);
			}
		}

		void writeEntity(std::ofstream &os, ECSManager &ecs, Entity const &e)
		{
			// --- entity header "GAM300EN" ---
			Header h{};
			h.magic[0] = 'G'; h.magic[1] = 'A'; h.magic[2] = 'M'; h.magic[3] = '3';
			h.magic[4] = '0'; h.magic[5] = '0'; h.magic[6] = 'E'; h.magic[7] = 'N';
			os.write(reinterpret_cast<const char *>(&h), sizeof(h));

			const EntityID id = e.get_id();
			write_any(os, id);

			const auto mask = e.get_component_mask(); 

			std::uint32_t present = 0u;

			write_any(os, present);

		}


		void writeComponent(std::ofstream &os, Component const &e)
		{
			// Write component header
			Header h{};
			h.magic[0] = 'G'; h.magic[1] = 'A'; h.magic[2] = 'M'; h.magic[3] = '3';
			h.magic[4] = '0'; h.magic[5] = '0'; h.magic[6] = 'C'; h.magic[7] = 'P';
			os.write(reinterpret_cast<const char *>(&h), sizeof(h));

			// Write component as_tuple

		}

	private:
		struct Header
		{
			char     magic[8];   // "GAM300SC" "GAM300EN" "GAM300CP"
		};

		inline void write_header(std::ofstream &os)
		{
			Header h{};
			h.magic[0] = 'G'; h.magic[1] = 'A'; h.magic[2] = 'M'; h.magic[3] = '3';
			h.magic[4] = '0'; h.magic[5] = '0'; h.magic[6] = 'S'; h.magic[7] = 'C';
			os.write(reinterpret_cast<const char *>(&h), sizeof(h));
		}
		inline bool read_header(std::ifstream &is)
		{
			Header h{};
			is.read(reinterpret_cast<char *>(&h), sizeof(h));
			const bool magic_ok =
				h.magic[0] == 'G' && h.magic[1] == 'A' && h.magic[2] == 'M' && h.magic[3] == '3' &&
				h.magic[4] == '0' && h.magic[5] == '0' && h.magic[6] == 'S' && h.magic[7] == 'C';
			return magic_ok;
		}

		//===================== Core dispatch =====================
		/*!***********************************************************************
		\brief
		Generic dispatcher for writing a value to a binary stream.
		Routes to:
			- write_container() if T is contiguous+resizable (string/vector),
			- write_data()      if T is trivially copyable,
			- write_tuple() otherwise (field-by-field via as_tuple()).
		*************************************************************************/
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
			else
			{
				write_tuple(os, v);
			}
		}

		/*!***********************************************************************
		\brief
		Generic dispatcher for reading a value from a binary stream.
		Routes to:
			- read_container() if T is contiguous+resizable (string/vector),
			- read_data()      if T is trivially copyable,
			- read_tuple() otherwise (field-by-field via as_tuple()).
		*************************************************************************/
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
			else
			{
				read_tuple(is, v);
			}
		}

		//===================== u64 size I/O =====================

		/*!***********************************************************************
		\brief
		Writes an integer value to the output stream as a 64-bit unsigned integer.
		*************************************************************************/
		template <typename T>
		static inline void write_u64(std::ofstream &os, T n)
		{
			static_assert(std::is_integral_v<T>);
			std::uint64_t v = static_cast<std::uint64_t>(n);
			os.write(reinterpret_cast<const char *>(&v), sizeof(v));
		}

		/*!***********************************************************************
		\brief
		Reads a 64-bit unsigned integer value from the input stream.
		*************************************************************************/
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

		template <class Tuple, class F>
		static inline void tuple_for_each(Tuple &&tp, F &&f)
		{
			std::apply(
				[&](auto&&... elems)
				{
					(f(std::forward<decltype(elems)>(elems)), ...);
				},
				std::forward<Tuple>(tp)
			);
		}

		//===================== data (POD) =====================

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

		//===================== containers (contiguous, resizable) =====================

		// Trivial element -> bulk size + data
		template <class C>
		static std::enable_if_t<is_contig_resizable_v<C> &&std::is_trivially_copyable_v<typename C::value_type>>
			write_container(std::ofstream &os, const C &c)
		{
			write_u64(os, c.size());
			if (!c.empty())
			{
				os.write(reinterpret_cast<const char *>(c.data()),
					sizeof(typename C::value_type) * c.size());
			}
		}

		template <class C>
		static std::enable_if_t<is_contig_resizable_v<C> &&std::is_trivially_copyable_v<typename C::value_type>>
			read_container(std::ifstream &is, C &c)
		{
			const auto n = static_cast<std::size_t>(read_u64(is));
			c.resize(n);
			if (n)
			{
				is.read(reinterpret_cast<char *>(c.data()),
					sizeof(typename C::value_type) * n);
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

		//===================== components (as_tuple) =====================

		template <class T>
		static void write_tuple(std::ofstream &os, const T &obj)
		{
			auto tup = obj.as_tuple(); // tuple of (const) refs
			tuple_for_each(tup, [&](auto const &f)
				{
					write_any(os, f);
				});
		}

		template <class T>
		static void read_tuple(std::ifstream &is, T &obj)
		{
			auto tup = obj.as_tuple(); // tuple of refs
			tuple_for_each(tup, [&](auto &f)
				{
					read_any(is, f);
				});
		}
	};

} // end of namespace gam300
#endif // __SERIALISATION_MANAGER_H__