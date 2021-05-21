# Specify the cross compiler version
# See: https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack
ARG XPM_ARM_XCOMPILER_VERSION_DEFAULT="10.2.1-1.1.2"

# Stage 0

FROM node:latest
# Issues with XPM cross-compiler on alpine..
ARG XPM_ARM_XCOMPILER_VERSION_DEFAULT
ENV XPM_ARM_XCOMPILER_VERSION=$XPM_ARM_XCOMPILER_VERSION_DEFAULT

RUN npm install --global xpm@latest; \
    xpm install --global @xpack-dev-tools/arm-none-eabi-gcc@${XPM_ARM_XCOMPILER_VERSION}

# Stage 1

FROM ubuntu:latest
# FROM alpine:latest
COPY --from=0 /root/.local/xPacks/@xpack-dev-tools /usr/tools/
ARG XPM_ARM_XCOMPILER_VERSION_DEFAULT
ENV XPM_ARM_XCOMPILER_VERSION=$XPM_ARM_XCOMPILER_VERSION_DEFAULT
RUN echo ${XPM_ARM_XCOMPILER_VERSION}

RUN apt-get update && apt-get install -y cmake g++
# RUN apk add cmake make g++

ENV PATH="/usr/tools/arm-none-eabi-gcc/${XPM_ARM_XCOMPILER_VERSION}/.content/bin:${PATH}"

WORKDIR /usr/src/app
COPY . .

RUN set -ex; \
    rm -rf build-freertos; \
    mkdir build-freertos; \
    cd build-freertos; \
    cmake -DCMAKE_BUILD_TYPE=Release ..;

ENTRYPOINT ["cmake", "--build", "build-freertos"]
CMD ["-j"]
# CMD ["bash"]
