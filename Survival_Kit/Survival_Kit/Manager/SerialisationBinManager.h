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

namespace gam300 {

	/*!***********************************************************************
	\brief
	Binary (de)serialization helpers for PODs, contiguous containers
	(std::string/std::vector), and user types exposing `as_tuple()`.
	*************************************************************************/
	class SerializerBin {
	public:
		/*!***********************************************************************
		\brief
		Writes the entire scene (all entities and their components) to a binary
		file with a top-level "GAM300SC" header, followed by a sequence of
		"GAM300EN" entity chunks (each containing "GAM300CP" component records).

		\param ecs
		Reference to the ECSManager providing entity/component access.

		\param fp
		Null-terminated file path to write to.

		\details
		Does nothing if the file cannot be opened or if there are no entities.
		Each entity is emitted by calling writeEntity().
		*************************************************************************/
		void writeScene(ECSManager &ecs, char const *fp) {
			std::ofstream os(fp, std::ios::binary);
			if(!os) return;

			auto &ents{ ecs.getAllEntities() };
			if(ents.empty())
				return;

			Header h{};
			h.magic[0] = 'G'; h.magic[1] = 'A'; h.magic[2] = 'M'; h.magic[3] = '3';
			h.magic[4] = '0'; h.magic[5] = '0'; h.magic[6] = 'S'; h.magic[7] = 'C';
			os.write(reinterpret_cast<const char *>(&h), sizeof(h));

			// Write each entity
			for(auto &e : ents) {
				writeEntity(os, ecs, e);
			}
		}

		/*!***********************************************************************
		\brief
		Reads a scene from a binary file previously written by writeScene().

		\param ecs
		Reference to the ECSManager that will receive loaded entities/components.

		\param fp
		Null-terminated file path to read from.

		\details
		Validates the top-level "GAM300SC" header. Then repeatedly probes for
		"GAM300EN" chunks and delegates each to readEntity(). Stops on EOF or
		on encountering a non-entity chunk. No exceptions are thrown; returns
		silently on format errors or I/O failures.
		*************************************************************************/
		void readScene(ECSManager &ecs, char const *fp) {
			std::ifstream is(fp, std::ios::binary);
			if(!is) return;

			Header h{};
			if(!is.read(reinterpret_cast<char *>(&h), sizeof(h))) return;
			if(!(h.magic[0] == 'G' && h.magic[1] == 'A' && h.magic[2] == 'M' && h.magic[3] == '3' &&
			   h.magic[4] == '0' && h.magic[5] == '0' && h.magic[6] == 'S' && h.magic[7] == 'C')) {
				return; // bad file
			}

			// stream entities until EOF or non-EN chunk
			for(;;) {
				std::streampos pos = is.tellg();
				Header next{};
				if(!is.read(reinterpret_cast<char *>(&next), sizeof(next))) break; // EOF

				const bool is_entity =
					(next.magic[0] == 'G' && next.magic[1] == 'A' && next.magic[2] == 'M' && next.magic[3] == '3' &&
					 next.magic[4] == '0' && next.magic[5] == '0' && next.magic[6] == 'E' && next.magic[7] == 'N');

				if(!is_entity) {
					is.seekg(pos); break;
				}
				is.seekg(pos); // let readEntity consume the header
				readEntity(is, ecs);
			}
		}

