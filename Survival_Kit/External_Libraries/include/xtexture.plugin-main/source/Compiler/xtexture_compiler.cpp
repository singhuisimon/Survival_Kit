#include "xtexture_compiler.h"
#include "dependencies/xbmp_tools/src/xbmp_tools.h"
#include "dependencies/crunch/inc/crnlib.h"
#include "../xtexture_rsc_descriptor.h"
#include "dependencies/xproperty/source/examples/xcore_sprop_serializer/xcore_sprop_serializer.h"
#include "Compressonator.h"
#include <iostream>
#include "half.h"
#include "dependencies/xmath/source/xmath_flinear.h"
#include "dependencies/xbitmap/source/bridges/xserializer/xbitmap_to_xserializer.h"

namespace crnlib
{
    extern std::uint32_t g_number_of_processors;
}

//---------------------------------------------------------------------------------------------

struct implementation final : xtexture_compiler::instance
{
    using state    = xresource_pipeline::state;
    using msg_type = xresource_pipeline::msg_type;

    xtexture_rsc::descriptor                        m_Descriptor;
    std::unordered_map<std::wstring, int>           m_BitmapHash;
    std::vector<xbitmap>                            m_Bitmaps;
    xbitmap                                         m_FinalBitmap;
    void*                                           m_pDDSData;
    bool                                            m_HasMixes;
    bool                                            m_bCubeMap;

    //---------------------------------------------------------------------------------------------
/*
    xcore::guid::rcfull<> getResourcePipelineFullGuid() const noexcept override
    {
        return xtexture_rsc::full_guid_v;
    }
*/
    //---------------------------------------------------------------------------------------------

    xerr onCompile(void) override
    {
        //
        // Read the descriptor file...
        //
        {
            xproperty::settings::context    Context{};
            auto                            DescriptorFileName = std::format(L"{}/{}/Descriptor.txt", m_ProjectPaths.m_Project, m_InputSrcDescriptorPath);

            if ( auto Err = m_Descriptor.Serialize(true, DescriptorFileName, Context); Err)
                return Err;
        }

        //
        // Do a quick validation of the descriptor
        //
        if (auto Err = DoValidation(); Err )
            return Err;

        //
        // Compile the textures
        //
        displayProgressBar("Processing ", 0.0f);
        DumpAllFileNamesIntoHash();
        displayProgressBar("Processing ", 0.2f);
        LoopThrowTheHashAndLoadImages();
        displayProgressBar("Processing ", 0.4f);
        if (m_HasMixes) CollapseMixes();
        displayProgressBar("Processing ", 0.6f);
        RunGenericFilters();
        displayProgressBar("Processing ", 1.0f);

        //
        // Now we are ready to compress and serialize our texture
        //
        if ( false && 
             m_Descriptor.m_Compression == xtexture_rsc::compression_format::RGBA_BC1_A1 
          && m_Descriptor.m_UsageType   == xtexture_rsc::usage_type::COLOR_AND_ALPHA )
        {
            // Turns out that Compressanator can not handle Color and Alpha compression well in this format
            // It makes the color black for the transparent pixels...
            // Seems like crunch has also this problem...
            LogMessage(msg_type::INFO, "using Crunch as the compression compiler" );
            UseCrunch();
        }
        else
        {
            LogMessage(msg_type::INFO, "using Compressonator as the compression compiler");
            UseCompressonatorHDRFriendlyFormat();
            UseCompressonator();
        }

        //
        // Serialize Final xBitmap
        //
        int Count = 0;
        for (auto& T : m_Target)
        {
            displayProgressBar("Serializing", Count++ / (float)m_Target.size() );

            if (T.m_bValid)
            {
                Serialize(T.m_DataPath);
            }
        }
        displayProgressBar("Serializing", 1);

        return {};
    }

    //---------------------------------------------------------------------------------------------

    xerr DoValidation() const
    {
        {
            std::vector<std::string> Errors;
            m_Descriptor.Validate(Errors);
            if (Errors.empty() == false)
            {
                for (auto& E : Errors)
                {
                    LogMessage( msg_type::ERROR, E );
                }

                return xerr::create_f<state, "The descriptor has validation errors">();
            }
        }

        if (m_Descriptor.m_UsageType == xtexture_rsc::usage_type::TANGENT_NORMAL && m_Descriptor.m_bSRGB)
        {
            LogMessage( msg_type::WARNING, "You have selected SRGB (Gamma) space, this will unnormalize the normals and create problems" );
        }

        if (m_Descriptor.m_UsageType == xtexture_rsc::usage_type::INTENSITY && m_Descriptor.m_bSRGB)
        {
            LogMessage(msg_type::WARNING, "You have selected SRGB (Gamma) space, for an intensity texture... This is unusual...");
        }

        return {};
    }

    //---------------------------------------------------------------------------------------------

    void RunGenericFilters()
    {
        //
        // If the user told us that he does not care about alpha let us make sure is set to 255
        //
        if (m_Descriptor.m_UsageType == xtexture_rsc::usage_type::COLOR ||
            m_Descriptor.m_UsageType == xtexture_rsc::usage_type::TANGENT_NORMAL)
        {
            for (auto& B : m_Bitmaps)
            {
                for (auto& E : B.getMip<xcolori>(0))
                {
                    E.m_A = 255;
                }
            }
        }

        //
        // If we are compressing base on BC1 force the alpha base on the threshold
        //
        if (false) // This should not be needed as the compiler already doing this
        if (m_Descriptor.m_Compression == xtexture_rsc::compression_format::RGBA_BC1_A1)
        {
            for (auto& B : m_Bitmaps)
            {
                xbmp::tools::filters::ForcePunchThroughAlpha(B, m_Descriptor.m_AlphaThreshold);
            }
        }

        //
        // If the user ask us to fill the average color to all the pixels that have alpha...
        // 
        if (m_Descriptor.m_UsageType == xtexture_rsc::usage_type::COLOR_AND_ALPHA && m_Descriptor.m_bFillAveColorByAlpha)
        {
            for (auto& B : m_Bitmaps)
            {
                // If we are doing debugging let us be obvious to what this filter is doing...
                if ( m_DebugType == debug_type::Dz || m_DebugType == debug_type::D1 )
                    xbmp::tools::filters::FillAvrColorBaseOnAlpha(B, m_Descriptor.m_AlphaThreshold );
                else
                    xbmp::tools::filters::FillAvrColorBaseOnAlpha(B, m_Descriptor.m_AlphaThreshold, 1);
            }
        }

        //
        // Make it tillable if requested by the user
        //
        if ( m_Descriptor.m_bTillableFilter)
        {
            if (m_Descriptor.m_UsageType == xtexture_rsc::usage_type::HDR_COLOR)
            {
                for (auto& B : m_Bitmaps)
                {
                    xbmp::tools::filters::MakeBitmapTilableHDR(B, m_Descriptor.m_TilableWidthPercentage, m_Descriptor.m_TilableHeightPercentage);
                }
            }
            else
            {
                for (auto& B : m_Bitmaps)
                {
                    xbmp::tools::filters::MakeBitmapTilable(B, m_Descriptor.m_TilableWidthPercentage, m_Descriptor.m_TilableHeightPercentage);
                }
            }
        }

        //
        // Prepare Normal Map Compressions
        //
        if (m_Descriptor.m_UsageType == xtexture_rsc::usage_type::TANGENT_NORMAL )
        {
            if (m_Descriptor.m_bNormalMapFlipY)
            {
                for (auto& B : m_Bitmaps)
                {
                    for (auto& E : B.getMip<xcolori>(0))
                    {
                        E.m_G = 255 - E.m_G;
                    }
                }
            }

            if (m_Descriptor.m_bNormalizeNormals)
            {
                for (auto& B : m_Bitmaps)
                {
                    for (auto& E : B.getMip<xcolori>(0))
                    {
                        E.setupFromNormal(xmath::fvec3(E.getNormal()).NormalizeSafeCopy());
                    }
                }
            }

            if (m_Descriptor.m_Compression == xtexture_rsc::compression_format::RGBA_BC3_A8)
            {
                for (auto& B : m_Bitmaps)
                {
                    for (auto& E : B.getMip<xcolori>(0))
                    {
                        auto O = E;
                        E.m_R = 0xff;
                        E.m_G = O.m_G;
                        E.m_B = 0;
                        E.m_A = O.m_R;
                    }
                }
            }
            else if (m_Descriptor.m_Compression == xtexture_rsc::compression_format::RG_BC5)
            {
                for (auto& B : m_Bitmaps)
                {
                    for (auto& E : B.getMip<xcolori>(0))
                    {
                        auto O = E;
                        E.m_R = O.m_G;
                        E.m_G = O.m_R;
                        E.m_B = 0;
                        E.m_A = 0;
                    }
                }
            }
        }

        //
        // Simplify / collapse all the inputs into our final bitmap[0]
        //
        if ( m_Descriptor.m_bConvertToCubeMap && 
                  ( m_Descriptor.m_InputVariant.index() == static_cast<std::size_t>(xtexture_rsc::variant_enum::MIX_SOURCE) 
                 || m_Descriptor.m_InputVariant.index() == static_cast<std::size_t>(xtexture_rsc::variant_enum::SINGLE_INPUT) ))
         {
             if ( m_Descriptor.m_UsageType == xtexture_rsc::usage_type::HDR_COLOR )
             {
                 xbitmap Bitmap;
                 xbmp::tools::filters::ConvertToCubeMapHDR(Bitmap, m_Bitmaps[0], m_Descriptor.m_ToCubeMapFaceResolution, m_Descriptor.m_ToCubeMapUseBilinear);
                 m_Bitmaps[0] = std::move(Bitmap);

                 m_bCubeMap = true;
             }
             else 
             {
                 xbitmap Bitmap;
                 xbmp::tools::filters::ConvertToCubeMap(Bitmap, m_Bitmaps[0], m_Descriptor.m_ToCubeMapFaceResolution, m_Descriptor.m_ToCubeMapUseBilinear );
                 m_Bitmaps[0] = std::move(Bitmap);

                 m_bCubeMap = true;
             }
         }
        else if( m_Descriptor.m_InputVariant.index() == static_cast<std::size_t>(xtexture_rsc::variant_enum::CUBE_INPUT) 
              || m_Descriptor.m_InputVariant.index() == static_cast<std::size_t>(xtexture_rsc::variant_enum::CUBE_INPUT_MIX) )
        {
            assert(m_Bitmaps.size() == 6);

            xbitmap Bitmap;

            auto FaceSize  = m_Bitmaps[0].getFaceSize();
            auto FrameSize = FaceSize * 6;
            auto TotalSize = FrameSize*1 + sizeof(int);
            auto pData     = new std::byte[TotalSize];

            pData[0] = pData[1] = pData[2] = pData[3] = std::byte{0};

            Bitmap.setup
            ( m_Bitmaps[0].getWidth()
            , m_Bitmaps[0].getHeight()
            , m_Bitmaps[0].getFormat()
            , FaceSize
            , { pData, TotalSize }
            , true
            , 1
            , 1
            , true
            );

            // Copy all the cube faces to the destination bitmap
            for (int i = 0; i != 6; ++i)
            {
                const auto& B = m_Bitmaps[i];
                const auto  S = B.getMip<std::byte>(0);
                auto        D = Bitmap.getMip<std::byte>(0, i);
                std::memcpy(D.data(), S.data(), FaceSize);
            }

            // Free all the bitmaps...
            m_Bitmaps.clear();

            // Set out final input bitmap
            m_Bitmaps.push_back(std::move(Bitmap));

            // yes we are a cube map...
            m_bCubeMap = true;
        }

    }

