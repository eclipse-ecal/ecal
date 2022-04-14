#ecal base image:
FROM ecal-runtime

WORKDIR /src/pub

COPY CMakeLists.txt main.cpp ./
RUN cmake . && make
CMD ./hello_world_snd