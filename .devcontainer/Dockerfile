FROM ubuntu:24.04

RUN apt update && apt install -y git pkgconf pkgconf-bin sudo openocd gdb ccache cmake ninja-build build-essential clang-tools clang-format lld lldb clang clang-tidy clangd gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential libstdc++-arm-none-eabi-newlib unzip wget xz-utils python3-pip python3-venv python3-full 
RUN mkdir /workspaces
# ENV WORKSPACE_BASE_PATH /workspaces/

WORKDIR /workspaces

COPY pi_pico.sh /workspaces/

WORKDIR /workspaces

ENV SKIP_VSCODE 1
ENV SKIP_UART 1

RUN apt install -y  libusb-0.1-4 libusb-dev

RUN chmod +x ./pi_pico.sh && ./pi_pico.sh

# RUN git clone -b master --depth=1 https://github.com/raspberrypi/pico-sdk.git ${PICO_SDK_PATH} \
#   && git -C ${PICO_SDK_PATH}/lib submodule update --init --depth=1 \
#   && git -C ${PICO_SDK_PATH}/lib/tinyusb submodule update --init --depth=1 hw/mcu/raspberry_pi

# ENV PICO_EXAMPLES_PATH /workspaces/pico-examples
#   RUN locale-gen en_US.UTF-8
# ENV LANG en_US.UTF-8
# ENV LANGUAGE en_US:en
# ENV LC_ALL en_US.UTF-8