    //---------------------------------------------------------------------------------------------

    void LoadImageByCompressonator( const std::wstring_view FilePath, xbitmap& Bitmap )
    {
        //
        // Function to convert CMP_MipSet to CMP_Texture
        //
        constexpr auto ConvertMipSetToTexture = [](const CMP_MipSet & mipSetIn, CMP_Texture & texture)
        {
            // Assuming the topmost mip level (level 0)
            auto mipLevel = mipSetIn.m_pMipLevelTable[0];

            texture.dwSize      = sizeof(CMP_Texture);
            texture.dwWidth     = mipSetIn.m_nWidth;
            texture.dwHeight    = mipSetIn.m_nHeight;
            texture.format      = mipSetIn.m_format;
            texture.dwDataSize  = mipLevel->m_dwLinearSize;
            texture.pData       = new CMP_BYTE[texture.dwDataSize];
            std::memcpy(texture.pData, mipLevel->m_pbData, texture.dwDataSize);
        };

        //
        // DecompressTexture
        //
        constexpr auto DecompressTexture = []( CMP_Texture& texture)
        {
            CMP_Texture decompressedTexture {};
            decompressedTexture.dwSize   = sizeof(CMP_Texture);
            decompressedTexture.format   = CMP_FORMAT_ARGB_8888;
            decompressedTexture.dwWidth  = texture.dwWidth;
            decompressedTexture.dwHeight = texture.dwHeight;
            decompressedTexture.dwDataSize = 4 * texture.dwWidth * texture.dwHeight;
            decompressedTexture.pData    = new CMP_BYTE[decompressedTexture.dwDataSize];

            if (auto result = CMP_ConvertTexture(&texture, &decompressedTexture, nullptr, nullptr); result != CMP_OK)
                return false;

            std::swap(texture, decompressedTexture);
            delete[] decompressedTexture.pData;
            return true;
        };

        //
        // load image file
        //
        CMP_MipSet MipSetIn {};
        if (auto result = CMP_LoadTexture(xstrtool::To(FilePath).c_str(), &MipSetIn); result != CMP_OK)
        {
            throw(std::runtime_error(std::format("Unable to load image file. [BROKEN_LINK] {}", xstrtool::To(FilePath) )));
        }

        //
        // Convert it to a freendly format for the compiler
        //
        CMP_Texture Texture {};
        ConvertMipSetToTexture(MipSetIn, Texture);
        if ( DecompressTexture(Texture) == false )
        {
            throw(std::runtime_error(std::format("Failed to the image unable to decompress the texture {}", xstrtool::To(FilePath) )));
        }

        //
        // Convert to xbitmap
        //
        Bitmap.CreateBitmap(Texture.dwWidth, Texture.dwHeight);
        std::memcpy(Bitmap.getMip<std::byte>(0).data(), Texture.pData, Texture.dwDataSize);

        //
        // Free the memory and call it a day...
        //
        delete[] Texture.pData;
        CMP_FreeMipSet(&MipSetIn);
    }

    //---------------------------------------------------------------------------------------------

    void LoadTexture( xbitmap& Bitmap, const std::wstring_view FilePath )
    {
        // let xbmp tools deal with the common formats
        if (   xstrtool::findI(FilePath, L".jpeg") != std::string::npos
            || xstrtool::findI(FilePath, L".jpg")  != std::string::npos
            || xstrtool::findI(FilePath, L".tga")  != std::string::npos
            || xstrtool::findI(FilePath, L".png")  != std::string::npos
            || xstrtool::findI(FilePath, L".bmp")  != std::string::npos
            || xstrtool::findI(FilePath, L".psd")  != std::string::npos
            || xstrtool::findI(FilePath, L".hdr")  != std::string::npos
            || xstrtool::findI(FilePath, L".exr")  != std::string::npos )
        {
            if ( m_Descriptor.m_UsageType == xtexture_rsc::usage_type::HDR_COLOR )
            {
                if (xstrtool::findI(FilePath, L".exr") != std::string::npos )
                {
                    if (auto Err = xbmp::tools::loader::LoadHDREXRImage(Bitmap, FilePath); Err)
                        throw(std::runtime_error(std::format("{}, [BROKEN_LINK] {}", Err.getMessage(), xstrtool::To(FilePath) )));
                }
                else 
                {
                    if (auto Err = xbmp::tools::loader::LoadHDRSTDImage(Bitmap, FilePath); Err)
                        throw(std::runtime_error(std::format("{}, [BROKEN_LINK] {}", Err.getMessage(), xstrtool::To(FilePath) )));
                }
            }
            else
            {
                if (xstrtool::findI(FilePath, L".exr") != std::string::npos )
                {
                    if (auto Err = xbmp::tools::loader::LoadEXRImage(Bitmap, FilePath); Err)
                        throw(std::runtime_error(std::format("{}, [BROKEN_LINK] {}", Err.getMessage(), xstrtool::To(FilePath) )));
                }
                else
                {
                    if (auto Err = xbmp::tools::loader::LoadSTDImage(Bitmap, FilePath); Err)
                        throw(std::runtime_error(std::format("{}, [BROKEN_LINK] {}", Err.getMessage(), xstrtool::To(FilePath) )));
                }
            }
        }
        else
        {
            // Let Compressonator deal with all other formats...
            LoadImageByCompressonator(FilePath, Bitmap);
        }
    }

    //---------------------------------------------------------------------------------------------

    void DumpAllFileNamesIntoHash()
    {
        auto AddTexture = [&]( std::wstring& Str)
        {
            //
            // Let first clean the path for the textures...
            //
            if ( auto i = xstrtool::findI(Str, L".lion_project"); i != std::string::npos )
            {
                Str = Str.substr(i + 14);
            }
            else if ( i = xstrtool::findI(Str, L".lion_library"); i != std::string::npos )
            {
                Str = Str.substr(i + 15);
            }
            
            //
            // Then we can add to the hash
            //
            if (auto P = m_BitmapHash.find(Str); P != m_BitmapHash.end())
            {
                LogMessage( msg_type::WARNING, std::format(L"You have duplicated file names ({})", Str ) );
            }
            else
            {
                m_BitmapHash[Str] = -1;

                // Add string to our dependency list...
                m_Dependencies.m_Assets.push_back(Str);
            }
        };

        std::visit( [&]<typename T>( T& Input )
        {
            if constexpr( std::is_same_v<T, xtexture_rsc::single_input> )
            {
                m_bCubeMap = false;
                m_HasMixes = false;
                AddTexture(Input.m_FileName);
            }
            else if constexpr (std::is_same_v<T, xtexture_rsc::single_input_array>)
            {
                m_bCubeMap = false;
                m_HasMixes = false;
                for( auto& E : Input.m_FileNameList )
                {
                    AddTexture(E);
                }
            }
            else if constexpr (std::is_same_v<T, xtexture_rsc::mix_source>)
            {
                m_bCubeMap = false;
                m_HasMixes = true;
                for ( auto& E : Input.m_Inputs )
                {
                    AddTexture(E.m_FileName);
                }
            }
            else if constexpr (std::is_same_v<T, xtexture_rsc::mix_source_array>)
            {
                m_bCubeMap = false;
                m_HasMixes = true;
                
                for (auto& L : Input.m_MixSourceList)
                    for (auto& E : L.m_Inputs )
                    {
                        AddTexture(E.m_FileName);
                    }
            }
            else if constexpr (std::is_same_v<T, xtexture_rsc::cube_input>)
            {
                m_bCubeMap = true;
                m_HasMixes = false;
                AddTexture(Input.m_FileNameRight);
                AddTexture(Input.m_FileNameLeft);
                AddTexture(Input.m_FileNameUp);
                AddTexture(Input.m_FileNameDown);
                AddTexture(Input.m_FileNameForward);
                AddTexture(Input.m_FileNameBack);
            }
            else if constexpr (std::is_same_v<T, xtexture_rsc::cube_input_array>)
            {
                m_bCubeMap = true;
                m_HasMixes = false;
                for (auto& E : Input.m_CubeInputArray )
                {
                    AddTexture(E.m_FileNameRight);
                    AddTexture(E.m_FileNameLeft);
                    AddTexture(E.m_FileNameUp);
                    AddTexture(E.m_FileNameDown);
                    AddTexture(E.m_FileNameForward);
                    AddTexture(E.m_FileNameBack);
                }
            }
            else if constexpr (std::is_same_v<T, xtexture_rsc::cube_input_mix>)
            {
                m_bCubeMap = true;
                m_HasMixes = true;
                auto HandleMix = [&](xtexture_rsc::mix_source& MixSrc )
                {
                    for (auto& E : MixSrc.m_Inputs)
                        AddTexture(E.m_FileName);
                };

                HandleMix(Input.m_Right);
                HandleMix(Input.m_Left);
                HandleMix(Input.m_Up);
                HandleMix(Input.m_Down);
                HandleMix(Input.m_Forward);
                HandleMix(Input.m_Back);
            }
            else if constexpr (std::is_same_v<T, xtexture_rsc::cube_input_mix_array>)
            {
                m_bCubeMap = true;
                m_HasMixes = true;
                auto HandleMix = [&](xtexture_rsc::mix_source& MixSrc )
                {
                    for (auto& E : MixSrc.m_Inputs)
                        AddTexture(E.m_FileName);
                };

                for( auto& E : Input.m_CubeMixArray )
                {
                    HandleMix(E.m_Right);
                    HandleMix(E.m_Left);
                    HandleMix(E.m_Up);
                    HandleMix(E.m_Down);
                    HandleMix(E.m_Forward);
                    HandleMix(E.m_Back);
                }
            }
            else
            {
                assert(false);
            }

        }, m_Descriptor.m_InputVariant );
    }

