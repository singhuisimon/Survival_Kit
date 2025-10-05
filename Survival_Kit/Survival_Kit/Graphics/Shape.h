/**
 * @file Shape.h
 * @brief Procedural geometry generation utilities
 * @details Provides factory functions for creating common 3D primitive shapes
 *          and utilities for uploading mesh data to the GPU. All generated
 *          meshes include vertex positions, normals, and texture coordinates.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#ifndef __SHAPE_H__
#define __SHAPE_H__
#include "../Graphics/MeshData.h"

namespace gam300 {

	namespace Shape {

		/**
		 * @brief Generates a unit cube centered at the origin
		 * @return MeshData containing vertices, normals, and UVs for a cube
		 */
		MeshData make_cube();

		/**
		 * @brief Generates a flat plane on the XZ plane
		 * @return MeshData containing vertices, normals, and UVs for a plane
		 */
		MeshData make_plane();

		/**
		 * @brief Generates a UV sphere centered at the origin
		 * @return MeshData containing vertices, normals, and UVs for a sphere
		 */
		MeshData make_sphere();
	
		/**
		 * @brief Uploads CPU mesh data to GPU and creates OpenGL buffers
		 * @param mesh The mesh data to upload (VAO, VBO, EBO will be created)
		 * @return MeshGL object containing OpenGL handles and draw information
		 */
		MeshGL   upload_mesh_data(MeshData& mesh);
	}
}


#endif // !__SHAPE_H__
