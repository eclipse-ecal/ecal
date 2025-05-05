FROM ecal_base

WORKDIR /app
RUN apt-get update && apt-get install -y libtclap-dev

COPY ../src/ ./src/
COPY ../scripts/entrypoint.sh ./entrypoint.sh

WORKDIR /app/src
RUN mkdir -p build && cd build && \
    cmake .. && \
    make -j$(nproc)

WORKDIR /app
RUN chmod +x ./entrypoint.sh
ENTRYPOINT ["./entrypoint.sh"]