    //---------------------------------------------------------------------------------------------

    void StandardizeBitmap( xbitmap& Bitmap )
    {
        //
        // Set all the wrapping properly
        //
        auto SetTheRightWrapMode = [&]()
        {
            static constexpr auto Table = []() consteval
            {
                std::array<xbitmap::wrap_mode, (int)xtexture_rsc::wrap_type::ENUM_COUNT > WrapTable = { xbitmap::wrap_mode::ENUM_COUNT };

                WrapTable[(int)xtexture_rsc::wrap_type::CLAMP_TO_EDGE]  = xbitmap::wrap_mode::CLAMP_TO_EDGE;
                WrapTable[(int)xtexture_rsc::wrap_type::WRAP]           = xbitmap::wrap_mode::WRAP;
                WrapTable[(int)xtexture_rsc::wrap_type::MIRROR]         = xbitmap::wrap_mode::MIRROR;

                return WrapTable;
            }();

            Bitmap.setUWrapMode( Table[(int)m_Descriptor.m_UWrap] );
            Bitmap.setVWrapMode( Table[(int)m_Descriptor.m_VWrap] );
        };

        //
        // Let us handle the trivial case...
        //
        if ( m_Descriptor.m_UsageType == xtexture_rsc::usage_type::HDR_COLOR )
        {
            if (Bitmap.getFormat() == xbitmap::format::R32G32B32A32_FLOAT)
            {
                // Make sure it has the right wrap mode
                SetTheRightWrapMode();
                return;
            }
        }
        else
        {
            if (Bitmap.getFormat() == xbitmap::format::R8G8B8A8)
            {
                // Make sure it has the right wrap mode
                SetTheRightWrapMode();
                return;
            }
        }

        //
        // Handle official conversions...
        //
        if (m_Descriptor.m_UsageType == xtexture_rsc::usage_type::HDR_COLOR)
        {
            if( Bitmap.getFormat() != xbitmap::format::R8G8B8 
             && Bitmap.getFormat() != xbitmap::format::R5G6B5
             && Bitmap.getFormat() != xbitmap::format::R32G32B32_FLOAT)
                throw(std::runtime_error("Source texture has a strange format"));

            const auto  nPixels      = Bitmap.getHeight() * Bitmap.getWidth();
            const auto  FaceSize     = nPixels * 4;
            const auto  DataSize     = 1 + FaceSize;
            auto        Data         = std::make_unique<float[]>(DataSize);
            auto*       pData        = &Data[1];

            Data[0] = 0;

            if ( Bitmap.getFormat() == xbitmap::format::R32G32B32_FLOAT )
            {
                const auto* pBitmapData  = Bitmap.getMip<float>(0).data();

                // Copy all the pixels over
                for( auto i=0u; i<nPixels; ++i)
                {
                    // R
                    *pData = *pBitmapData;
                    pData++; pBitmapData++;

                    // G
                    *pData = *pBitmapData;
                    pData++; pBitmapData++;

                    // B
                    *pData = *pBitmapData;
                    pData++; pBitmapData++;

                    // A
                    *pData = 1;
                    pData++;
                }
            }
            else
            {
                //
                // Integer conversions to float
                //
                const auto          ColorFmt        = xcolor::format{ static_cast<xcolor::format::type>(Bitmap.getFormat()) };
                const auto&         Descriptor      = ColorFmt.getDescriptor();
                const auto          BytesPerPixel   = Descriptor.m_TB / 8;
                const std::byte*    pBitmapData     = Bitmap.getMip<std::byte>(0).data();

                for (auto i = 0u; i < nPixels; ++i)
                {
                    const std::uint32_t D       = *reinterpret_cast<const std::uint32_t*>(pBitmapData);
                    const auto          Color   = xcolori{ D, ColorFmt };

                    *pData = static_cast<float>(Color.m_R) / 0xff;
                    pData++;

                    *pData = static_cast<float>(Color.m_G) / 0xff;
                    pData++;

                    *pData = static_cast<float>(Color.m_B) / 0xff;
                    pData++;

                    *pData = static_cast<float>(Color.m_A) / 0xff;
                    pData++;

                    pBitmapData += BytesPerPixel;
                }
            }

            //
            // Setup the bitmap
            //
            Bitmap.setup
            ( Bitmap.getWidth()
            , Bitmap.getHeight()
            , xbitmap::format::R32G32B32A32_FLOAT
            , sizeof(float) * FaceSize
            , { reinterpret_cast<std::byte*>(Data.release()), sizeof(float) * DataSize }
            , true
            , 1
            , 1
            );
        }
        else
        {
            if( Bitmap.getFormat() != xbitmap::format::R8G8B8 
             && Bitmap.getFormat() != xbitmap::format::R5G6B5 )
                throw(std::runtime_error("Source texture has a strange format"));

            const auto          nPixels         = Bitmap.getHeight() * Bitmap.getWidth();
            const auto          ColorFmt        = xcolor::format{ static_cast<xcolor::format::type>(Bitmap.getFormat()) };
            const auto&         Descriptor      = ColorFmt.getDescriptor();
            const auto          BytesPerPixel   = Descriptor.m_TB / 8;
            const std::byte*    pBitmapData     = Bitmap.getMip<std::byte>(0).data();
            auto                Data            = std::make_unique<xcolori[]>( 1 + nPixels);
            auto*               pData           = &Data[1];

            Data[0].m_Value = 0;

            for (auto i = 0u; i < nPixels; ++i)
            {
                const std::uint32_t D = *reinterpret_cast<const std::uint32_t*>(pBitmapData);
                *pData = xcolori{ D, ColorFmt };

                pData++;
                pBitmapData += BytesPerPixel;
            }   

            //
            // Setup the bitmap again
            //
            Bitmap.setup
            ( Bitmap.getWidth()
            , Bitmap.getHeight()
            , xbitmap::format::R8G8B8A8
            , sizeof(xcolori) * nPixels
            , { reinterpret_cast<std::byte*>(Data.release()), sizeof(xcolori) * (1 + nPixels) }
            , true
            , 1
            , 1
            );
        }

        //
        // Make sure it has the right wrap mode
        //
        SetTheRightWrapMode();
    }

    //---------------------------------------------------------------------------------------------

    void LoopThrowTheHashAndLoadImages()
    {
        m_Bitmaps.resize(m_BitmapHash.size());

        int Index  = 0;
        int Width  = 0;
        int Height = 0;
        for (auto& [FileName, BitmapIndex] : m_BitmapHash)
        {
            BitmapIndex = Index++;
            LoadTexture(m_Bitmaps[BitmapIndex], std::format(L"{}/{}", m_ProjectPaths.m_Project, FileName));

            if (BitmapIndex == 0 )
            {
                Width  = m_Bitmaps[BitmapIndex].getWidth();
                Height = m_Bitmaps[BitmapIndex].getHeight();

                if ( (Width%4) != 0 )
                {
                    LogMessage( msg_type::WARNING, std::format(L"Input Texture: [{}] Width is not a multiple of 4", FileName) );
                }

                if ( (Height%4)!= 0)
                {
                    LogMessage(msg_type::WARNING, std::format(L"Input Texture: [%s] Height is not a multiple of 4", FileName));
                }
            }
            else
            {
                if (Width != m_Bitmaps[BitmapIndex].getWidth() || Height != m_Bitmaps[BitmapIndex].getHeight())
                    throw std::runtime_error(std::format("Input Texture: [{}] All textures should be the same size", xstrtool::To(FileName) ));
            }

            //
            // Make sure to convert all textures to the same format... RGBA
            //
            StandardizeBitmap(m_Bitmaps[BitmapIndex]);
        }
    }

    //---------------------------------------------------------------------------------------------

