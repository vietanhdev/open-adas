#if !defined(CAMERA_H)
#define CAMERA_H

#include <string>

struct Camera { 
   int v4l_id;
   std::string identifier;
   Camera(int v4l_id, std::string identifier) :
    v4l_id(v4l_id), identifier(identifier)
    {}
};

#endif