FROM gcc:14.2.0

# Устанавливаем нужные пакеты + Boost
RUN apt-get update && apt-get install -y \
    cmake \
    python3 python3-dev python3-pip \
    wrk \
    clangd \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

# Устанавливаем Python пакеты
RUN pip install numpy --break-system-packages

WORKDIR /workspace