    void CollapseMixes()
    {
        std::vector<xbitmap>  MixedBitmaps;

        auto HandleMix = [&](xbitmap& Dest, const xtexture_rsc::mix_source& MixSrc)
        {

            //
            // Prepare the destination bitmap
            //
            if (m_Descriptor.m_UsageType == xtexture_rsc::usage_type::HDR_COLOR)
            {
                auto faceSize = sizeof(xmath::fvec4) * m_Bitmaps[0].getWidth()* m_Bitmaps[0].getHeight();
                auto datasize  = faceSize + sizeof(std::uint32_t);
                auto pdata     = new std::byte[datasize];
                pdata[0] = pdata[1] = pdata[2] = pdata[3] = std::byte{0u};

                Dest.setup
                ( m_Bitmaps[0].getWidth()
                , m_Bitmaps[0].getHeight()
                , xbitmap::format::R32G32B32A32_FLOAT
                , faceSize
                , { pdata, datasize }
                , true
                , 1
                , 1
                );
            }
            else
            {
                Dest.CreateBitmap(m_Bitmaps[0].getWidth(), m_Bitmaps[0].getHeight());
            }
            
            //
            // Do the actual mixing...
            //
            auto Mixing = [&]<typename T_COLOR>(T_COLOR*)
            {
                for (auto& E : MixSrc.m_Inputs)
                {
                    xbitmap& Src = m_Bitmaps[ m_BitmapHash[E.m_FileName] ];

                    for (int y = 0, end_y = Src.getHeight(); y < end_y; ++y)
                    for (int x = 0, end_x = Src.getWidth();  x < end_x; ++x)
                    {
                              T_COLOR D = Dest.getMip<T_COLOR>(0)[y * Src.getWidth() + x];
                        const T_COLOR S = Src.getMip<T_COLOR>(0) [y * Src.getWidth() + x];

                        if (E.m_CopyFrom == xtexture_rsc::compositing::A)
                        {
                                 if (E.m_CopyTo == xtexture_rsc::compositing::R) D.m_R = S.m_A;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::G) D.m_G = S.m_A;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::B) D.m_B = S.m_A;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::A) D.m_A = S.m_A;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::RGB)  { D.m_R = S.m_A; D.m_G = S.m_A; D.m_B = S.m_A; }
                            else if (E.m_CopyTo == xtexture_rsc::compositing::RGBA) { D.m_R = S.m_A; D.m_G = S.m_A; D.m_B = S.m_A; D.m_A = S.m_A; }
                        }
                        else if (E.m_CopyFrom == xtexture_rsc::compositing::R)
                        {
                                 if (E.m_CopyTo == xtexture_rsc::compositing::R) D.m_R = S.m_R;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::G) D.m_G = S.m_R;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::B) D.m_B = S.m_R;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::A) D.m_A = S.m_R;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::RGB) {  D.m_R = S.m_R; D.m_G = S.m_R; D.m_B = S.m_R; }
                            else if (E.m_CopyTo == xtexture_rsc::compositing::RGBA) { D.m_R = S.m_R; D.m_G = S.m_R; D.m_B = S.m_R; D.m_A = S.m_R; }
                        }
                        else if (E.m_CopyFrom == xtexture_rsc::compositing::G)
                        {
                                 if (E.m_CopyTo == xtexture_rsc::compositing::R) D.m_R = S.m_G;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::G) D.m_G = S.m_G;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::B) D.m_B = S.m_G;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::A) D.m_A = S.m_G;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::RGB) { D.m_R = S.m_G; D.m_G = S.m_G; D.m_B = S.m_G; }
                            else if (E.m_CopyTo == xtexture_rsc::compositing::RGBA) { D.m_R = S.m_G; D.m_G = S.m_G; D.m_B = S.m_G; D.m_A = S.m_G; }
                        }
                        else if (E.m_CopyFrom == xtexture_rsc::compositing::B)
                        {
                                 if (E.m_CopyTo == xtexture_rsc::compositing::R) D.m_R = S.m_B;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::G) D.m_G = S.m_B;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::B) D.m_B = S.m_B;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::A) D.m_A = S.m_B;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::RGB)  { D.m_R = S.m_B; D.m_G = S.m_B; D.m_B = S.m_B; }
                            else if (E.m_CopyTo == xtexture_rsc::compositing::RGBA) { D.m_R = S.m_B; D.m_G = S.m_B; D.m_B = S.m_B; D.m_A = S.m_B; }
                        }
                        else if (E.m_CopyFrom == xtexture_rsc::compositing::RGBA)
                        {
                                 if (E.m_CopyTo == xtexture_rsc::compositing::RGB)  { D.m_R = S.m_R; D.m_G = S.m_G; D.m_B = S.m_B; }
                            else if (E.m_CopyTo == xtexture_rsc::compositing::RGBA) { D.m_R = S.m_R; D.m_G = S.m_G; D.m_B = S.m_B; D.m_A = S.m_A; }
                            else if (E.m_CopyTo == xtexture_rsc::compositing::R) D.m_R = S.m_R;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::G) D.m_G = S.m_G;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::B) D.m_B = S.m_B;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::A) D.m_A = S.m_A;
                            else assert(false);
                        }
                        else if (E.m_CopyFrom == xtexture_rsc::compositing::RGB)
                        {
                                 if (E.m_CopyTo == xtexture_rsc::compositing::RGBA) { D.m_R = S.m_R; D.m_G = S.m_G; D.m_B = S.m_B; D.m_A = 0xff; }
                            else if (E.m_CopyTo == xtexture_rsc::compositing::RGB)  { D.m_R = S.m_R; D.m_G = S.m_G; D.m_B = S.m_B; }
                            else if (E.m_CopyTo == xtexture_rsc::compositing::R) D.m_R = S.m_R;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::G) D.m_G = S.m_G;
                            else if (E.m_CopyTo == xtexture_rsc::compositing::B) D.m_B = S.m_B;
                            else throw(std::runtime_error("It does not have alpha information to copy from"));
                        }
                        else
                        {
                            assert(false);
                        }

                        //
                        // Set the destination pixel
                        //
                        Dest.getMip<T_COLOR>(0)[y * Src.getWidth() + x] = D;
                    }
                }
            };

           if( m_Descriptor.m_UsageType == xtexture_rsc::usage_type::HDR_COLOR) Mixing((xcolorf*)nullptr);
           else                                                                 Mixing((xcolori*)nullptr);
            
        };

        std::visit([&]<typename T>(T & Input)
        {
            if constexpr (std::is_same_v<T, xtexture_rsc::single_input>)
            {
                // Nothing to do...
            }
            else if constexpr (std::is_same_v<T, xtexture_rsc::single_input_array>)
            {
                // Nothing to do...
            }
            else if constexpr (std::is_same_v<T, xtexture_rsc::mix_source>)
            {
                MixedBitmaps.resize(1);
                HandleMix(MixedBitmaps[0], Input);
            }
            else if constexpr (std::is_same_v<T, xtexture_rsc::mix_source_array>)
            {
                MixedBitmaps.resize(Input.m_MixSourceList.size());
                int Index = 0;
                for (auto& A : Input.m_MixSourceList)
                    HandleMix(MixedBitmaps[Index++], A);
            }
            else if constexpr (std::is_same_v<T, xtexture_rsc::cube_input>)
            {
                // Nothing to do...
            }
            else if constexpr (std::is_same_v<T, xtexture_rsc::cube_input_array>)
            {
                // Nothing to do...
            }
            else if constexpr (std::is_same_v<T, xtexture_rsc::cube_input_mix>)
            {
                MixedBitmaps.resize(6);
                HandleMix(MixedBitmaps[0], Input.m_Right);
                HandleMix(MixedBitmaps[1], Input.m_Left);
                HandleMix(MixedBitmaps[2], Input.m_Up);
                HandleMix(MixedBitmaps[3], Input.m_Down);
                HandleMix(MixedBitmaps[4], Input.m_Forward);
                HandleMix(MixedBitmaps[5], Input.m_Back);
            }
            else if constexpr (std::is_same_v<T, xtexture_rsc::cube_input_mix_array>)
            {
                MixedBitmaps.resize(6 * Input.m_CubeMixArray.size() );
                int Index = 0;
                for (auto& E : Input.m_CubeMixArray)
                {
                    HandleMix(MixedBitmaps[Index++], E.m_Right);
                    HandleMix(MixedBitmaps[Index++], E.m_Left);
                    HandleMix(MixedBitmaps[Index++], E.m_Up);
                    HandleMix(MixedBitmaps[Index++], E.m_Down);
                    HandleMix(MixedBitmaps[Index++], E.m_Forward);
                    HandleMix(MixedBitmaps[Index++], E.m_Back);
                }
            }
            else
            {
                assert(false);
            }

        }, m_Descriptor.m_InputVariant);

        // We are overriding all the bitmaps with the final bitmaps
        m_Bitmaps = std::move(MixedBitmaps);
    }

    //---------------------------------------------------------------------------------------------

    void UseCrunch( void )
    {
        constexpr static auto TableConvertFormat = []() consteval ->auto
            {
                std::array< crn_format, static_cast<std::int32_t>(xtexture_rsc::compression_format::count_v) > Array = { crn_format::cCRNFmtInvalid };

                Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_BC1)]     = crn_format::cCRNFmtDXT1;
                Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC1_A1)] = crn_format::cCRNFmtDXT1;
                Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC3_A8)] = crn_format::cCRNFmtDXT3;

                return Array;
            }();

        // Crunch the image data and return a pointer to the crunched result array
        crn_comp_params Params;
        Params.clear();

        Params.m_dxt1a_alpha_threshold = m_Descriptor.m_AlphaThreshold;

        Params.m_alpha_component = ( m_Descriptor.m_Compression == xtexture_rsc::compression_format::RGBA_BC1_A1 
                                  || m_Descriptor.m_Compression == xtexture_rsc::compression_format::RGBA_BC3_A8)
                                   && m_Descriptor.m_UsageType  == xtexture_rsc::usage_type::COLOR_AND_ALPHA ? 3 : 0;
        Params.m_format          = TableConvertFormat[static_cast<std::int32_t>(m_Descriptor.m_Compression)];

        if ( Params.m_format == crn_format::cCRNFmtInvalid )
            throw(std::runtime_error("The compiler can not handle the specified compression format"));

        float Scalar;
             if (m_OptimizationType == xresource_pipeline::compiler::base::optimization_type::O0) Scalar = 0.0f;
        else if (m_OptimizationType == xresource_pipeline::compiler::base::optimization_type::O1) Scalar = m_Descriptor.m_Quality;
        else if (m_OptimizationType == xresource_pipeline::compiler::base::optimization_type::Oz) Scalar = 1.0f;
        else Scalar = m_Descriptor.m_Quality;

             if (Scalar == 0.0f)  Params.m_dxt_quality = crn_dxt_quality::cCRNDXTQualitySuperFast;
        else if (Scalar <= 0.3f)  Params.m_dxt_quality = crn_dxt_quality::cCRNDXTQualityFast;
        else if (Scalar <= 0.7f)  Params.m_dxt_quality = crn_dxt_quality::cCRNDXTQualityNormal;
        else if (Scalar <= 0.8f)  Params.m_dxt_quality = crn_dxt_quality::cCRNDXTQualityBetter;
        else if (Scalar <= 0.9f)  Params.m_dxt_quality = crn_dxt_quality::cCRNDXTQualityUber;
        else Params.m_dxt_quality = crn_dxt_quality::cCRNDXTQualityUber;


        Params.m_width                  = m_Bitmaps[0].getWidth();
        Params.m_height                 = m_Bitmaps[0].getHeight();
        Params.m_file_type              = crn_file_type::cCRNFileTypeDDS;
        Params.m_num_helper_threads     = std::min( static_cast<std::uint32_t>(cCRNMaxHelperThreads), Scalar >= 0.5f ? std::thread::hardware_concurrency() : std::thread::hardware_concurrency()/2 );
        crnlib::g_number_of_processors  = Params.m_num_helper_threads;


        Params.m_faces = static_cast<std::uint32_t>(m_Bitmaps.size());
        for (int i = 0; i < m_Bitmaps.size(); ++i)
        {
            Params.m_pImages[i][0] = m_Bitmaps[i].getMip<std::uint32_t>(0).data();
        }

        // If we are doing colors we can use perceptual compression otherwise we can not
        if (m_Descriptor.m_bSRGB )
            Params.m_flags |= crn_comp_flags::cCRNCompFlagPerceptual;
        else
            Params.m_flags &= ~crn_comp_flags::cCRNCompFlagPerceptual;

        if( m_Descriptor.m_UsageType == xtexture_rsc::usage_type::INTENSITY ) 
            Params.m_flags |= crn_comp_flags::cCRNCompFlagGrayscaleSampling;
        else 
            Params.m_flags &= ~crn_comp_flags::cCRNCompFlagGrayscaleSampling;

        if ( m_Descriptor.m_UsageType == xtexture_rsc::usage_type::COLOR_AND_ALPHA 
          && m_Descriptor.m_Compression == xtexture_rsc::compression_format::RGBA_BC1_A1 )
            Params.m_flags |= cCRNCompFlagDXT1AForTransparency;

             if (m_DebugType == debug_type::D0) Params.m_flags &= ~crn_comp_flags::cCRNCompFlagDebugging;
        else if (m_DebugType == debug_type::D1) Params.m_flags |= crn_comp_flags::cCRNCompFlagDebugging;
        else if (m_DebugType == debug_type::Dz) Params.m_flags |= crn_comp_flags::cCRNCompFlagDebugging;
        else Params.m_flags &= ~crn_comp_flags::cCRNCompFlagDebugging;

        // Check to make sure everything is OK
        if( Params.check() == false )
            throw(std::runtime_error("Parameters for the compressor (crunch) failed."));


        crn_uint32        CompressSize;
        crn_mipmap_params Mipmaps;
        Mipmaps.clear();

        // Set gamma filtering... 
        Mipmaps.m_gamma_filtering = m_Descriptor.m_bSRGB;

        //
        // Actual compression
        //
        {
            crn_uint32  Actual_quality_level;   // Print stats
            float       Actual_bitrate;

            Params.m_pProgress_func_data = this;
            Params.m_pProgress_func      = [](crn_uint32 phase_index, crn_uint32 total_phases, crn_uint32 subphase_index, crn_uint32 total_subphases, void* pUser_data_ptr) -> crn_bool
            {
                auto pThis = reinterpret_cast<implementation*>(pUser_data_ptr);
                auto i     = phase_index * total_subphases + subphase_index ;

                if ((i % 20) == 0 || (phase_index == (total_phases-1) && subphase_index == (total_subphases-1)))
                {
                    float total = static_cast<float>(total_phases * total_subphases);
                    pThis->displayProgressBar("Compression", i/total);
                }

                return true;
            };

            m_pDDSData = crn_compress
            ( Params
            , Mipmaps
            , CompressSize
            , &Actual_quality_level
            , &Actual_bitrate
            );

            if(m_pDDSData == nullptr )
                throw(std::runtime_error("The compressor (crunch) failed."));
        }

        //
        // Debug save the dds file
        //
        if (m_DebugType == debug_type::D1 || m_DebugType == debug_type::D1)
        {
            auto filename = std::format(L"{}\\FinalImage.dds", m_ResourceLogPath);

            // IF it suposed to have gamma make sure to convert it to sRGB
            FILE*   fp = nullptr;
            
            if (auto Err = _wfopen_s(&fp, filename.c_str(), L"wb"); Err != 0 || fp == nullptr )
            {
                std::array<wchar_t, 256> errMsg;
                if (_wcserror_s(errMsg.data(), errMsg.size(), Err) == 0)
                {
                    xerr::LogMessage<state::FAILURE>(std::format("Unable to save the dds: {}, {}", Err, xstrtool::To(errMsg.data())));
                }
                else
                {
                    xerr::LogMessage<state::FAILURE>(std::format("Unable to save the dds, system unkown error: {}", Err));
                }

                throw(std::runtime_error("Unable to save the dds..."));
            }

            // in the DDS file offset to dxgiFormat part of the DX10 header
            if (-1 == fseek(fp, 128, SEEK_SET))
                throw(std::runtime_error("Unable to save the Debug dds..."));

            if (1 != fwrite(m_pDDSData, CompressSize, 1, fp))
                throw(std::runtime_error("Unable to save the Debug dds..."));

            fclose(fp);
        }


        //
        // Convert from DDS format to xbitmap
        //
        if (auto Err = xbmp::tools::loader::LoadDSS(m_FinalBitmap, { reinterpret_cast<std::byte*>(m_pDDSData), CompressSize }); Err)
            throw(std::runtime_error(std::format("{}",Err.getMessage())) );
    }


    // https://stackoverflow.com/questions/76799117/how-to-convert-a-float-to-a-half-type-and-the-other-way-around-in-c
