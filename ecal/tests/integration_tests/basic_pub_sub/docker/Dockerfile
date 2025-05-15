FROM ecal_base

WORKDIR /app

# Install dependencies
RUN apt-get update && apt-get install -y libtclap-dev

# Copy libraries and scripts
COPY lib/EcalConfigHelper /app/lib/EcalConfigHelper
COPY basic_pub_sub/src/ ./src/
COPY basic_pub_sub/scripts/entrypoint.sh ./entrypoint.sh

# Build the libraries
WORKDIR /app/lib/EcalConfigHelper
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make -j$(nproc) && make install

# Build publisher and subscriber
WORKDIR /app/src
RUN mkdir -p build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Set entrypoint
WORKDIR /app
RUN chmod +x ./entrypoint.sh
ENTRYPOINT ["./entrypoint.sh"]
