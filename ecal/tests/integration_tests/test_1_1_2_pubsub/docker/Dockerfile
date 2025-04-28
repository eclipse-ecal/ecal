FROM ecal_base

WORKDIR /app

# Copy everything you need (source + scripts)
COPY ../src/ ./src/
COPY ../scripts/entrypoint.sh .

# Build publisher and subscriber using CMake
WORKDIR /app/src
RUN mkdir -p build && cd build && \
    cmake .. && \
    make -j$(nproc)

WORKDIR /app
RUN chmod +x entrypoint.sh
ENTRYPOINT ["./entrypoint.sh"]