#include "immintrin.h"

    uint32_t float_as_uint32(float a)
    {
        uint32_t r;
        memcpy(&r, &a, sizeof r);
        return r;
    }

    uint16_t float2half_rn(float a)
    {
        uint32_t ia = float_as_uint32(a);
        uint16_t ir;

        ir = (ia >> 16) & 0x8000;
        if ((ia & 0x7f800000) == 0x7f800000) {
            if ((ia & 0x7fffffff) == 0x7f800000) {
                ir |= 0x7c00; /* infinity */
            }
            else {
                ir |= 0x7e00 | ((ia >> (24 - 11)) & 0x1ff); /* NaN, quietened */
            }
        }
        else if ((ia & 0x7f800000) >= 0x33000000) {
            int shift = (int)((ia >> 23) & 0xff) - 127;
            if (shift > 15) {
                ir |= 0x7c00; /* infinity */
            }
            else {
                ia = (ia & 0x007fffff) | 0x00800000; /* extract mantissa */
                if (shift < -14) { /* denormal */
                    ir |= ia >> (-1 - shift);
                    ia = ia << (32 - (-1 - shift));
                }
                else { /* normal */
                    ir |= ia >> (24 - 11);
                    ia = ia << (32 - (24 - 11));
                    ir = ir + ((14 + shift) << 10);
                }
                /* IEEE-754 round to nearest of even */
                if ((ia > 0x80000000) || ((ia == 0x80000000) && (ir & 1))) {
                    ir++;
                }
            }
        }
        return ir;
    }

    uint16_t float2half_rn_ref(float a)
    {
        __m128 pa = _mm_set_ps1(a);
        __m128i r16 = _mm_cvtps_ph(pa, _MM_FROUND_TO_NEAREST_INT);
        uint16_t res;
        memcpy(&res, &r16, sizeof res);
        return res;
    }

    float uint32_as_float(uint32_t a)
    {
        float r;
        memcpy(&r, &a, sizeof r);
        return r;
    }

    // Function to convert a float to an unsigned half-precision float (UF16)
    // UF16(unsigned float), 5 exponent bits + 11 mantissa bits
    uint16_t floatToUF16(float value)
    {
        // Interpret the float as an unsigned 32-bit integer
        uint32_t floatBits;
        std::memcpy(&floatBits, &value, sizeof(float));

        // Extract the sign, exponent, and mantissa from the float
        uint32_t sign     = (floatBits >> 31) & 0x1;
        uint32_t exponent = (floatBits >> 23) & 0xFF;
        uint32_t mantissa = floatBits & 0x7FFFFF;

        // Adjust the exponent from float to UF16
        int32_t newExponent = static_cast<int32_t>(exponent) - 127 + 15;

        // Handle special cases
        if (exponent == 0xFF) // NaN or Infinity
        {
            newExponent = 0x1F;
            if (mantissa != 0) // NaN
            {
                mantissa = 0x3FF;
            }
            else // Infinity
            {
                mantissa = 0;
            }
        }
        else if (newExponent <= 0) // Underflow to zero or denormalized number
        {
            if (newExponent < -10)
            {
                newExponent = 0;
                mantissa = 0;
            }
            else
            {
                mantissa = (mantissa | 0x800000) >> (1 - newExponent);
                newExponent = 0;
            }
        }
        else if (newExponent >= 0x1F) // Overflow to infinity
        {
            newExponent = 0x1F;
            mantissa = 0;
        }
        else // Normalized number
        {
            mantissa >>= 12;
        }

        // Combine the sign, exponent, and mantissa into the UF16 format
        uint16_t uf16 = ((newExponent&31) << 11) | (mantissa & 0x7FF);

        return uf16;
    }

    // UF16(unsigned float), 1 signed bit + 5 exponent bits + 10 mantissa bits
    uint16_t floatToSF16(float value)
    {
        // Interpret the float as an unsigned 32-bit integer
        uint32_t floatBits;
        std::memcpy(&floatBits, &value, sizeof(float));

        // Extract the sign, exponent, and mantissa from the float
        uint32_t sign = (floatBits >> 31) & 0x1;
        uint32_t exponent = (floatBits >> 23) & 0xFF;
        uint32_t mantissa = floatBits & 0x7FFFFF;

        // Adjust the exponent from float to SF16
        int32_t newExponent = static_cast<int32_t>(exponent) - 127 + 15;

        // Handle special cases
        if (exponent == 0xFF) // NaN or Infinity
        {
            newExponent = 0x1F;
            if (mantissa != 0) // NaN
            {
                mantissa = 0x3FF;
            }
            else // Infinity
            {
                mantissa = 0;
            }
        }
        else if (newExponent <= 0) // Underflow to zero or denormalized number
        {
            if (newExponent < -10)
            {
                newExponent = 0;
                mantissa = 0;
            }
            else
            {
                mantissa = (mantissa | 0x800000) >> (1 - newExponent);
                newExponent = 0;
            }
        }
        else if (newExponent >= 0x1F) // Overflow to infinity
        {
            newExponent = 0x1F;
            mantissa = 0;
        }
        else // Normalized number
        {
            mantissa >>= 13;
        }

        // Combine the sign, exponent, and mantissa into the SF16 format
        uint16_t sf16 = (sign << 15) | (newExponent << 10) | (mantissa & 0x3FF);

        return sf16;
    }

    //---------------------------------------------------------------------------------------------

    void UseCompressonatorHDRFriendlyFormat()
    {
        // If we are not dealing with HDR formats then we can skip this step
        if (m_Descriptor.m_UsageType != xtexture_rsc::usage_type::HDR_COLOR) return;

        xbitmap HDRHalfBitmap;

        auto FullColorDataSize = m_Bitmaps[0].getDataSize();
        auto HalfColorDataSize = (FullColorDataSize - sizeof(int))/2 + sizeof(int);
        auto FaceSize          = m_Bitmaps[0].getFaceSize()/2;
        auto pData             = new std::byte[HalfColorDataSize];

        pData[0] = pData[1] = pData[2] = pData[3] = std::byte{ 0u };

        HDRHalfBitmap.setup
        ( m_Bitmaps[0].getWidth()
        , m_Bitmaps[0].getHeight()
        , xbitmap::format::R16G16B16A16_FLOAT
        , FaceSize
        , { pData, HalfColorDataSize }
        , true
        , 1
        , m_Bitmaps[0].getFrameCount()
        , m_Bitmaps[0].isCubemap()
        );

        struct half_color
        {
            std::uint16_t m_R;
            std::uint16_t m_G;
            std::uint16_t m_B;
            std::uint16_t m_A;
        };

        for( auto iFrame = 0, FrameCount = HDRHalfBitmap.getFrameCount(); iFrame < FrameCount; ++iFrame)
        {
            for (auto iFace = 0, FaceCount = HDRHalfBitmap.getFaceCount(); iFace < FaceCount; ++iFace)
            {
                const auto SrcFullColor  = m_Bitmaps[0].getMip<xcolorf>(0, iFace, iFrame);
                      auto DestHalfColor = HDRHalfBitmap.getMip<half_color>(0, iFace, iFrame);
                for ( auto i=0u; i< SrcFullColor.size(); ++i )
                {
                    auto& D = DestHalfColor[i];
                    auto& S = SrcFullColor[i];

                    D.m_R = half(S.m_R).bits();
                    D.m_G = half(S.m_G).bits();
                    D.m_B = half(S.m_B).bits();
                    D.m_A = half(S.m_A).bits();
                }
            }
        }

        // Set the final bitmap
        HDRHalfBitmap.setUWrapMode( m_Bitmaps[0].getUWrapMode() );
        HDRHalfBitmap.setVWrapMode( m_Bitmaps[0].getVWrapMode() );
        m_Bitmaps[0] = std::move(HDRHalfBitmap);
    }

    //---------------------------------------------------------------------------------------------

    void UseCompressonator(void)
    {
        constexpr static auto TextureConversionTable = []() consteval ->auto
        {
            std::array< CMP_FORMAT, static_cast<std::int32_t>(xtexture_rsc::compression_format::count_v) > Array = { CMP_FORMAT::CMP_FORMAT_Unknown };

            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_BC1)]             = CMP_FORMAT::CMP_FORMAT_BC1;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC1_A1)]         = CMP_FORMAT::CMP_FORMAT_BC1;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC3_A8)]         = CMP_FORMAT::CMP_FORMAT_BC3;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::R_BC4)]               = CMP_FORMAT::CMP_FORMAT_BC4;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RG_BC5)]              = CMP_FORMAT::CMP_FORMAT_BC5;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_UHDR_BC6)]        = CMP_FORMAT::CMP_FORMAT_BC6H;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_SHDR_BC6)]        = CMP_FORMAT::CMP_FORMAT_BC6H_SF;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC7)]            = CMP_FORMAT::CMP_FORMAT_BC7;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_SUPER_COMPRESS)]  = CMP_FORMAT::CMP_FORMAT_BASIS;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_SUPER_COMPRESS)] = CMP_FORMAT::CMP_FORMAT_BASIS;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_UNCOMPRESSED)]   = CMP_FORMAT::CMP_FORMAT_RGBA_8888;

            return Array;
        }();

        constexpr static auto ChannelConversionTable = []() consteval ->auto
        {
            std::array< CMP_BYTE, static_cast<std::int32_t>(xtexture_rsc::compression_format::count_v) > Array = { 0 };

            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_BC1)]             = 3;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC1_A1)]         = 4;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC3_A8)]         = 4;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::R_BC4)]               = 1;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RG_BC5)]              = 2;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_UHDR_BC6)]        = 4;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_SHDR_BC6)]        = 4;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC7)]            = 4;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_SUPER_COMPRESS)]  = 3;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_SUPER_COMPRESS)] = 4;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_UNCOMPRESSED)]   = 4;

            return Array;
        }();

        constexpr static auto DataTypeConversionTable = []() consteval ->auto
        {
            std::array< CMP_TextureDataType, static_cast<std::int32_t>(xtexture_rsc::compression_format::count_v) > Array = { CMP_TextureDataType::TDT_ARGB };

            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_BC1)]             = CMP_TextureDataType::TDT_XRGB;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC1_A1)]         = CMP_TextureDataType::TDT_ARGB;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC3_A8)]         = CMP_TextureDataType::TDT_ARGB;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::R_BC4)]               = CMP_TextureDataType::TDT_XRGB;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RG_BC5)]              = CMP_TextureDataType::TDT_XRGB;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_UHDR_BC6)]        = CMP_TextureDataType::TDT_XRGB;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_SHDR_BC6)]        = CMP_TextureDataType::TDT_XRGB;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC7)]            = CMP_TextureDataType::TDT_XRGB;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_SUPER_COMPRESS)]  = CMP_TextureDataType::TDT_XRGB;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_SUPER_COMPRESS)] = CMP_TextureDataType::TDT_ARGB;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_UNCOMPRESSED)]   = CMP_TextureDataType::TDT_ARGB;

            return Array;
        }();

        //
        // Initialize the framework
        // 
        CMP_InitFramework();

        //
        // Setup the mip tables
        //
        constexpr auto max_mip_levels_v = 20 * 6;
        CMP_MipLevel** MipLevelTable = new CMP_MipLevel*[max_mip_levels_v] {};
        for (int i = 0; i < max_mip_levels_v; ++i) MipLevelTable[i] = new CMP_MipLevel();

        //
        // Set up our texture 
        //
        CMP_MipSet MipSet;
        memset(&MipSet, 0, sizeof(CMP_MipSet));

        MipSet.m_nWidth             = static_cast<CMP_INT>(m_Bitmaps[0].getWidth());
        MipSet.m_nHeight            = static_cast<CMP_INT>(m_Bitmaps[0].getHeight());
        MipSet.m_nDepth             = m_Bitmaps[0].isCubemap() ? 6 : 1;
        MipSet.m_format             = m_Descriptor.m_UsageType == xtexture_rsc::usage_type::HDR_COLOR ? CMP_FORMAT::CMP_FORMAT_RGBA_16F : CMP_FORMAT::CMP_FORMAT_ARGB_8888;
        MipSet.m_ChannelFormat      = m_Descriptor.m_UsageType == xtexture_rsc::usage_type::HDR_COLOR ? CF_Float16 : CF_8bit;
        MipSet.m_TextureDataType    = m_Descriptor.m_UsageType == xtexture_rsc::usage_type::TANGENT_NORMAL ? TextureDataType::TDT_NORMAL_MAP : DataTypeConversionTable[static_cast<std::int32_t>(m_Descriptor.m_Compression)];
        MipSet.m_TextureType        = m_Bitmaps[0].isCubemap() ? TT_CubeMap : TT_2D;
        MipSet.m_Flags              = 0;
        MipSet.m_CubeFaceMask       = m_Bitmaps[0].isCubemap() ? 0x3f : 0; // MS_CF_All
        MipSet.m_dwFourCC           = 0;
        MipSet.m_dwFourCC2          = 0;
        MipSet.m_nMaxMipLevels      = m_Bitmaps[0].isCubemap() ? static_cast<CMP_INT>(max_mip_levels_v/6) : static_cast<CMP_INT>(max_mip_levels_v);
        MipSet.m_nMipLevels         = 1;
        MipSet.m_transcodeFormat    = CMP_FORMAT::CMP_FORMAT_Unknown;
        MipSet.m_compressed         = false;
        MipSet.m_isDeCompressed     = CMP_FORMAT::CMP_FORMAT_Unknown;
        MipSet.m_swizzle            = false;
        MipSet.m_nBlockWidth        = 0;
        MipSet.m_nBlockHeight       = 0;
        MipSet.m_nBlockDepth        = 0;
        MipSet.m_isSigned           = m_Descriptor.m_Compression == xtexture_rsc::compression_format::RGB_SHDR_BC6 ? true : false;
        MipSet.m_nChannels          = ChannelConversionTable[static_cast<std::int32_t>(m_Descriptor.m_Compression)];
        MipSet.dwWidth              = MipSet.m_nWidth;
        MipSet.dwHeight             = MipSet.m_nHeight;
        MipSet.dwDataSize           = static_cast<CMP_DWORD>(m_Bitmaps[0].getFaceSize());
        MipSet.pData                = new CMP_BYTE[MipSet.dwDataSize];

        //
        // If we do HDR let us convert to f16 as Compressonator can not handle f32
        //
        memcpy(MipSet.pData, reinterpret_cast<CMP_BYTE*>(m_Bitmaps[0].getMip<std::byte>(0).data()), MipSet.dwDataSize);

        for( int i=1; i< MipSet.m_nDepth; ++i )
        {
            MipLevelTable[i]->m_nWidth          = MipSet.dwWidth;
            MipLevelTable[i]->m_nHeight         = MipSet.m_nHeight;
            MipLevelTable[i]->m_dwLinearSize    = MipSet.dwDataSize;
            MipLevelTable[i]->m_pbData          = new CMP_BYTE[MipSet.dwDataSize];
            memcpy(MipLevelTable[i]->m_pbData, reinterpret_cast<CMP_BYTE*>(m_Bitmaps[0].getMip<std::byte>(0, i).data()), MipSet.dwDataSize);
        }

        MipLevelTable[0]->m_nWidth       = MipSet.dwWidth;
        MipLevelTable[0]->m_nHeight      = MipSet.dwHeight;
        MipLevelTable[0]->m_dwLinearSize = MipSet.dwDataSize;
        MipLevelTable[0]->m_pbData       = MipSet.pData;

        MipSet.m_pMipLevelTable    = MipLevelTable;

        //
        // Generate the mipmaps
        //
        {
            CMP_CFilterParams   CFilterParam  = {};

            CFilterParam.dwMipFilterOptions = 0;
            CFilterParam.nFilterType        = 1;    // Using D3DX options (Seems like it requires the GPU to actually run the filters)

            switch (m_Descriptor.m_MipmapFilter)
            {
            case xtexture_rsc::mipmap_filter::NONE:      CFilterParam.dwMipFilterOptions = CFilterParam.dwMipFilterOptions & 0xFFFFFFE0u | CMP_D3DX_FILTER_NONE;       break;
            case xtexture_rsc::mipmap_filter::POINT:     CFilterParam.dwMipFilterOptions = CFilterParam.dwMipFilterOptions & 0xFFFFFFE0u | CMP_D3DX_FILTER_POINT;      break;
            case xtexture_rsc::mipmap_filter::LINEAR:    CFilterParam.dwMipFilterOptions = CFilterParam.dwMipFilterOptions & 0xFFFFFFE0u | CMP_D3DX_FILTER_LINEAR;     break;
            case xtexture_rsc::mipmap_filter::TRIANGLE:  CFilterParam.dwMipFilterOptions = CFilterParam.dwMipFilterOptions & 0xFFFFFFE0u | CMP_D3DX_FILTER_TRIANGLE;   break;
            case xtexture_rsc::mipmap_filter::BOX:       CFilterParam.dwMipFilterOptions = CFilterParam.dwMipFilterOptions & 0xFFFFFFE0u | CMP_D3DX_FILTER_BOX;        break;
            }                                                                            

            if (m_Descriptor.m_UWrap == xtexture_rsc::wrap_type::MIRROR || m_Descriptor.m_VWrap == xtexture_rsc::wrap_type::MIRROR ) 
                CFilterParam.dwMipFilterOptions |= CMP_D3DX_FILTER_MIRROR;
            else
                CFilterParam.dwMipFilterOptions &= ~CMP_D3DX_FILTER_MIRROR;

            // Does this do anything?
            if (m_Descriptor.m_bSRGB )
                CFilterParam.dwMipFilterOptions |= CMP_D3DX_FILTER_SRGB;
            else
                CFilterParam.dwMipFilterOptions &= ~CMP_D3DX_FILTER_SRGB;

            CFilterParam.nMinSize           = (m_Descriptor.m_bGenerateMips==false) ? std::max(MipSet.m_nHeight, MipSet.m_nWidth) : (m_Descriptor.m_MipCustomMinSize*2-1);//CMP_CalcMaxMipLevel(MipSet.m_nHeight, MipSet.m_nWidth, true));
            CFilterParam.fGammaCorrection   = 1.0f;

            // This line below does not seem to change anything... 
            CFilterParam.useSRGB            = m_Descriptor.m_bSRGB;

            CMP_GenerateMIPLevelsEx(&MipSet, &CFilterParam);
        }

        //
        // Set the compression type
        //
        CMP_MipSet MipSetCompressed;
        {
            KernelOptions KernelOps;

            float Scalar;
                 if (m_OptimizationType == xresource_pipeline::compiler::base::optimization_type::O0) Scalar = 0.0f;
            else if (m_OptimizationType == xresource_pipeline::compiler::base::optimization_type::O1) Scalar = m_Descriptor.m_Quality;
            else if (m_OptimizationType == xresource_pipeline::compiler::base::optimization_type::Oz) Scalar = 1.0f;
            else Scalar = m_Descriptor.m_Quality;

            memset(&KernelOps, 0, sizeof(KernelOps));
            KernelOps.format        = TextureConversionTable[static_cast<std::int32_t>(m_Descriptor.m_Compression)];
            KernelOps.fquality      = std::clamp(Scalar, 0.05f, 1.0f);
            KernelOps.threads       = 0;
            KernelOps.getPerfStats  = true;

                 if (m_DebugType == debug_type::D0) KernelOps.getDeviceInfo = false;
            else if (m_DebugType == debug_type::D1) KernelOps.getDeviceInfo = true;
            else if (m_DebugType == debug_type::Dz) KernelOps.getDeviceInfo = true;

            // Set alpha compatibility for textures that need it
            if (m_Descriptor.m_Compression == xtexture_rsc::compression_format::RGBA_BC1_A1 
             && m_Descriptor.m_UsageType == xtexture_rsc::usage_type::COLOR_AND_ALPHA )
            {
                KernelOps.bc15.useAlphaThreshold = true;
                KernelOps.bc15.alphaThreshold    = m_Descriptor.m_AlphaThreshold;
            }

            // I have not idea what this does...
            KernelOps.useSRGBFrames = m_Descriptor.m_bSRGB;

            //
            // handle gamma textures
            //
            if ( m_Descriptor.m_bSRGB )
            {
                // Set channel weights for better perceptual compression
                KernelOps.bc15.useChannelWeights = true;
                KernelOps.bc15.channelWeights[0] = 0.3086f; // Red
                KernelOps.bc15.channelWeights[1] = 0.6094f; // Green
                KernelOps.bc15.channelWeights[2] = 0.0820f; // Blue
            }

            //
            // Compress the texture
            //
            if(m_Descriptor.m_Compression == xtexture_rsc::compression_format::RGBA_UNCOMPRESSED)
            {
                MipSetCompressed = MipSet;
                memset(&MipSet, 0, sizeof(CMP_MipSet));
            }
            else
            {
                static int   s_ActualProgress;
                static int   s_nMipMaps;
                static int   s_Updates;
                static base* s_pBase;
                static float s_total;

                s_nMipMaps       = MipSet.m_nMipLevels * (m_Bitmaps[0].isCubemap() ? 6 : 1);
                s_ActualProgress = 0;
                s_Updates        = 0;
                s_pBase          = this;
                s_total          = 0;
                memset(&MipSetCompressed, 0, sizeof(CMP_MipSet));

                if ( auto Status = CMP_ProcessTexture(&MipSet, &MipSetCompressed, KernelOps, [](CMP_FLOAT fProgress, CMP_DWORD_PTR, CMP_DWORD_PTR) ->bool
                {
                    if (fProgress >= 100) 
                    {
                        s_ActualProgress++;
                        s_Updates=0;
                        fProgress=0;
                    }
                    else
                    {
                        s_Updates++;
                    }
                    
                    if ((s_Updates%20)==0)
                    {
                        float t       =  (fProgress / 100.f) / static_cast<float>(s_nMipMaps);
                        s_total       =  (s_ActualProgress / static_cast<float>(s_nMipMaps)) + t;
                        s_pBase->displayProgressBar( "Compression", s_total);
                    }

                    return CMP_OK;
                    }); Status != CMP_OK) throw(std::runtime_error("Unable to compress the texture"));

                // Make sure that we have reach 100%
                if (s_total<1)s_pBase->displayProgressBar("Compression", 1);

                CMP_FreeMipSet(&MipSet);
            }
        }

        //
        // Serialize DDS texture (Only for debug mode... since we are using xbmp for the final texture)
        //
        if (m_DebugType == debug_type::D1)
        {
            // Force the DDS file to serialize wih the DX10 Header (Only for gamma textures)
            if (m_Descriptor.m_bSRGB) MipSetCompressed.m_dwFourCC = CMP_MAKEFOURCC('D', 'X', '1', '0');
            auto filename = std::format(L"{}\\FinalImage.dds", m_ResourceLogPath );

            if (auto cmp_status = CMP_SaveTexture( xstrtool::To(filename).c_str(), &MipSetCompressed); cmp_status != CMP_OK)
                throw(std::runtime_error("Unable to export the texture"));

            //  
            // HACK: Hack to convert to sRGB since compressonator does not support it...
            //
            if (m_Descriptor.m_bSRGB)
            {
                constexpr static auto ToSRGB = []() consteval ->auto
                {
                    std::array< std::uint32_t, static_cast<std::int32_t>(xtexture_rsc::compression_format::count_v) > Array = { 0 };
                    Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_BC1)]             = 72;   // DDSFile::DXGIFormat::BC1_UNorm_SRGB
                    Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC1_A1)]         = 72;   // DDSFile::DXGIFormat::BC1_UNorm_SRGB
                    Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC3_A8)]         = 78;   // DDSFile::DXGIFormat::BC3_UNorm_SRGB
                    Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC7)]            = 99;   // DDSFile::DXGIFormat::BC7_UNorm_SRGB
                    Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_UNCOMPRESSED)]   = 29;   // DDSFile::DXGIFormat::R8G8B8A8_UNorm_SRGB
                    return Array;
                }();

                if(auto NewFormat = ToSRGB[static_cast<std::int32_t>(m_Descriptor.m_Compression)]; NewFormat)
                {
                    // IF it suposed to have gamma make sure to convert it to sRGB
                    FILE* fp = nullptr;
                        
                    if( auto Err = _wfopen_s( &fp, filename.c_str(), L"r+b"); Err != 0 || fp == nullptr)
                    {
                        std::array<wchar_t, 256> errMsg;
                        if (_wcserror_s(errMsg.data(), errMsg.size(), Err) == 0)
                        {
                            xerr::LogMessage<state::FAILURE>(std::format("Unable to reload the Debug dds: {}, {}", Err, xstrtool::To(errMsg.data())));
                        }
                        else
                        {
                            xerr::LogMessage<state::FAILURE>(std::format("Unable to reload the Debug dds, system unknown error: {}", Err));
                        }

                        throw(std::runtime_error("Unable to reload the Debug dds..."));
                    }
                        

                    // in the DDS file offset to dxgiFormat part of the DX10 header
                    if (-1 == fseek(fp, 128, SEEK_SET))        
                        throw(std::runtime_error("Unable to reload the Debug dds..."));

                    if (1 != fwrite(&NewFormat, 4, 1, fp))
                        throw(std::runtime_error("Unable to reload the Debug dds..."));

                    fclose(fp);
                }
            }
        }

        //
        // Convert from Mipset to xbitmap
        //
        static constexpr auto DescriptorBitmapFormatToxBitmap = []() consteval ->auto
        {
            std::array< xbitmap::format, static_cast<std::int32_t>(xtexture_rsc::compression_format::count_v) > Array = { xbitmap::format::XCOLOR_END };
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_BC1)]             = xbitmap::format::BC1_4RGB;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC1_A1)]         = xbitmap::format::BC1_4RGBA1;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC3_A8)]         = xbitmap::format::BC3_8RGBA;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::R_BC4)]               = xbitmap::format::BC4_4R;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RG_BC5)]              = xbitmap::format::BC5_8RG;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_UHDR_BC6)]        = xbitmap::format::BC6H_8RGB_UFLOAT;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGB_SHDR_BC6)]        = xbitmap::format::BC6H_8RGB_SFLOAT;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_BC7)]            = xbitmap::format::BC7_8RGBA;
            Array[static_cast<std::int32_t>(xtexture_rsc::compression_format::RGBA_UNCOMPRESSED)]   = xbitmap::format::XCOLOR;
            return Array;
        }();

        if (DescriptorBitmapFormatToxBitmap[static_cast<std::int32_t>(m_Descriptor.m_Compression)] == xbitmap::format::XCOLOR_END)
            throw(std::runtime_error("Unable to convert the texture to xbitmap"));

        //
        // Set up the Final xBitmap
        //
        {
            //
            // Set up the actual Final xBitmap
            //
            {
                const auto CompressTotalMips = MipSetCompressed.m_nMipLevels * m_Bitmaps[0].getFaceCount();

                // Compute total memory require for the texture
                std::uint32_t TotalTexelByteSize = 0;
                for ( int i=0; i< CompressTotalMips; i ++ )
                {
                    TotalTexelByteSize += MipSetCompressed.m_pMipLevelTable[i]->m_dwLinearSize;
                }

                std::uint32_t FaceTexelByteSize = 0;
                for (int i = 0; i < CompressTotalMips; i += m_Bitmaps[0].getFaceCount())
                {
                    FaceTexelByteSize += MipSetCompressed.m_pMipLevelTable[i]->m_dwLinearSize;
                }

                assert( [&]{auto total = FaceTexelByteSize * m_Bitmaps[0].getFaceCount(); return total  == TotalTexelByteSize; }()  );

                const auto MipTableSize = sizeof(xbitmap::mip) * MipSetCompressed.m_nMipLevels;
                std::unique_ptr<std::byte[]> TextureData = std::make_unique<std::byte[]>(MipTableSize + TotalTexelByteSize);

                // Set the mip table as well
                {
                    auto            pMipTable = reinterpret_cast<xbitmap::mip*>(TextureData.get());
                    int             CurOffset = 0;
                    for (int i = 0, j=0; i < CompressTotalMips; i+= m_Bitmaps[0].getFaceCount(), j++ )
                    {
                        // Handle copying the mip data
                        const auto& Mip = *MipSetCompressed.m_pMipLevelTable[i];

                        // Handle the mip table
                        pMipTable[j].m_Offset = CurOffset;

                        // Update the trackers
                        CurOffset += Mip.m_dwLinearSize;
                    }
                }

                //
                // Set the final data structure
                //
                m_FinalBitmap.setup
                ( static_cast<std::uint32_t>(MipSetCompressed.m_nWidth)
                , static_cast<std::uint32_t>(MipSetCompressed.m_nHeight)
                , m_Descriptor.m_Compression == xtexture_rsc::compression_format::RGBA_UNCOMPRESSED ? m_Bitmaps[0].getFormat() : DescriptorBitmapFormatToxBitmap[static_cast<std::int32_t>(m_Descriptor.m_Compression)]
                , static_cast<std::uint64_t>(TotalTexelByteSize / (m_Bitmaps[0].isCubemap()?6:1))
                , { reinterpret_cast<std::byte*>(TextureData.release()), static_cast<std::uint64_t>(MipTableSize + TotalTexelByteSize) }
                , true
                , MipSetCompressed.m_nMipLevels
                , 1
                , m_Bitmaps[0].isCubemap()
                );

                //
                // Make sure the final xbitmap has all the basics setup
                //
                m_FinalBitmap.setColorSpace(m_Descriptor.m_bSRGB ? xbitmap::color_space::SRGB : xbitmap::color_space::LINEAR);
                m_FinalBitmap.setUWrapMode(m_Bitmaps[0].getUWrapMode());
                m_FinalBitmap.setVWrapMode(m_Bitmaps[0].getVWrapMode());

                //
                // Upgrade formats for Normals maps when required
                //
                if (m_Descriptor.m_UsageType == xtexture_rsc::usage_type::TANGENT_NORMAL)
                {
                    // These two formats require special decoding...
                         if (m_FinalBitmap.getFormat() == xbitmap::format::BC3_8RGBA) m_FinalBitmap.setFormat(xbitmap::format::BC3_81Y0X_NORMAL);
                    else if (m_FinalBitmap.getFormat() == xbitmap::format::BC5_8RG)   m_FinalBitmap.setFormat(xbitmap::format::BC5_8YX_NORMAL);
                }
            }

            //
            // Copy the actual data to our final bitmap
            //
            for ( int iFace =0; iFace < m_FinalBitmap.getFaceCount(); iFace++ )
            {
                for( int iMip = 0; iMip < m_FinalBitmap.getMipCount(); iMip++  )
                {
                    auto  xBmpData = m_FinalBitmap.getMip<std::byte>(iMip, iFace);
                    auto& ComPress = *MipSetCompressed.m_pMipLevelTable[iMip * m_FinalBitmap.getFaceCount() + iFace ];

                    assert( ComPress.m_dwLinearSize == xBmpData.size() );

                    // Handle copying the mip data
                    memcpy(xBmpData.data(), ComPress.m_pbData, ComPress.m_dwLinearSize);
                }
            }
        }

        CMP_FreeMipSet(&MipSetCompressed);
    }

    //---------------------------------------------------------------------------------------------

    virtual ~implementation()
    {
        if (m_pDDSData != nullptr)
            crn_free_block(m_pDDSData);
    }

    //---------------------------------------------------------------------------------------------

    void Serialize(const std::wstring_view FilePath)
    {
        //
        // We serialize the final image as a xbmp because the file size is usually half the size of a DDS file
        //
        auto FinalPath = FilePath;

        {
            xserializer::stream Serializer;
            if ( auto Err = Serializer.Save( FinalPath
                                            , m_FinalBitmap
                                            ,     m_OptimizationType == optimization_type::O0 ? xserializer::compression_level::FAST 
                                                : m_OptimizationType == optimization_type::O1 ? xserializer::compression_level::MEDIUM
                                                : xserializer::compression_level::HIGH
                                            ); Err )
            {
                throw(std::runtime_error( std::string(Err.getMessage()) ));
            }
        }

        //
        // Verify this can be loaded... 
        //
        if( m_DebugType == debug_type::Dz )
        {
            xbitmap* pTemp;
            xserializer::stream Serializer;

            if (auto Err = Serializer.Load(FinalPath, pTemp); Err)
                throw(std::runtime_error( std::string( Err.getMessage() ) ));

            //
            // OK Time to let things go...
            //
            xserializer::default_memory_handler_v.Free( { .m_bUnique = true }, pTemp );
        }
    }
};

//---------------------------------------------------------------------------------------------

namespace xtexture_compiler
{
    std::unique_ptr<instance> instance::Create( void )
    {
        return std::make_unique<implementation>();
    }
}