		/*!***********************************************************************
		\brief
		Serializes a single entity and all of its present components.

		\param os
		Output binary stream (must be open).

		\param ecs
		Reference to the ECSManager for component retrieval.

		\param e
		Entity to serialize.

		\details
		Emits:
			1) "GAM300EN" entity header,
			2) the entity's ID,
			3) zero or more component records, each as:
			 - "GAM300CP" header,
			 - component type ID (uint32),
			 - payload size (uint64),
			 - payload bytes written by writeComponent().

		Only components whose mask bits are set and that can be fetched from
		the ECS are written. The size field is backpatched after payload write.
		*************************************************************************/
		void writeEntity(std::ofstream &os, ECSManager &ecs, Entity const &e) {
			// --- entity header "GAM300EN" ---
			Header h{}; h.magic[0] = 'G'; h.magic[1] = 'A'; h.magic[2] = 'M'; h.magic[3] = '3';
			h.magic[4] = '0'; h.magic[5] = '0'; h.magic[6] = 'E'; h.magic[7] = 'N';
			os.write(reinterpret_cast<const char *>(&h), sizeof(h));

			// entity id first
			write_any(os, e.get_id());

			const auto mask = e.get_component_mask();
			for(size_t cid{}; cid < MAX_COMPONENTS; ++cid) {
				if(!mask.test(cid)) continue;

				if(auto const *comp = ecs.getComponentByMask(static_cast<ComponentTypeID>(cid), e.get_id())) {
					// --- component header "GAM300CP" ---
					Header hc{}; hc.magic[0] = 'G'; hc.magic[1] = 'A'; hc.magic[2] = 'M'; hc.magic[3] = '3';
					hc.magic[4] = '0'; hc.magic[5] = '0'; hc.magic[6] = 'C'; hc.magic[7] = 'P';
					os.write(reinterpret_cast<const char *>(&hc), sizeof(hc));

					// comp_id
					std::uint32_t type_id32 = static_cast<std::uint32_t>(cid);
					write_any(os, type_id32);

					// size placeholder
					auto size_pos = os.tellp();
					write_u64(os, 0ull);

					// payload only
					writeComponent(os, *comp);

					// backpatch size
					auto end_pos = os.tellp();
					std::uint64_t payload_size = static_cast<std::uint64_t>(end_pos - size_pos) - sizeof(std::uint64_t);
					os.seekp(size_pos); write_u64(os, payload_size); os.seekp(end_pos);
				}
			}
		}

		/*!***********************************************************************
		\brief
		Deserializes a single entity and its component records from the stream.

		\param is
		Input binary stream positioned at an entity header.

		\param ecs
		Reference to the ECSManager where components will be created/updated.

		\details
		Validates and consumes the "GAM300EN" header, then reads:
			- entity ID,
			- a sequence of "GAM300CP" component records:
				* comp_id (uint32),
				* comp_size (uint64),
				* payload forwarded to readComponent().

		Reading stops when the next chunk is not a component ("GAM300CP") or on
		EOF. If a reader under-consumes bytes, the function seeks forward to the
		end of the declared component payload to remain chunk-aligned.
		*************************************************************************/
		void readEntity(std::ifstream &is, ECSManager &ecs) {
			// entity header "GAM300EN"
			Header h{};
			if(!is.read(reinterpret_cast<char *>(&h), sizeof(h))) return;
			if(!(h.magic[0] == 'G' && h.magic[1] == 'A' && h.magic[2] == 'M' && h.magic[3] == '3' &&
			   h.magic[4] == '0' && h.magic[5] == '0' && h.magic[6] == 'E' && h.magic[7] == 'N')) {
				return; // not an entity
			}

			// entity id
			EntityID eid{};
			read_any(is, eid);

			// components until next chunk isn't CP
			for(;;) {
				std::streampos hdr_pos = is.tellg();

				Header hc{};
				if(!is.read(reinterpret_cast<char *>(&hc), sizeof(hc))) break; // EOF => done with this entity

				const bool is_comp =
					(hc.magic[0] == 'G' && hc.magic[1] == 'A' && hc.magic[2] == 'M' && hc.magic[3] == '3' &&
					 hc.magic[4] == '0' && hc.magic[5] == '0' && hc.magic[6] == 'C' && hc.magic[7] == 'P');

				if(!is_comp) {
					// next chunk belongs to the next entity (or garbage). rewind and return.
					is.seekg(hdr_pos);
					break;
				}

				// comp id + size
				std::uint32_t comp_id{};
				read_any(is, comp_id);
				std::uint64_t comp_size = read_u64(is);

				// hand off to component reader
				readComponent(is, ecs, eid, comp_id, comp_size);

				// ensure we're positioned at the end of this component record even if a reader under-consumed
				std::streampos after_payload = is.tellg();
				std::streamoff consumed = after_payload - (hdr_pos + static_cast<std::streamoff>(sizeof(Header) + sizeof(std::uint32_t) + sizeof(std::uint64_t)));
				std::streamoff remain = static_cast<std::streamoff>(comp_size) - consumed;
				if(remain > 0) {
					is.seekg(remain, std::ios::cur);
				}
			}
		}

