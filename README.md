Hi i've been building kbeam, a native Miracast application build using KDE-frameworks. 
Below are the steps to build:

### Install Dependencies: 
• Arch Linux / Manjaro / EndeavourOS:
````
    sudo pacman -S --needed cmake extra-cmake-modules qt6-declarative qt6-quickcontrols2 \
    kf6-kirigami kf6-kcoreaddons kf6-ki18n kf6-networkmanager-qt kf6-kitemmodels \
    gst-plugins-base gst-plugins-good gst-plugins-bad gst-plugins-ugly \
    gst-plugin-pipewire gst-rtsp-server
````

• Fedora :
````
    sudo dnf install cmake extra-cmake-modules qt6-qtdeclarative-devel \
    kf6-kirigami-devel kf6-kcoreaddons-devel kf6-ki18n-devel \
    kf6-networkmanager-qt-devel kf6-kitemmodels-devel \
    gstreamer1-devel gstreamer1-rtsp-server-devel gstreamer1-plugins-base-devel \
    gstreamer1-plugins-good gstreamer1-plugins-bad-free gstreamer1-plugins-ugly-free \
    gstreamer1-plugin-pipewire
````

• openSUSE Tumbleweed:
````
    sudo zypper install cmake extra-cmake-modules qt6-declarative-devel \
    kf6-kirigami-devel kf6-kcoreaddons-devel kf6-ki18n-devel \
    kf6-networkmanager-qt-devel kf6-kitemmodels-devel \
    gstreamer-devel gstreamer-plugins-base-devel gstreamer-plugins-bad-devel \
    gstreamer-rtsp-server-devel
````

### Build:
````
    git clone https://invent.kde.org/mradul/kbeam.git
    cd kbeam
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ./build/bin/kbeam
````
