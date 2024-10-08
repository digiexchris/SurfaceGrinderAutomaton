# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.177.0/containers/cpp/.devcontainer/base.Dockerfile

# [Choice] Debian / Ubuntu version: debian-10, debian-9, ubuntu-20.04, ubuntu-18.04
ARG VARIANT="ubuntu-22.04"

FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-$VARIANT
# https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz
ENV TOOLCHAINURL="https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel"
ENV TOOLCHAINARCHIVE="arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz"
ENV TOOLCHAINDIR="arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi"

# [Optional] Uncomment this section to install additional packages.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends cmake clangd usbutils ninja-build build-essential wget ca-certificates \
    gdb-multiarch automake autoconf libtool libftdi-dev libusb-1.0-0-dev pkg-config clang-format libhidapi-dev
RUN mkdir /workspaces
WORKDIR /workspaces

RUN echo "$TOOLCHAINURL/$TOOLCHAINARCHIVE"

RUN wget $TOOLCHAINURL/$TOOLCHAINARCHIVE 
RUN tar xf $TOOLCHAINARCHIVE

RUN mv $TOOLCHAINDIR gcc-arm-none && \
    rm $TOOLCHAINARCHIVE
ENV PATH=$PATH:/workspaces/gcc-arm-none/bin

RUN ln -s /lib/x86_64-linux-gnu/libncursesw.so.6 /lib/x86_64-linux-gnu/libncursesw.so.5 && \
    ln -s /lib/x86_64-linux-gnu/libtinfo.so.6 /lib/x86_64-linux-gnu/libtinfo.so.5
# RUN mkdir -p /workspaces/pico
# WORKDIR /workspaces/pico
# RUN git clone -b master https://github.com/raspberrypi/pico-sdk.git --depth=1 && git -C pico-sdk submodule update --init
# ENV PICO_SDK_PATH=/workspaces/pico/pico-sdk
# RUN git clone -b master https://github.com/raspberrypi/pico-examples.git --depth=1
# RUN git clone -b master https://github.com/raspberrypi/pico-extras.git --depth=1
# RUN git clone -b master https://github.com/raspberrypi/debugprobe.git --depth=1


RUN git clone https://github.com/raspberrypi/openocd.git --depth=1 --recurse-submodules \
    && cd openocd && ./bootstrap && ./configure --enable-ftdi --enable-sysfsgpio --enable-picoprobe --enable-cmsis-dap && make -j 8 install 

RUN git clone https://github.com/raspberrypi/picotool.git --depth=1 \
    && cd picotool && mkdir build && cd build && cmake ../ && make -j 8 && cp picotool /usr/local/bin

# RUN git clone https://github.com/wtarreau/bootterm.git --depth=1 \
#     && cd bootterm && make -j 8 install 


# Install Node.js and npm for the webserial ui
RUN curl -fsSL https://deb.nodesource.com/setup_18.x | bash - \
    && apt-get install -y nodejs \
    && npm install -g npm \
    && npm install -g typescript \
    && npm install -g nextui-cli

WORKDIR /workspaces

COPY 99-picoprobe.rules /etc/udev/rules.d/99-picoprobe.rules

RUN usermod -aG plugdev vscode