		/*!***********************************************************************
		\brief
		Writes a component payload (type-specific fields only) to the stream.

		\param os
		Output binary stream.

		\param c
		Component instance to serialize.

		\details
		This function is the type-dispatch point for component payloads.
		Implement by checking the dynamic type (or replacing with an ID->writer
		registry) and calling write_any(os, component) for types that expose
		as_tuple(), or writing fields directly. Should NOT write headers, IDs,
		or size—only the payload bytes for the component.
		*************************************************************************/
		void writeComponent(std::ofstream &os, Component const &c) {
			(void)os; (void)c;
			// TransformComponent
			//if (auto const* t = dynamic_cast<const TransformComponent*>(&c)) { write_any(os, *t); return; }
			// RenderComponent
			//if (auto const* r = dynamic_cast<const RenderComponent*>(&c)) { write_any(os, *r); return; }
			// PhysicsComponent
			//if (auto const* p = dynamic_cast<const PhysicsComponent*>(&c)) { write_any(os, *p); return; }
		}

		/*!***********************************************************************
		\brief
		Reads a component payload (type-specific fields) and attaches/applies it
		to the given entity.

		\param is
		Input binary stream positioned at the start of the component payload.

		\param ecs
		Reference to the ECSManager used to add/get the concrete component.

		\param eid
		Entity ID the component belongs to.

		\param comp_id
		Runtime component type identifier (matches what writeEntity() emitted).

		\param comp_size
		Declared size in bytes of this component’s payload.
		Readers should fully consume this many bytes; any short read is padded
		by the caller to keep the stream aligned.

		\details
		Dispatch on \p comp_id, ensure the component exists on \p eid (add if
		missing), then read its fields (preferably via read_any(is, *c) when the
		type implements as_tuple()). Unknown \p comp_id values are ignored.
		*************************************************************************/
		void readComponent(std::ifstream &is, ECSManager &ecs, EntityID eid,
						   std::uint32_t comp_id, std::uint64_t /*comp_size*/) {
			// Dispatch by comp_id. For each known type:
			// - ensure the component exists on the entity (add if missing)
			// - read payload with read_any(is, *c)  (T has as_tuple())

			switch(comp_id) {
				// case get_component_type_id<TransformComponent>(): {
				//     using T = TransformComponent;
				//     T* c = ecs.getComponent<T>(eid);
				//     if (!c) { ecs.addComponent<T>(eid); c = ecs.getComponent<T>(eid); }
				//     if (c) read_any(is, *c);
				// } break;

				// case get_component_type_id<RenderComponent>(): {
				//     using T = RenderComponent;
				//     T* c = ecs.getComponent<T>(eid);
				//     if (!c) { ecs.addComponent<T>(eid); c = ecs.getComponent<T>(eid); }
				//     if (c) read_any(is, *c);
				// } break;

				// case get_component_type_id<PhysicsComponent>(): {
				//     using T = PhysicsComponent;
				//     T* c = ecs.getComponent<T>(eid);
				//     if (!c) { ecs.addComponent<T>(eid); c = ecs.getComponent<T>(eid); }
				//     if (c) read_any(is, *c);
				// } break;

				default:
					break;
			}
		}

