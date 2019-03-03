FROM debian:stretch-slim as build

# Install our build dependencies
RUN apt-get update \
  && apt-get install -y \
    curl \
    build-essential \
  	pkg-config \
  	libc6-dev \
  	m4 \
  	g++-multilib \
    autoconf \
  	libtool \
  	ncurses-dev \
  	unzip \
  	git \
  	python \
    zlib1g-dev \
  	wget \
  	bsdmainutils \
  	automake \
  	p7zip-full \
  	pwgen \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/local/src/
COPY . /usr/local/src/

RUN ./zcutil/build.sh -j$(nproc)
RUN ./zcutil/fetch-params.sh

FROM debian:stretch-slim

# Add our user and group first to ensure consistency
RUN groupadd -r bitcoinz && useradd -r -d /bitcoinz -g bitcoinz bitcoinz

# Install our run dependencies
RUN apt-get update \
  && apt-get install -y \
    curl \
    libc6-dev \
    g++-multilib \
  && rm -rf /var/lib/apt/lists/*

# Setup application directory
RUN mkdir -p /bitcoinz/data

# Copy binaries from build container
COPY --from=build /usr/local/src/src/bitcoinzd /usr/local/bin
COPY --from=build /usr/local/src/src/bitcoinz-cli /usr/local/bin
COPY --from=build /usr/local/src/src/zcash-gtest /usr/local/bin
COPY --from=build /usr/local/src/src/bitcoinz-tx /usr/local/bin
COPY contrib/docker/cli /usr/local/bin

RUN chmod +x /usr/local/bin/cli

# Copy zcash params
COPY --from=build /root/.zcash-params /bitcoinz/.zcash-params

RUN chown -R bitcoinz: /bitcoinz
USER bitcoinz
WORKDIR /bitcoinz
CMD ["bitcoinzd", "-datadir=/bitcoinz/data"]
