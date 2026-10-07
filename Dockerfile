# syntax=docker/dockerfile:1

# -----------------------------------------------------------------
# Build stage: same toolchain as .github/workflows/build-linux.yml
# -----------------------------------------------------------------
FROM ubuntu:24.04 AS build

RUN apt-get update -q \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      ca-certificates \
      git \
      cmake \
      gcc-13 \
      g++-13 \
      ninja-build \
      libboost-all-dev \
      libssl-dev \
      libmysqlclient-dev \
      libreadline-dev \
      zlib1g-dev \
 && rm -rf /var/lib/apt/lists/*

ARG BUILD_TYPE=RelWithDebInfo
ARG JOBS=6

WORKDIR /src
COPY . /src

# The build tree is kept in a cache mount, so a rebuild after a source
# change only recompiles what changed.
RUN --mount=type=cache,target=/build \
    git config --global --add safe.directory /src \
 && cmake -S /src -B /build -G Ninja \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DCMAKE_C_COMPILER=gcc-13 \
      -DCMAKE_CXX_COMPILER=g++-13 \
      -DSCRIPTS=static \
      -DTOOLS=ON \
      -DSERVERS=ON \
      -DCMAKE_INSTALL_PREFIX=/opt/arguscore \
 && cmake --build /build --parallel ${JOBS} \
 && cmake --install /build

# -----------------------------------------------------------------
# Runtime stage: servers, extraction tools and the sql tree used by
# the database auto-updater
# -----------------------------------------------------------------
FROM ubuntu:24.04 AS runtime

# mysql-client: the auto-updater applies sql files through the mysql CLI
RUN apt-get update -q \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      ca-certificates \
      libboost-filesystem1.83.0 \
      libboost-program-options1.83.0 \
      libboost-regex1.83.0 \
      libboost-locale1.83.0 \
      libssl3t64 \
      libmysqlclient21 \
      libreadline8t64 \
      zlib1g \
      mysql-client \
 && rm -rf /var/lib/apt/lists/*

COPY --from=build /opt/arguscore /opt/arguscore
COPY sql /opt/arguscore/src/sql
COPY contrib/Docker/entrypoint.sh /usr/local/bin/entrypoint.sh

# Fail the image build early if a shared library is missing
RUN chmod +x /usr/local/bin/entrypoint.sh \
 && for bin in /opt/arguscore/bin/*; do \
      if ldd "$bin" | grep "not found"; then echo "missing libraries for $bin"; exit 1; fi; \
    done

ENV PATH=/opt/arguscore/bin:$PATH

# Configs, client data, logs and the TDB dumps live here (bind mount)
WORKDIR /srv/arguscore

ENTRYPOINT ["entrypoint.sh"]
CMD ["worldserver"]
