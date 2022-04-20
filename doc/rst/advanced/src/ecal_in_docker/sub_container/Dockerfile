#ecal base image:
FROM ecal-runtime

WORKDIR /src/sub

COPY CMakeLists.txt main.cpp ./
RUN cmake . && make
CMD ./hello_world_rec