	private:
		struct Header {
			char     magic[8];   // "GAM300SC" "GAM300EN" "GAM300CP"
		};

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
		static void write_any(std::ofstream &os, const T &v) {
			if constexpr(is_contig_resizable_v<T>) {
				write_container(os, v);
			}
			else if constexpr(std::is_trivially_copyable_v<T>) {
				write_data(os, v);
			}
			else {
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
		static void read_any(std::ifstream &is, T &v) {
			if constexpr(is_contig_resizable_v<T>) {
				read_container(is, v);
			}
			else if constexpr(std::is_trivially_copyable_v<T>) {
				read_data(is, v);
			}
			else {
				read_tuple(is, v);
			}
		}

		//===================== u64 size I/O =====================

		/*!***********************************************************************
		\brief
		Writes an integer value to the output stream as a 64-bit unsigned integer.
		*************************************************************************/
		template <typename T>
		static inline void write_u64(std::ofstream &os, T n) {
			static_assert(std::is_integral_v<T>);
			std::uint64_t v = static_cast<std::uint64_t>(n);
			os.write(reinterpret_cast<const char *>(&v), sizeof(v));
		}

		/*!***********************************************************************
		\brief
		Reads a 64-bit unsigned integer value from the input stream.
		*************************************************************************/
		static inline std::uint64_t read_u64(std::ifstream &is) {
			std::uint64_t v{};
			is.read(reinterpret_cast<char *>(&v), sizeof(v));
			return v;
		}

		//===================== traits & tuple utils =====================

		// Detect "contiguous + resizable" containers (std::string, std::vector, etc.)
		template <class C>
		struct is_contig_resizable {
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
		static inline void tuple_for_each(Tuple &&tp, F &&f) {
			std::apply(
				[&](auto&&... elems) {
					(f(std::forward<decltype(elems)>(elems)), ...);
				},
				std::forward<Tuple>(tp)
			);
		}

		//===================== data (POD) =====================

		template <class T>
		static std::enable_if_t<std::is_trivially_copyable_v<T> && !is_contig_resizable_v<T>>
			write_data(std::ofstream &os, const T &value) {
			os.write(reinterpret_cast<const char *>(&value), sizeof(T));
		}

		template <class T>
		static std::enable_if_t<std::is_trivially_copyable_v<T> && !is_contig_resizable_v<T>>
			read_data(std::ifstream &is, T &value) {
			is.read(reinterpret_cast<char *>(&value), sizeof(T));
		}

		//===================== containers (contiguous, resizable) =====================

		// Trivial element -> bulk size + data
		template <class C>
		static std::enable_if_t<is_contig_resizable_v<C> &&std::is_trivially_copyable_v<typename C::value_type>>
			write_container(std::ofstream &os, const C &c) {
			write_u64(os, c.size());
			if(!c.empty()) {
				os.write(reinterpret_cast<const char *>(c.data()),
						 sizeof(typename C::value_type) * c.size());
			}
		}

		template <class C>
		static std::enable_if_t<is_contig_resizable_v<C> &&std::is_trivially_copyable_v<typename C::value_type>>
			read_container(std::ifstream &is, C &c) {
			const auto n = static_cast<std::size_t>(read_u64(is));
			c.resize(n);
			if(n) {
				is.read(reinterpret_cast<char *>(c.data()),
						sizeof(typename C::value_type) * n);
			}
		}

		// Non-trivial elements in std::vector -> element-wise recurse
		template <class T, class Alloc>
		static std::enable_if_t<!std::is_trivially_copyable_v<T>>
			write_container(std::ofstream &os, const std::vector<T, Alloc> &v) {
			write_u64(os, v.size());
			for(auto const &e : v) write_any(os, e);
		}

		template <class T, class Alloc>
		static std::enable_if_t<!std::is_trivially_copyable_v<T>>
			read_container(std::ifstream &is, std::vector<T, Alloc> &v) {
			const auto n = static_cast<std::size_t>(read_u64(is));
			v.clear();
			v.reserve(n);
			for(std::size_t i = 0; i < n; ++i) {
				T tmp{};
				read_any(is, tmp);
				v.emplace_back(std::move(tmp));
			}
		}

		//===================== components (as_tuple) =====================

		template <class T>
		static void write_tuple(std::ofstream &os, const T &obj) {
			auto tup = obj.as_tuple(); // tuple of (const) refs
			tuple_for_each(tup, [&](auto const &f) {
				write_any(os, f);
						   });
		}

		template <class T>
		static void read_tuple(std::ifstream &is, T &obj) {
			auto tup = obj.as_tuple(); // tuple of refs
			tuple_for_each(tup, [&](auto &f) {
				read_any(is, f);
						   });
		}
	};

} // end of namespace gam300
#endif // __SERIALISATION_MANAGER_H__