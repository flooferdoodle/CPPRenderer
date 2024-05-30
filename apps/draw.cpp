#include "../include/GColor.h"
#include "../include/GBitmap.h"
#include "../include/GBlend.h"
#include "../MyCanvas.h"
#include <string>
#include "../Camera.h"
#include "../Projector.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    // Handle command inputs
    if(argc < 2) {
        cout << "Need to specify json scene file to render." << endl;
        cout << "Command: ./render <json file> [-o filename]" << endl;
        return -1;
    }

    string sceneFile(argv[1]);
    string title = "image";
    bool verbose = false;
    for(int i = 2; i < argc; ++i) {
        if(string(argv[i]) == "-o") {
            if(i + 1 >= argc) throw CustomException("Unspecified output filename.");
            title = string(argv[i+1]);
        }
        else if(string(argv[i]) == "-v") verbose = true;
    }


    // Build scene
    SceneBuilder builder(sceneFile);

    GBitmap bitmap;
    bitmap.alloc(256, 256);
    auto canvas = GCreateCanvas(bitmap);
    if(!canvas){
        cout << "Failed to create bitmap." << endl;
        return -1;
    }

    Projector projector(canvas.get(), GISize{bitmap.width(), bitmap.height()}, &bitmap);

    RenderStatistic stats = projector.RenderSceneTo(builder.getScene(), *canvas,
                                                     {bitmap.width(), bitmap.height()});

    string filename = title + ".png";
    bitmap.writeToFile(filename.c_str());

    cout << "Rendered " << filename << " in " << stats.secondsTaken * 1000.f << "ms (" << 1.f / stats.secondsTaken << " fps)" << endl;
    cout << "# Objects:\t" << stats.numObjects << endl;
    cout << "# Triangles:\t" << stats.numTrisDrawn << "/" << stats.numTrisTotal << endl;

    // DEBUG: Show buffers
    if(verbose){
        GBitmap buff_bitmap;
        buff_bitmap.alloc(256, 256);

        filename = title + "_depth.png";
        projector.ShowBuffer(Projector::BufferType::Depth, buff_bitmap, builder.getScene());
        buff_bitmap.writeToFile(filename.c_str());
        
        filename = title + "_invdepth.png";
        projector.ShowBuffer(Projector::BufferType::Inv_Depth, buff_bitmap, builder.getScene());
        buff_bitmap.writeToFile(filename.c_str());

        filename = title + "_normal.png";
        projector.ShowBuffer(Projector::BufferType::Normal, buff_bitmap, builder.getScene());
        buff_bitmap.writeToFile(filename.c_str());

        filename = title + "_albedo.png";
        projector.ShowBuffer(Projector::BufferType::Albedo, buff_bitmap, builder.getScene());
        buff_bitmap.writeToFile(filename.c_str());

        filename = title + "_specular.png";
        projector.ShowBuffer(Projector::BufferType::Specular, buff_bitmap, builder.getScene());
        buff_bitmap.writeToFile(filename.c_str());

        filename = title + "_position.png";
        projector.ShowBuffer(Projector::BufferType::Position, buff_bitmap, builder.getScene());
        buff_bitmap.writeToFile(filename.c_str());

        /*
        copyToCanvas(&bitmap, Projector::getDepthBuffer());
        bitmap.writeToFile(filename.c_str());
        filename = title + "_invdepth.png";
        copyToCanvas(&bitmap, Projector::getInvDepthBuffer());
        bitmap.writeToFile(filename.c_str());

        filename = title + "_position.png";
        copyToCanvas(&bitmap, Projector::getPositionBuffer());
        bitmap.writeToFile(filename.c_str());

        filename = title + "_normal.png";
        copyToCanvas(&bitmap, Projector::getNormalBuffer());
        bitmap.writeToFile(filename.c_str());


        filename = title + "_albedo.png";
        copyToCanvas(&bitmap, Projector::getAlbedoBuffer());
        bitmap.writeToFile(filename.c_str());*/

    }

    return 0;
}