# Build Instructions
This project fully supports Linux and Windows today.
Support for Android is TBD.

## Git the Bits

You should have access to the Khronos GitHub repository at https://github.com/KhronosGroup/. The
preferred work flow is to clone the repo, create a branch, push branch to GitHub and then
issue a merge request to integrate that work back into the repo.

Note: If you are doing ICD (driver) development, please make sure to look at documentation in the [ICD Loader](loader/README.md) and the [Sample Driver](icd).

## Linux System Requirements
Ubuntu 14.04.3 LTS, 14.10, 15.04 and 15.10 have been used with the sample driver.

These packages are used for building and running the samples.
```
sudo apt-get install git subversion cmake libgl1-mesa-dev freeglut3-dev libglm-dev qt5-default libpciaccess-dev libpthread-stubs0-dev libudev-dev bison graphviz libpng-dev
sudo apt-get build-dep mesa
```

The sample driver uses DRI3 for its window system interface.
That requires extra configuration of Ubuntu systems.

### Ubuntu 14.04.3 LTS support of DRI 3

Ubuntu 14.04.3 LTS does not ship a xserver-xorg-video-intel package with supported DRI 3 on intel graphics.
The xserver-xorg-video-intel package can be built from source with DRI 3 enabled.
Use the following commands to enable DRI3 on ubuntu 14.04.3 LTS.

- Install packages used to build:
```
sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get install devscripts
sudo apt-get build-dep xserver-xorg-video-intel-lts-vivid
```

- Get the source code for xserver-xorg-video-intel-lts-vivid
```
mkdir xserver-xorg-video-intel-lts-vivid_source
cd xserver-xorg-video-intel-lts-vivid_source
apt-get source xserver-xorg-video-intel-lts-vivid
cd xserver-xorg-video-intel-lts-vivid-2.99.917
debian/rules patch
quilt new 'enable-DRI3'
quilt edit configure.ac
```

- Use the editor to make these changes.
```
--- a/configure.ac
+++ b/configure.ac
@@ -340,9 +340,9 @@
 	      [DRI2=yes])
 AC_ARG_ENABLE(dri3,
 	      AS_HELP_STRING([--enable-dri3],
-			     [Enable DRI3 support [[default=no]]]),
+			     [Enable DRI3 support [[default=yes]]]),
 	      [DRI3=$enableval],
-	      [DRI3=no])
+	      [DRI3=yes])
 AC_ARG_ENABLE(xvmc, AS_HELP_STRING([--disable-xvmc],
                                   [Disable XvMC support [[default=yes]]]),
```
- Build and install xserver-xorg-video-intel-lts-vivid
```
quilt refresh
debian/rules clean
debuild -us -uc
sudo dpkg -i ../xserver-xorg-video-intel-lts-vivid_2.99.917-1~exp1ubuntu2.2~trusty1_amd64.deb
```
- Prevent updates from replacing this version of the package.
```
sudo bash -c 'echo xserver-xorg-video-intel-lts-vivid hold | dpkg --set-selections'
```
- save your work then restart the X server with the next command.
```
sudo service lightdm restart
```
- After logging in again, check for success with this command and look for DRI3.
```
xdpyinfo | grep DRI
```

### Ubuntu 14.10 support of DRI 3

Warning: Recent versions of 14.10 have **REMOVED** DRI 3.
Version: 2:2.99.914-1~exp1ubuntu4.1 is known to work.
To see status of this package:
```
dpkg -s xserver-xorg-video-intel
```

Note:
Version 2:2.99.914-1~exp1ubuntu4.2 does not work anymore.
To install the working driver from launchpadlibrarian.net:
- Remove the current driver:
```
sudo apt-get purge xserver-xorg-video-intel
```
- Download the old driver:
```
wget http://launchpadlibrarian.net/189418339/xserver-xorg-video-intel_2.99.914-1%7Eexp1ubuntu4.1_amd64.deb
```
- Install the driver:
```
sudo dpkg -i xserver-xorg-video-intel_2.99.914-1~exp1ubuntu4.1_amd64.deb
```
- Pin the package to prevent updates
```
sudo bash -c "echo $'Package: xserver-xorg-video-intel\nPin: version 2:2.99.914-1~exp1ubuntu4.1\nPin-Priority: 1001' > /etc/apt/preferences.d/xserver-xorg-video-intel"
```

- Either restart Ubuntu or just X11.


### Ubuntu 15.04 support of DRI 3

Ubuntu 15.04 has never shipped a xserver-xorg-video-intel package with supported DRI 3 on intel graphics.
The xserver-xorg-video-intel package can be built from source with DRI 3 enabled.
Use the following commands to enable DRI3 on ubuntu 15.04.

- Install packages used to build:
```
sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get install devscripts
sudo apt-get build-dep xserver-xorg-video-intel
```

