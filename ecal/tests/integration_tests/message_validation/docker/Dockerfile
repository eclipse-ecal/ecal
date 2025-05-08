FROM ecal_base

WORKDIR /app

COPY ../src/ ./src/
RUN rm -rf ./src/build
COPY ../scripts/entrypoint.sh ./entrypoint.sh

WORKDIR /app/src
RUN mkdir -p build && cd build && \
    cmake .. && \
    make -j$(nproc)

WORKDIR /app
RUN chmod +x ./entrypoint.sh
ENTRYPOINT ["./entrypoint.sh"]
