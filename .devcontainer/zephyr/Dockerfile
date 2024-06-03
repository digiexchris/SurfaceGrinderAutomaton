FROM ubuntu:24.04

## Arguments
ARG ZSDK_VERSION=0.16.8
ARG LLVM_VERSION=14
ARG WGET_ARGS="-q --show-progress --progress=bar:force:noscroll --no-check-certificate"

ARG UID=1000
ARG GID=1000

ENV DEBIAN_FRONTEND noninteractive

# RUN dpkg --add-architecture i386 && \ gcc-arm-none-eabi libnewlib-arm-none-eabi
RUN	apt-get -y update && \
    #apt-get -y upgrade && \
    apt-get install --no-install-recommends clangd -y \
            python3-pip python3-venv python3-full pipx cmake git gpg-agent locales \
			lsb-release make ninja-build device-tree-compiler software-properties-common \
			ssh sudo unzip wget xz-utils  \
            gperf build-essential libc6-dev cmake openocd  build-essential \ 
  ccache dfu-util device-tree-compiler wget dotnet8 dotnet-sdk-8.0 dotnet-runtime-8.0 \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 screen && \
    apt-get clean && \
	apt-get autoremove --purge 
    # && \
    #//pipx ensurepath  # optional to allow pipx actions in global scope. See "Global installation" section below.

RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

## Python3
# RUN pip3 install --no-cache-dir --upgrade wheel pip && \
#     pip3 install --no-cache-dir -r https://raw.githubusercontent.com/zephyrproject-rtos/zephyr/master/scripts/requirements.txt && \
#     rm -rf $(pip cache dir)

## LLVM & clang-format
# RUN wget ${WGET_ARGS} https://apt.llvm.org/llvm.sh && \
#     chmod +x llvm.sh && \
#     ./llvm.sh ${LLVM_VERSION} && \
#     rm llvm.sh && \
#     apt-get install --no-install-recommends -y clangd clang-format-${LLVM_VERSION} && \
#     apt-get clean && \
# 	apt-get autoremove --purge

# RUN update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-${LLVM_VERSION} 100
# RUN update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-${LLVM_VERSION} 100

## get run-clang-format
RUN wget https://raw.githubusercontent.com/Sarcasm/run-clang-format/master/run-clang-format.py -O /usr/local/bin/run-clang-format.py && \
    chmod 755 /usr/local/bin/run-clang-format.py
# https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.5/zephyr-sdk-0.16.5_linux-x86_64.tar.xz

## Zephyr SDK (only arm toolchain) https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_linux-x86_64.tar.xz
RUN mkdir -p /opt/toolchains && wget ${WGET_ARGS} https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZSDK_VERSION}/zephyr-sdk-${ZSDK_VERSION}_linux-x86_64.tar.xz && \
    tar -xf "zephyr-sdk-${ZSDK_VERSION}_linux-x86_64.tar.xz" -C /opt/toolchains && \
    rm "zephyr-sdk-${ZSDK_VERSION}_linux-x86_64.tar.xz"

## Zephyr
# RUN python3 -m venv /workspaces/zephyrproject/.venv && source /workspaces/zephyrproject/.venv/bin/activate && \
RUN  rm /usr/lib/python3.12/EXTERNALLY-MANAGED
RUN  python3 -m pip install west && west init /workspaces/zephyrproject && \
    cd /workspaces/zephyrproject && \
    west update && \
    west zephyr-export && \
    python3 -m pip install -r /workspaces/zephyrproject/zephyr/scripts/requirements.txt 
    # && \
    # rm -rf $(pip cache dir) && \
    # mkdir -m777 /workspaces/zephyrproject/zephyr/.cache 
    #&& \
    #chown -R user:user /workspaces/zephyrproject

# RUN mkdir -p /opt/toolchains/ && cd /opt/toolchains/ && git clone https://github.com/raspberrypi/pico-sdk.git --branch master && \
#     cd /opt/toolchains/pico-sdk && git submodule update --init && \


## install nrfjprog and Segger JLink Tools
# RUN wget ${WGET_ARGS} https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/10-12-1/nRFCommandLineTools10121Linuxamd64.tar.gz && \
#     tar xvfz nRFCommandLineTools10121Linuxamd64.tar.gz ./JLink_Linux_V688a_x86_64.deb ./nRF-Command-Line-Tools_10_12_1_Linux-amd64.deb && \
# 	dpkg -i JLink_Linux_V688a_x86_64.deb && \
#     dpkg -i nRF-Command-Line-Tools_10_12_1_Linux-amd64.deb && \
#     ln -s /opt/nrfjprog/nrfjprog /usr/local/bin/nrfjprog && \
#     ln -s /opt/mergehex/mergehex /usr/local/bin/mergehex && \
#     rm nRFCommandLineTools10121Linuxamd64.tar.gz JLink_Linux_V688a_x86_64.deb nRF-Command-Line-Tools_10_12_1_Linux-amd64.deb

## add user
# RUN groupadd -g $GID -o user && \
#     useradd -u $UID -m -g user -G plugdev user && \
# 	echo 'user ALL = NOPASSWD: ALL' > /etc/sudoers.d/user && \
# 	chmod 0440 /etc/sudoers.d/user

# ## bash-git-prompt
# RUN git clone https://github.com/magicmonty/bash-git-prompt.git /home/user/.bash-git-prompt --depth=1 && \
#     echo '\n \
#         if [ -f "$HOME/.bash-git-prompt/gitprompt.sh" ]; then \n \
#             GIT_PROMPT_ONLY_IN_REPO=1 \n \
#             source $HOME/.bash-git-prompt/gitprompt.sh \n \
#         fi\n' >> /home/user/.bashrc && \
#     echo '\n \
#         source /workspaces/zephyrproject/zephyr/zephyr-env.sh \
#         \n' >> /home/user/.bashrc && \
#         git config --global --add safe.directory /workspaces/zephyrproject/zephyr

# Set environment variables
ENV ZEPHYR_TOOLCHAIN_VARIANT=zephyr
ENV ZEPHYR_SDK_INSTALL_DIR=/opt/toolchains/zephyr-sdk-${ZSDK_VERSION}
ENV GNUARMEMB_TOOLCHAIN_PATH=/opt/toolchains/${GCC_ARM_NAME}
ENV PKG_CONFIG_PATH=/usr/lib/i386-linux-gnu/pkgconfig
ENV ZEPHYR_BASE=/workspaces/zephyrproject/zephyr
#ENV PATH="${PATH}:/root/.local/bin"

#RUN echo "export PATH=/root/.local/bin:${PATH}" >> /etc/bash.bashrc
# RUN echo "pipx ensurepath" >> /etc/bash.bashrc

# RUN python3 -m pipx ensurepath

# RUN python3 -m pipx completions

# RUN echo "eval \"\$(register-python-argcomplete pipx)\"" >> ~/.bashrc