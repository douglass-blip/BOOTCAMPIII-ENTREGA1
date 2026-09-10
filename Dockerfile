# syntax=docker/dockerfile:1
FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j"$(nproc)"

# Imagem final, mais enxuta, só com o binário compilado
FROM ubuntu:24.04
WORKDIR /app
COPY --from=build /app/build/url_shortener /app/url_shortener

ENTRYPOINT ["/app/url_shortener"]
