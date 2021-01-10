FROM debian:stretch-slim AS wine

# https://wiki.winehq.org/Debian

RUN set -eux; \
        apt-get update; \
        apt-get install -y --no-install-recommends apt-transport-https ca-certificates; \
        savedAptMark="$(apt-mark showmanual)"; \
        apt-get install -y --no-install-recommends gnupg dirmngr; \
        rm -rf /var/lib/apt/lists/*; \
        \
        export GNUPGHOME="$(mktemp -d)"; \
        gpg --batch --keyserver ha.pool.sks-keyservers.net --recv-keys D43F640145369C51D786DDEA76F1A20FF987672F; \
        gpg --batch --export --armor D43F640145369C51D786DDEA76F1A20FF987672F > /etc/apt/trusted.gpg.d/winehq.gpg.asc; \
        gpgconf --kill all; \
        rm -rf "$GNUPGHOME"; \
        apt-key list | grep 'WineHQ'; \
        \
        apt-mark auto '.*' > /dev/null; \
        apt-mark manual $savedAptMark > /dev/null; \
        apt-get purge -y --auto-remove -o APT::AutoRemove::RecommendsImportant=false; \
        \
        dpkg --add-architecture i386; \
        suite="$(awk '$1 == "deb" { print $3; exit }' /etc/apt/sources.list)"; \
        echo "deb https://dl.winehq.org/wine-builds/debian $suite main" > /etc/apt/sources.list.d/winehq.list

# https://dl.winehq.org/wine-builds/debian/dists/buster/main/binary-amd64/?C=N;O=D
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


FROM wine AS transaq

WORKDIR /app
ENV WINEARCH=win32

RUN apt update && apt install -y wget unzip

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
