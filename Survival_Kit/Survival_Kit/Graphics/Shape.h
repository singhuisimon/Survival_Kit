#pragma once

#ifndef __SHAPE_H__
#define __SHAPE_H__
#include "../Graphics/MeshData.h"

namespace gam300 {

	namespace Shape {

		MeshData make_cube();
		MeshData make_plane();
	
		MeshGL   upload_mesh_data(MeshData& mesh);
	}
}


#endif // !__SHAPE_H__
