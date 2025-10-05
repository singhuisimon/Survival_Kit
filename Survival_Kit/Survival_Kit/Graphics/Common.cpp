/**
 * @file Common.cpp
 * @brief STB image library implementation file
 * @details Single translation unit that includes the STB image implementation.
 *          The STB_IMAGE_IMPLEMENTATION macro must be defined in exactly one
 *          .cpp file to generate the function definitions. This prevents
 *          multiple definition linker errors while allowing the header to be
 *          included elsewhere for declarations.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#define STB_IMAGE_IMPLEMENTATION
#include "../Graphics/stb_image.h"