# Installation

<!-- toc -->

* [Distribution packages](#distribution-packages)
  * [Ubuntu](#ubuntu)
  * [Fedora](#fedora)
  * [Conda](#conda)
  * [Windows](#windows)
* [Building from source](#building-from-source)
  * [Linux](#linux)
  * [macos](#macos)
  * [Windows](#windows-1)
    * [Visual Studio native build](#visual-studio-native-build)

<!-- tocstop -->

### Distribution packages

#### Ubuntu

```bash
# Currently supported Ubuntu versions are Focal, Jammy and Kinetic
sudo add-apt-repository ppa:bkryza/clang-uml
sudo apt update
sudo apt install clang-uml
```

#### Fedora

```bash
# Fedora 36
wget https://github.com/bkryza/clang-uml/releases/download/0.3.1/clang-uml-0.3.1-1.fc36.x86_64.rpm
sudo dnf install ./clang-uml-0.3.1-1.fc36.x86_64.rpm

# Fedora 37
wget https://github.com/bkryza/clang-uml/releases/download/0.3.1/clang-uml-0.3.1-1.fc37.x86_64.rpm
sudo dnf install ./clang-uml-0.3.1-1.fc37.x86_64.rpm
```

#### Conda

```bash
conda config --add channels conda-forge
conda config --set channel_priority strict
conda install -c bkryza/label/clang-uml clang-uml
```

#### Windows

Download and run the latest Windows installer from
[Releases page](https://github.com/bkryza/clang-uml/releases).

### Building from source

#### Linux
First make sure that you have the following dependencies installed:

```bash
# Ubuntu (clang version will vary depending on Ubuntu version)
apt install ccache cmake libyaml-cpp-dev clang-12 libclang-12-dev libclang-cpp12-dev pkg-config
```

Then proceed with building the sources:

```bash
git clone https://github.com/bkryza/clang-uml
cd clang-uml
# Please note that top level Makefile is just a convenience wrapper for CMake
make release
release/src/clang-uml --help

# To build using a specific installed version of LLVM use:
LLVM_VERSION=14 make release

# Optionally
make install
# or
export PATH=$PATH:$PWD/release
```

#### macos

```bash
brew install ccache cmake llvm yaml-cpp

export CC=/usr/local/opt/llvm/bin/clang
export CCX=/usr/local/opt/llvm/bin/clang++
LLVM_VERSION=14 make release
```

#### Windows

##### Visual Studio native build

These steps present how to build and use `clang-uml` natively using Visual Studio only.

First, install the following dependencies manually:

* [Python 3](https://www.python.org/downloads/windows/)
* [Git](https://git-scm.com/download/win)
* [CMake](https://cmake.org/download/)
* [Visual Studio](https://visualstudio.microsoft.com/vs/community/)

> All the following steps should be invoked in `Developer PowerShell for VS`.

Create installation directory for `clang-uml` and its dependencies:

```bash
# This is where clang-uml binary and its dependencies will be installed after build
# If you change this path, adapt all consecutive steps
mkdir C:\clang-uml
# This directory will be removed after build
mkdir C:\clang-uml-tmp
cd C:\clang-uml-tmp
```

Build and install `yaml-cpp`:

```bash
git clone https://github.com/jbeder/yaml-cpp
cd yaml-cpp
git checkout yaml-cpp-0.7.0
cd ..
cmake -S .\yaml-cpp\ -B .\yaml-cpp-build\ -DCMAKE_INSTALL_PREFIX="C:\clang-uml" -Thost=x64
cd yaml-cpp-build
msbuild .\INSTALL.vcxproj -maxcpucount /p:Configuration=Release
```

Build and install `LLVM`:

```bash 
pip install psutil
# Update the LLVM branch if necessary
git clone --branch llvmorg-15.0.6 --depth 1 https://github.com/llvm/llvm-project.git llvm
cmake -S .\llvm\llvm -B llvm-build -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_INSTALL_PREFIX="C:\clang-uml" -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86 -Thost=x64
cd llvm-build
msbuild .\INSTALL.vcxproj -maxcpucount /p:Configuration=Release
```

Build and install `clang-uml`:

```bash
git clone https://github.com/bkryza/clang-uml
cmake -S .\clang-uml\ -B .\clang-uml-build\ -DCMAKE_INSTALL_PREFIX="C:\clang-uml" -DCMAKE_PREFIX_PATH="C:\clang-uml" -DBUILD_TESTS=OFF -Thost=x64
cd clang-uml-build
msbuild .\INSTALL.vcxproj -maxcpucount /p:Configuration=Release
```

Check if `clang-uml` works:

```bash
cd C:\clang-uml
bin\clang-uml.exe --version
```
It should produce something like:
```bash
clang-uml 0.3.1
Copyright (C) 2021-2023 Bartek Kryza <bkryza@gmail.com>
Built against LLVM/Clang libraries version: 15.0.6
Using LLVM/Clang libraries version: clang version 15.0.6 (https://github.com/llvm/llvm-project.git 088f33605d8a61ff519c580a71b1dd57d16a03f8)
```

Finally, remove the temporary build directory:

```bash
rm -r C:\clang-uml-tmp
```