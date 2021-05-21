FROM ubuntu:latest
# FROM alpine:latest

RUN apt-get update && apt-get install -y cmake g++
# RUN apk add cmake make g++

WORKDIR /usr/src/app
COPY . .

RUN set -ex; \
    rm -rf build-hosted; \
    mkdir build-hosted; \
    cd build-hosted; \
    cmake -DCMAKE_BUILD_TYPE=Release -DOS_FSFW=host ..;

ENTRYPOINT ["cmake", "--build", "build-hosted"]
CMD ["-j"]
# CMD ["bash"]
