# OpenADAS - Compile and Run OpenADAS on Jetson Xavier

Open-source advanced driver-assistance system on Jetson Xavier.

This tutorial guides you on how to set up OpenADAS system on a Jetson Xavier.

## I. Environment Setup

### 1. JetPack and TensorRT

- Download and Install Jetson Xavier board with JetPack 4.3 from SDK Manager. See [this link](https://developer.nvidia.com/jetpack-43-archive).

- Setup CUDA 10.0, CuDNN 7.6 and TensorRT 6.0.1 by following command:

```
sudo apt install cuda-toolkit-10-0 tensorrt
```

### 2. OpenCV, Qt, Protobuf

```
sudo apt install -y libopencv libopencv-dev
sudo apt install -y qt5-default qtcreator qt5-doc qt5-doc-html qtbase5-doc-html qtbase5-examples qttools5-dev-tools libqt5svg5-dev qtmultimedia5-dev
sudo apt install -y libprotoc10 libprotoc-dev protobuf-compiler
sudo /sbin/ldconfig -v
```

### 3. Build tools

```
sudo apt install build-essential cmake nano
```

### 4. Development tools

- Setup VNC: <https://developer.nvidia.com/embedded/learn/tutorials/vnc-setup>.
- For remote development, we recommend VS Code SSH: <https://code.visualstudio.com/docs/remote/ssh>.


## II. Build and Run

### 1. Clone the source code

```
cd ~
mkdir Works
cd Works
git clone https://github.com/vietanhdev/open-adas
```

### 2. Update GPU_ARCHS

Find these lines in `CMakeLists.txt`:

```
# Set GPU architecture. This decides which instruction set will be used for GPU code.
set(GPU_ARCHS 75)  ## config your GPU_ARCHS,See [here](https://developer.nvidia.com/cuda-gpus) for finding what maximum compute capability your specific GPU supports.
```

Replace 75 with 72 for Jetson Xavier. Please read more on [this link](https://developer.nvidia.com/cuda-gpus).

### 3. Download models and sample data

Download models and sample data [here](https://drive.google.com/drive/folders/1-DDchZQNOWpppNX8udyKj0OViDhYD38O?usp=sharing) and extract to `open-adas/models` and `open-adas/data`.

### 4. Build

```
mkdir build
cd build
cmake ..
make -j4
```

### 5. Run

```
cd ~/Works/open-adas/build/bin
bash setup_vcan.sh
./OpenADAS
```

