# CarSmartCam

Smart dash camera for car.

## I. DEVELOPMENT ENVIRONMENT AND BUILD

### Requirements:

- CMake >= 3.10
- Qt 5
- OpenCV >= 4.0.1
- C++ 17 compiler

- CUDA 10.1
- TensorRT 5.1.5-1+cuda10.1

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

- Install SDL2 lib
```
sudo apt-get install libsdl2-dev
```

- Install protobuf 3.6.1

```
https://github.com/protocolbuffers/protobuf
```

- Install pycuda on Python 2.7:
```
sudo python -m pip install --global-option=build_ext --global-option="-I/usr/local/cuda-10.0/targets/aarch64-linux/include/" --global-option="-L/usr/local/cuda-10.0/targets/aarch64-linux/lib/" pycuda
```

#### Compile and Run

- Compile
```
cd <project directory>
mkdir build
cd build
cmake -DCUDA_INCLUDE_DIRS=/usr/local/cuda-10.0/include ..
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

- While running boost::python with numpy in a separate thread, you have to run `np::initialize();` for that thread.


## III. REFERENCES:

- In this project, we use code from:
    + Dark Theme for Qt5 by Juergen Skrotzky: [https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle](https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle).
