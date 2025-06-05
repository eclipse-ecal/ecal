FROM ecal_base

WORKDIR /app

# Dependencies
RUN apt-get update && apt-get install -y libtclap-dev

COPY lib/EcalConfigHelper /app/lib/EcalConfigHelper
COPY basic_pub_sub/src/ /app/src/
COPY basic_pub_sub/scripts/entrypoint.sh /app/entrypoint.sh

WORKDIR /app/src
RUN mkdir build && cd build && \
    cmake -DECALCONFIGHELPER_PATH=/app/lib/EcalConfigHelper .. && \
    make -j$(nproc)

WORKDIR /app
RUN chmod +x ./entrypoint.sh
ENTRYPOINT ["./entrypoint.sh"]
