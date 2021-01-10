FROM debian:stretch-slim

# https://wiki.winehq.org/Debian
WORKDIR /tmp

RUN apt update && apt install -y wget unzip gnupg apt-transport-https ca-certificates

RUN dpkg --add-architecture i386 && \
suite="$(awk '$1 == "deb" { print $3; exit }' /etc/apt/sources.list)" && \
echo "deb https://dl.winehq.org/wine-builds/debian $suite main" > /etc/apt/sources.list.d/winehq.list && \
wget -nc https://dl.winehq.org/wine-builds/winehq.key && apt-key add winehq.key

# RUN apt update

# RUN apt install --install-recommends winehq-stable && rm -rf /var/lib/apt/lists/*


# https://dl.winehq.org/wine-builds/debian/dists/stretch/main/binary-amd64/?C=N;O=D
# https://www.winehq.org/news/
ENV WINE_VERSION 5.0.2

RUN set -eux; \
        { \
                echo 'Package: *wine* *wine*:i386'; \
                echo "Pin: version $WINE_VERSION~*"; \
                echo 'Pin-Priority: 1001'; \
        } > /etc/apt/preferences.d/winehq.pref; \
        apt-get update; \
        apt-get install -y --no-install-recommends \
                "winehq-stable=$WINE_VERSION~*" \
        ; \
        rm -rf /var/lib/apt/lists/*


WORKDIR /app
ENV WINEARCH=win32

# Download transaq connector library
ARG TRANSAQ_CONNECTOR=616TXmlConnector.2.21.2.zip
RUN wget https://www.finam.ru/files/$TRANSAQ_CONNECTOR && unzip $TRANSAQ_CONNECTOR && \
ln -s x86/txmlconnector.dll txmlconnector.dll && \
rm *.zip

# Download transaq proxy build
RUN wget https://raw.githubusercontent.com/vagabondan/txcproxy/master/out/install/x86-Release/bin/txcproxy.exe
# or
# COPY ./out/install/x86-Release/bin/txcproxy.exe /app/

# Workaround: preconfigure wine
RUN wine txcproxy ; bash -c "exit 0"

ARG PX_PORT=4444
EXPOSE $PX_PORT 4445 4446 4447 4448 4449

ENTRYPOINT wine txcproxy -tq_addr tr1.finam.ru -tq_port 3900 -px_port $PX_PORT -ll2 2
