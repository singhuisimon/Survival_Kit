#include "xtexture_compiler.h"

//---------------------------------------------------------------------------------

int main( int argc, const char* argv[] )
{
    //
    // This is just for debugging
    //
    if constexpr (false)
    {
        static const char* pDebugArgs[] = 
        { "TextureCompiler"
        , "-PROJECT"
        , "D:\\LIONant\\xGPU\\dependencies\\xtexture.plugin\\bin\\example.lion_project"
        , "-DEBUG"
        , "D1"
        , "-DESCRIPTOR"
        , "Descriptors\\Texture\\D7\\00\\189E8EEFD88400D7.desc" 
        , "-OUTPUT"
        , "D:\\LIONant\\xGPU\\dependencies\\xtexture.plugin\\bin\\example.lion_project\\Cache\\Resources\\Platforms"
        };

        argv = pDebugArgs;
        argc = static_cast<int>(sizeof(pDebugArgs) / sizeof(pDebugArgs[0]));
    }

    //
    // Create the compiler instance
    //
    auto TextureCompilerPipeline = xtexture_compiler::instance::Create();

    //
    // Parse parameters
    //
    if( auto Err = TextureCompilerPipeline->Parse( argc, argv ); Err )
    {
        Err.ForEachInChain( [&](xerr Error)
        {
            auto Hint   = Err.getHint();
            auto String = std::format("Error: {}\n", Err.getMessage());
            printf("%s", String.c_str());
            if (Hint.empty() == false ) 
                printf("Hint: %s\n", Hint.data() );
        });
        return 1;
    }

    //
    // Start compilation
    //
    if( auto Err = TextureCompilerPipeline->Compile(); Err )
    {
        Err.ForEachInChain([&](xerr Error)
        {
            auto Hint   = Err.getHint();
            auto String = std::format("Error: {}\n", Err.getMessage());
            printf("%s", String.c_str());
            if (Hint.empty() == false ) 
                printf("Hint: %s\n", Hint.data() );
        });
        return 1;
    }

    return 0;
}


