FROM ecal_base

WORKDIR /app
COPY ../../src/ ./src/
COPY ../../scripts/entrypoint.sh .

WORKDIR /app/src
RUN g++ publisher.cpp -o /app/publisher \
    -I/usr/local/include \
    -L/usr/local/lib -Wl,-rpath,/usr/local/lib \
    -lecal_core -lecaltime -lecal

RUN g++ subscriber.cpp -o /app/subscriber \
    -I/usr/local/include \
    -L/usr/local/lib -Wl,-rpath,/usr/local/lib \
    -lecal_core -lecaltime -lecal

WORKDIR /app
RUN chmod +x entrypoint.sh
ENTRYPOINT ["./entrypoint.sh"]