- Get the source code for xserver-xorg-video-intel
```
mkdir xserver-xorg-video-intel_source
cd xserver-xorg-video-intel_source
apt-get source xserver-xorg-video-intel
cd xserver-xorg-video-intel-2.99.917
debian/rules patch
quilt new 'enable-DRI3'
quilt edit configure.ac
```

- Use the editor to make these changes.
```
--- a/configure.ac
+++ b/configure.ac
@@ -340,9 +340,9 @@
 	      [DRI2=yes])
 AC_ARG_ENABLE(dri3,
 	      AS_HELP_STRING([--enable-dri3],
-			     [Enable DRI3 support [[default=no]]]),
+			     [Enable DRI3 support [[default=yes]]]),
 	      [DRI3=$enableval],
-	      [DRI3=no])
+	      [DRI3=yes])
 AC_ARG_ENABLE(xvmc, AS_HELP_STRING([--disable-xvmc],
                                   [Disable XvMC support [[default=yes]]]),
```
- Build and install xserver-xorg-video-intel
```
quilt refresh
debian/rules clean
debuild -us -uc
sudo dpkg -i ../xserver-xorg-video-intel_2.99.917-1~exp1ubuntu2.2_amd64.deb
```
- Prevent updates from replacing this version of the package.
```
sudo bash -c 'echo xserver-xorg-video-intel hold | dpkg --set-selections'
```
- save your work then restart the X server with the next command.
```
sudo service lightdm restart
```
- After logging in again, check for success with this command and look for DRI3.
```
xdpyinfo | grep DRI
```
### Ubuntu 15.10 support of DRI 3

Ubuntu 15.10 has never shipped a xserver-xorg-video-intel package with supported DRI 3 on intel graphics.
The xserver-xorg-video-intel package can be built from source with DRI 3 enabled.
Use the following commands to enable DRI3 on ubuntu 15.10.

- Install packages used to build:
```
sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get install devscripts
sudo apt-get build-dep xserver-xorg-video-intel
```

- Get the source code for xserver-xorg-video-intel
```
mkdir xserver-xorg-video-intel_source
cd xserver-xorg-video-intel_source
apt-get source xserver-xorg-video-intel
cd xserver-xorg-video-intel-2.99.917+git20150808
debian/rules patch
quilt new 'enable-DRI3'
quilt edit configure.ac
```

- Use the editor to make these changes.
```
Index: xserver-xorg-video-intel-2.99.917+git20150808/configure.ac
===================================================================
--- xserver-xorg-video-intel-2.99.917+git20150808.orig/configure.ac
+++ xserver-xorg-video-intel-2.99.917+git20150808/configure.ac
@@ -356,7 +356,7 @@ AC_ARG_WITH(default-dri,
            AS_HELP_STRING([--with-default-dri],
                           [Select the default maximum DRI level [default 2]]),
              [DRI_DEFAULT=$withval],
-             [DRI_DEFAULT=2])
+             [DRI_DEFAULT=3])
 if test "x$DRI_DEFAULT" = "x0"; then
        AC_DEFINE(DEFAULT_DRI_LEVEL, 0,[Default DRI level])
 else
```
- Build and install xserver-xorg-video-intel
```
quilt refresh
debian/rules clean
debuild -us -uc
sudo dpkg -i ../xserver-xorg-video-intel_2.99.917+git20150808-0ubuntu4_amd64.deb
```
- Prevent updates from replacing this version of the package.
```
sudo bash -c 'echo xserver-xorg-video-intel hold | dpkg --set-selections'
```
- save your work then restart the X server with the next command.
```
sudo service lightdm restart
```
- After logging in again, check for success with this command and look for DRI3.
```
xdpyinfo | grep DRI
```


## Clone the repository

To create your local git repository:
```
mkdir YOUR_DEV_DIRECTORY  # it's called Vulkan-LoaderAndValidationLayers on Github, but the name doesn't matter
cd YOUR_DEV_DIRECTORY
git clone -o khronos https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers .
# Or substitute the URL from your forked repo for https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers above.
```

## Linux Build

The sample driver uses cmake and should work with the usual cmake options and utilities.
The standard build process builds the icd, the icd loader and all the tests.

Example debug build:
```
cd YOUR_DEV_DIRECTORY  # cd to the root of the Vulkan-LoaderAndValidationLayers git repository
./update_external_sources.sh  # Fetches and builds glslang, llvm, LunarGLASS, and spirv-tools
cmake -H. -Bdbuild -DCMAKE_BUILD_TYPE=Debug
cd dbuild
make
```

To run Vulkan programs you must tell the icd loader where to find the libraries.
This is described in a specification in the Khronos documentation Git
repository.  See the file:
https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/blob/sdk-1.0.3/loader/LoaderAndLayerInterface.md#vulkan-installable-client-driver-interface-with-the-loader

