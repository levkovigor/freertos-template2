FROM ubuntu:latest

RUN apt-get update -y && apt-get upgrade -y
RUN apt-get install -y cmake g++

COPY . /usr/src/app
WORKDIR /usr/src/app

RUN set -ex; \
    rm -rf build-hosted; \
    mkdir build-hosted; \
    cd build-hosted; \
    cmake -DCMAKE_BUILD_TYPE=Release -DOS_FSFW=host ..;

ENTRYPOINT ["cmake", "--build", "build-hosted"]
CMD ["-j"]
