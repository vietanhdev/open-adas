# Advanced driver-assistance system using Jetson Nano

An advanced driver-assistance system on Jetson Nano embedded computer with four main functions: forward collision warning, lane departure warning, traffic sign recognition and overspeed warning. This repository contains source code for Jetson Nano, not including the source code for model training and conversion.

**Blog posts:**

- <https://aicurious.io/posts/adas-jetson-nano-intro-and-hardware/>.
- <https://aicurious.io/posts/adas-jetson-nano-software/>.
- https://aicurious.io/posts/adas-jetson-nano-deep-neural-networks/.

**Update 21/12/2020:** I created an image of my SD card [here](https://drive.google.com/file/d/1jg68ySnTt4Zm_hb4JVZKKWXRWkDucqUu/view?usp=sharing). You can flash and run this image on Jetson Nano.

**For TensorRT 7 support:** Currently, only TensorRT 5 and TensorRT 6 are supported. TensorRT 7 has a lot of deprecated APIs and I think there is no way to run this project directly with that version. I don't have time to continue with this project soon, so I really need your contributions to extend this project further. 

## I. DEVELOPMENT ENVIRONMENT AND BUILD

### Requirements:

- CMake >= 3.10
- Qt 5
- OpenCV >= 4.0.1
- C++ 17 compiler

- CUDA 10.1
- TensorRT 5.1.5-1+cuda10.1. **This project should work with TensorRT 5 and TensorRT 6. TensorRT 7 is not supported for now.**

### Setup for Linux - Ubuntu 18.04

#### Setup

- Install QT:

```
sudo apt-get install build-essential
sudo apt-get install qtcreator
sudo apt-get install qt5-default
sudo apt-get install qt5-doc
sudo apt-get install qt5-doc-html qtbase5-doc-html
sudo apt-get install qtbase5-examples
sudo /sbin/ldconfig -v
```

- Install OpenCV

```
https://linuxize.com/post/how-to-install-opencv-on-ubuntu-18-04/
```

- Install protobuf 3.6.1

```
https://github.com/protocolbuffers/protobuf
```

#### Models and test data

- Download models and testing data [here](https://drive.google.com/drive/folders/1-DDchZQNOWpppNX8udyKj0OViDhYD38O?usp=sharing) and put into root folder of this project.

#### Compile and Run

- Compile
```
cd <project directory>
mkdir build
cd build
cmake -DCUDA_INCLUDE_DIRS=/usr/local/cuda-10.1/include ..
make
```

- Run
```
./CarSmartCam
```


#### Known errors

- `/usr/bin/ld: cannot find -lcudart`:
```
sudo ln -s /usr/local/cuda/lib64/libcudart.so /usr/lib/libcudart.so
```

- `/usr/bin/ld: cannot find -lcublas`:
```
sudo ln -s /usr/local/cuda/lib64/libcublas.so /usr/lib/libcublas.so
```

## II. REFERENCES:

- In this project, we use code from:
    + Dark Theme for Qt5 by Juergen Skrotzky: [https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle](https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle).

- ICSim