This specification describes both how ICDs and layers should be properly
packaged, and how developers can point to ICDs and layers within their builds.


## Validation Test

The test executables can be found in the dbuild/tests directory. The tests use the Google
gtest infrastructure. Tests available so far:
- vk_layer_validation_tests: Test Vulkan layers.

There are also a few shell and Python scripts that run test collections (eg,
`run_all_tests.sh`).

## Linux Demos

The demos executables can be found in the dbuild/demos directory. The demos use DRI 3
to render directly onto window surfaces.
- vulkaninfo: report GPU properties
- tri: a textured triangle
- cube: a textured spinning cube

## Windows System Requirements

Windows 7+ with additional required software packages:

- Microsoft Visual Studio 2013 Professional.  Note: it is possible that lesser/older versions may work, but that has not been tested.
- CMake (from http://www.cmake.org/download/).  Notes:
  - Tell the installer to "Add CMake to the system PATH" environment variable.
- Python 3 (from https://www.python.org/downloads).  Notes:
  - Select to install the optional sub-package to add Python to the system PATH environment variable.
  - Need python3.3 or later to get the Windows py.exe launcher that is used to get python3 rather than python2 if both are installed on Windows
- Git (from http://git-scm.com/download/win).
  - Note: If you use Cygwin, you can normally use Cygwin's "git.exe".  However, in order to use the "update_external_sources.bat" script, you must have this version.
  - Tell the installer to allow it to be used for "Developer Prompt" as well as "Git Bash".
  - Tell the installer to treat line endings "as is" (i.e. both DOS and Unix-style line endings).
  - Install each a 32-bit and a 64-bit version, as the 64-bit installer does not install the 32-bit libraries and tools.
- glslang is required for demos and tests.
  - You can download and configure it (in a peer directory) here: https://github.com/KhronosGroup/glslang/blob/master/README.md
  - A windows batch file has been included that will pull and build the correct version.  Run it from Developer Command Prompt for VS2013 like so:
    - update_external_sources.bat --build-glslang

Optional software packages:

- Cygwin (from https://www.cygwin.com/).  Notes:
  - Cygwin provides some Linux-like tools, which are valuable for obtaining the source code, and running CMake.
    Especially valuable are the BASH shell and git packages.
  - If you don't want to use Cygwin, there are other shells and environments that can be used.
    You can also use a Git package that doesn't come from Cygwin.

## Windows Build

Cygwin is used in order to obtain a local copy of the Git repository, and to run the CMake command that creates Visual Studio files.  Visual Studio is used to build the software, and will re-run CMake as appropriate.

Example debug x64 build (e.g. in a "Developer Command Prompt for VS2013" window):
```
cd YOUR_DEV_DIRECTORY  # cd to the root of the Vulkan-LoaderAndValidationLayers git repository
update_external_sources.bat --all
build_windows_targets.bat 
```

At this point, you can use Windows Explorer to launch Visual Studio by double-clicking on the "VULKAN.sln" file in the \build folder.  Once Visual Studio comes up, you can select "Debug" or "Release" from a drop-down list.  You can start a build with either the menu (Build->Build Solution), or a keyboard shortcut (Ctrl+Shift+B).  As part of the build process, Python scripts will create additional Visual Studio files and projects, along with additional source files.  All of these auto-generated files are under the "build" folder.

Vulkan programs must be able to find and use the vulkan-1.dll libary. Make sure it is either installed in the C:\Windows\System32 folder, or the PATH environment variable includes the folder that it is located in.

To run Vulkan programs you must tell the icd loader where to find the libraries.
This is described in a specification in the Khronos documentation Git
repository.  See the file:
https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/blob/sdk-1.0.3/loader/LoaderAndLayerInterface.md#vulkan-installable-client-driver-interface-with-the-loader

This specification describes both how ICDs and layers should be properly
packaged, and how developers can point to ICDs and layers within their builds.

### Windows 64-bit Installation Notes
If you plan on creating a Windows Install file (done in the windowsRuntimeInstaller sub-directory) you will need to build for both 32-bit and 64-bit Windows since both versions of EXEs and DLLs exist simultaneously on Windows 64.

To do this, simply create and build the release versions of each target:
```
cd LoaderAndValidationLayers  # cd to the root of the Vulkan git repository
update_external_sources.bat --all
mkdir build
cd build
cmake -G "Visual Studio 12 Win64" ..
msbuild ALL_BUILD.vcxproj /p:Platform=x64 /p:Configuration=Release
mkdir build32
cd build32
cmake -G "Visual Studio 12" ..
msbuild ALL_BUILD.vcxproj /p:Platform=x86 /p:Configuration=Release
```

